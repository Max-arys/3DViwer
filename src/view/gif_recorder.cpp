// gifrecorder.cpp
#include "gif_recorder.h"

#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QPainter>

#include "glwidget.h"

namespace s21 {

GIFRecorder::GIFRecorder(GLWidget* widget, QObject* parent)
    : QObject(parent), glWidget_(widget) {
  this->moveToThread(&workerThread_);

  connect(&workerThread_, &QThread::started, this,
          []() { qDebug() << "GIF worker thread started"; });

  connect(this, &GIFRecorder::frameReady, this, &GIFRecorder::processFrame,
          Qt::QueuedConnection);

  captureTimer_ = new QTimer(this);
  captureTimer_->setTimerType(Qt::PreciseTimer);
  connect(captureTimer_, &QTimer::timeout, this, &GIFRecorder::captureFrame);

  workerThread_.start();
}

GIFRecorder::~GIFRecorder() {
  stopRecording();
  workerThread_.quit();
  workerThread_.wait(1000);
  if (workerThread_.isRunning()) {
    workerThread_.terminate();
  }
}

void GIFRecorder::startRecording(const QString& filepath, int fps,
                                 int durationMs) {
  if (recording_.load()) {
    emit recordingError("Recording already in progress");
    return;
  }

  if (!glWidget_) {
    emit recordingError("GLWidget is null");
    return;
  }

  outputPath_ = filepath;
  fps_ = fps;
  totalDurationMs_ = durationMs;
  frameDelayMs_ = 1000 / fps;
  totalFrames_ = (durationMs * fps) / 1000;

  {
    QMutexLocker locker(&mutex_);
    frameQueue_.clear();
  }

  framesCaptured_ = 0;
  stopRequested_ = false;
  recording_ = true;

  recordingTimer_.start();

  QMetaObject::invokeMethod(captureTimer_, "start", Qt::QueuedConnection,
                            Q_ARG(int, frameDelayMs_));

  emit recordingStarted();
  qDebug() << "Recording started:" << filepath << "FPS:" << fps
           << "Duration:" << durationMs << "ms"
           << "Total frames:" << totalFrames_;
}

void GIFRecorder::stopRecording() {
  if (!recording_.load()) return;

  qDebug() << "Stop recording requested";
  stopRequested_ = true;

  QMetaObject::invokeMethod(captureTimer_, "stop", Qt::QueuedConnection);
}

void GIFRecorder::captureFrame() {
  if (!recording_.load() || stopRequested_.load()) {
    return;
  }

  qint64 elapsed = recordingTimer_.elapsed();
  emit recordingProgress(static_cast<int>(elapsed), totalDurationMs_);

  if (elapsed >= totalDurationMs_ || framesCaptured_ >= totalFrames_) {
    qDebug() << "Reached limit, stopping capture. Frames:" << framesCaptured_
             << "/" << totalFrames_;
    QMetaObject::invokeMethod(captureTimer_, "stop", Qt::QueuedConnection);
    QTimer::singleShot(500, this, [this]() { writeGifFile(); });
    return;
  }

  glWidget_->makeCurrent();

  int w = glWidget_->width();
  int h = glWidget_->height();

  // Важно: убедимся, что размеры корректные
  if (w <= 0 || h <= 0) {
    qDebug() << "Invalid widget size:" << w << "x" << h;
    glWidget_->doneCurrent();
    return;
  }

  // Читаем кадры из буфера цвета
  QImage frame(w, h, QImage::Format_RGBA8888);
  glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, frame.bits());

  glWidget_->doneCurrent();

  // Проверяем, что изображение не пустое
  if (frame.isNull()) {
    qDebug() << "Captured null frame!";
    return;
  }

  // Зеркалим по вертикали (OpenGL имеет начало координат внизу слева)
  QImage mirrored = frame.mirrored(false, true);

  // Конвертируем в RGB для экономии памяти и правильной обработки
  QImage rgbFrame = mirrored.convertToFormat(QImage::Format_RGB888);

  qDebug() << "Frame" << framesCaptured_ << "captured:" << rgbFrame.width()
           << "x" << rgbFrame.height();

  emit frameReady(rgbFrame);
  framesCaptured_++;

