#ifndef IMAGESAVER_H
#define IMAGESAVER_H

#include <QImage>
#include <QString>

namespace s21 {

class ImageSaver {
 public:
  ImageSaver();
  ~ImageSaver();

  bool saveImage(const QImage& image, const QString& filePath);

 private:
  bool saveAsBMP(const QImage& image, const QString& filePath);
  bool saveAsJPEG(const QImage& image, const QString& filePath,
                  int quality = 95);
};  // class ImageSaver

}  // namespace s21

#endif  // IMAGESAVER_H