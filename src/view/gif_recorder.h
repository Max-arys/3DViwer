// gifrecorder.h
#ifndef GIF_RECORDER_H
#define GIF_RECORDER_H

#include <QElapsedTimer>
#include <QImage>
#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QThread>
#include <QTimer>
#include <QWaitCondition>
#include <atomic>
#include <memory>

extern "C" {
#include <gif_lib.h>
}

namespace s21 {

class GLWidget;

class GIFRecorder : public QObject {
  Q_OBJECT

 public:
  explicit GIFRecorder(GLWidget* widget, QObject* parent = nullptr);
  ~GIFRecorder();

  void startRecording(const QString& filepath, int fps = 10,
                      int durationMs = 5000);
  void stopRecording();
  bool isRecording() const { return recording_.load(); }

 signals:
  void recordingStarted();
  void recordingStopped(const QString& filepath);
  void recordingProgress(int elapsedMs, int totalMs);
  void recordingError(const QString& error);

  // Внутренний сигнал для передачи кадров из GUI-потока в рабочий
  void frameReady(const QImage& frame);

 private slots:
  void captureFrame();
  void processFrame(const QImage& frame);

 private:
  void writeGifFile();
  int addFrameToGif(GifFileType* gif, const QImage& image, int delayMs);
  QByteArray quantizeImage(const QImage& image, GifColorType* palette);

  GLWidget* glWidget_;

  QThread workerThread_;
  QMutex mutex_;
  QWaitCondition frameAvailable_;
  QQueue<QImage> frameQueue_;

  std::atomic<bool> recording_{false};
  std::atomic<bool> stopRequested_{false};

  QString outputPath_;
  int fps_;
  int totalDurationMs_;
  int frameDelayMs_;
  int totalFrames_{0};
  int framesCaptured_{0};

  QElapsedTimer recordingTimer_;
  QTimer* captureTimer_;
};

}  // namespace s21

#endif  // GIF_RECORDER_H