  if (framesCaptured_ % 10 == 0) {
    qDebug() << "Frame captured:" << framesCaptured_ << "/" << totalFrames_;
  }
}

void GIFRecorder::processFrame(const QImage& frame) {
  if (!recording_.load() && !stopRequested_.load()) return;

  QMutexLocker locker(&mutex_);
  frameQueue_.enqueue(frame);
}

void GIFRecorder::writeGifFile() {
  qDebug() << "writeGifFile called, queue size:" << frameQueue_.size();

  if (frameQueue_.isEmpty()) {
    qDebug() << "No frames in queue!";
    recording_ = false;
    emit recordingError("No frames captured");
    return;
  }

  QQueue<QImage> framesToProcess;
  {
    QMutexLocker locker(&mutex_);
    framesToProcess = frameQueue_;
    frameQueue_.clear();
  }

  qDebug() << "Writing GIF file with" << framesToProcess.size() << "frames to"
           << outputPath_;

  int error = 0;
  GifFileType* gif =
      EGifOpenFileName(outputPath_.toUtf8().constData(), false, &error);
  if (!gif) {
    recording_ = false;
    emit recordingError(
        QString("Failed to create GIF: %1").arg(GifErrorString(error)));
    return;
  }

  int w = framesToProcess.first().width();
  int h = framesToProcess.first().height();

  EGifSetGifVersion(gif, true);

  // Создаём простую глобальную палитру
  ColorMapObject* globalCmap = GifMakeMapObject(256, nullptr);
  if (!globalCmap) {
    EGifCloseFile(gif, nullptr);
    recording_ = false;
    emit recordingError("Failed to create global color map");
    return;
  }

  // Заполняем палитру
  for (int i = 0; i < 256; i++) {
    globalCmap->Colors[i].Red = i;
    globalCmap->Colors[i].Green = i;
    globalCmap->Colors[i].Blue = i;
  }

  // Записываем Screen Descriptor
  if (EGifPutScreenDesc(gif, w, h, 8, 0, globalCmap) == GIF_ERROR) {
    GifFreeMapObject(globalCmap);
    EGifCloseFile(gif, nullptr);
    recording_ = false;
    emit recordingError("Failed to put screen descriptor");
    return;
  }
  GifFreeMapObject(globalCmap);

  // NETSCAPE 2.0 loop extension - правильная структура
  unsigned char nsExtension[19] = {
      0x21, 0xFF, 0x0B,  // Extension introducer, Application extension, Block
                         // size
      'N',  'E',  'T',  'S', 'C', 'A',
      'P',  'E',  '2',  '.', '0',  // Application identifier
      0x03, 0x01,                  // Sub-block size, always 1 for loop
      0x00, 0x00,                  // Loop count (0 = infinite)
      0x00                         // Block terminator
  };

  if (EGifPutExtension(gif, APPLICATION_EXT_FUNC_CODE, 16, nsExtension + 3) ==
      GIF_ERROR) {
    EGifCloseFile(gif, nullptr);
    recording_ = false;
    emit recordingError("Failed to put NETSCAPE extension");
    return;
  }

  // Добавляем комментарий
  const char* comment = "Created by 3DViewer";
  EGifPutComment(gif, const_cast<char*>(comment));

  int frameCount = 0;
  int totalFrames = framesToProcess.size();

  while (!framesToProcess.isEmpty()) {
    QImage frame = framesToProcess.dequeue();

    if (addFrameToGif(gif, frame, frameDelayMs_) == GIF_ERROR) {
      EGifCloseFile(gif, nullptr);
      recording_ = false;
      emit recordingError("Failed to add frame to GIF");
      return;
    }
    frameCount++;

    if (frameCount % 10 == 0) {
      qDebug() << "Processed" << frameCount << "/" << totalFrames << "frames";
    }
  }

  if (EGifCloseFile(gif, &error) == GIF_ERROR) {
    recording_ = false;
    emit recordingError(
        QString("Failed to close GIF: %1").arg(GifErrorString(error)));
    return;
  }

  recording_ = false;

  QFileInfo fi(outputPath_);
  qDebug() << "GIF saved:" << outputPath_ << "Size:" << fi.size() << "bytes"
           << "Frames:" << totalFrames;

  emit recordingStopped(outputPath_);
}

