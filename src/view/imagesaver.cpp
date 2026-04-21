#include "imagesaver.h"

#include <QDebug>
#include <QFileInfo>

namespace s21 {

ImageSaver::ImageSaver() {}

ImageSaver::~ImageSaver() {}

bool ImageSaver::saveImage(const QImage& image, const QString& filepath) {
  return image.save(filepath);
}

bool ImageSaver::saveAsBMP(const QImage& image, const QString& filePath) {
  QImage bmpImage = image;
  if (bmpImage.hasAlphaChannel()) {
    bmpImage = bmpImage.convertToFormat(QImage::Format_RGB32);
  }
  return bmpImage.save(filePath, "BMP");
}

bool ImageSaver::saveAsJPEG(const QImage& image, const QString& filePath,
                            int quality) {
  QImage jpegImage = image;
  if (jpegImage.hasAlphaChannel()) {
    jpegImage = jpegImage.convertToFormat(QImage::Format_RGB32);
  }
  return jpegImage.save(filePath, "JPEG", quality);
}

}  // namespace s21