int GIFRecorder::addFrameToGif(GifFileType* gif, const QImage& image,
                               int delayMs) {
  // Конвертируем в RGB
  QImage rgbImage = image.convertToFormat(QImage::Format_RGB888);

  // Создаём локальную палитру
  GifColorType palette[256];
  QByteArray indexedData = quantizeImage(rgbImage, palette);

  ColorMapObject* cmap = GifMakeMapObject(256, palette);
  if (!cmap) {
    qDebug() << "Failed to create color map";
    return GIF_ERROR;
  }

  // Графический контрол для анимации - ПРАВИЛЬНЫЙ СПОСОБ
  GraphicsControlBlock gcb;
  gcb.DisposalMode = DISPOSE_DO_NOT;
  gcb.UserInputFlag = false;
  gcb.DelayTime = delayMs / 10;  // В сотых долях секунды
  gcb.TransparentColor = NO_TRANSPARENT_COLOR;

  GifByteType gcbData[8];
  size_t gcbSize = EGifGCBToExtension(&gcb, gcbData);

  // Используем EGifPutExtensionLeader/Block/Trailer вместо EGifPutExtension
  if (EGifPutExtensionLeader(gif, GRAPHICS_EXT_FUNC_CODE) == GIF_ERROR) {
    GifFreeMapObject(cmap);
    return GIF_ERROR;
  }

  if (EGifPutExtensionBlock(gif, gcbSize, gcbData) == GIF_ERROR) {
    GifFreeMapObject(cmap);
    return GIF_ERROR;
  }

  if (EGifPutExtensionTrailer(gif) == GIF_ERROR) {
    GifFreeMapObject(cmap);
    return GIF_ERROR;
  }

  // Добавляем дескриптор изображения с локальной палитрой
  if (EGifPutImageDesc(gif, 0, 0, image.width(), image.height(), false, cmap) ==
      GIF_ERROR) {
    GifFreeMapObject(cmap);
    return GIF_ERROR;
  }

  gif->SWidth = image.width();
  gif->SHeight = image.height();
  gif->SColorResolution = 8;  // 8 бит на канал
  gif->SBackGroundColor = 0;
  gif->AspectByte = 0;  // 0 = квадратные пиксели (1:1)

  // Записываем данные изображения построчно
  for (int y = 0; y < image.height(); y++) {
    if (EGifPutLine(gif,
                    reinterpret_cast<GifPixelType*>(indexedData.data() +
                                                    y * image.width()),
                    image.width()) == GIF_ERROR) {
      GifFreeMapObject(cmap);
      return GIF_ERROR;
    }
  }

  GifFreeMapObject(cmap);
  return GIF_OK;
}

QByteArray GIFRecorder::quantizeImage(const QImage& image,
                                      GifColorType* palette) {
  QVector<QRgb> colorTable;

  // Улучшенная палитра с учётом часто используемых цветов
  for (int r = 0; r < 8; r++) {
    for (int g = 0; g < 8; g++) {
      for (int b = 0; b < 4; b++) {
        if (colorTable.size() < 256) {
          colorTable.append(qRgb(r * 36, g * 36, b * 85));
        }
      }
    }
  }

  // Заполняем оставшиеся слоты оттенками серого
  while (colorTable.size() < 256) {
    int gray = (colorTable.size() - 216) * 255 / 39;
    colorTable.append(qRgb(gray, gray, gray));
  }

  // Копируем палитру в GIF формат
  for (int i = 0; i < 256; i++) {
    QRgb rgb = colorTable[i];
    palette[i].Red = qRed(rgb);
    palette[i].Green = qGreen(rgb);
    palette[i].Blue = qBlue(rgb);
  }

  // Конвертируем изображение
  QImage indexed = image.convertToFormat(QImage::Format_Indexed8, colorTable);

  // КОПИРУЕМ ПОСТРОЧНО с учётом bytesPerLine()
  QByteArray result;
  result.resize(indexed.bytesPerLine() * image.height());

  for (int y = 0; y < image.height(); y++) {
    const uchar* srcLine = indexed.scanLine(y);
    memcpy(result.data() + y * indexed.bytesPerLine(), srcLine,
           indexed.bytesPerLine());
  }

  return result;
}

}  // namespace s21