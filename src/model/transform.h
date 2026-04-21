#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <QMatrix4x4>
#include <QObject>

namespace s21 {

enum class ProjectionType {
  Perspective,  // Центральная (перспективная)
  Orthographic  // Параллельная (ортографическая)
};

class Transform3D : public QObject {
  Q_OBJECT

 signals:
  void transChanged();

 public:
  explicit Transform3D(QObject* parent = nullptr);

  float getRotationX() const { return angleX; }
  float getRotationY() const { return angleY; }
  float getRotationZ() const { return angleZ; }

  float getOffsetX() const { return offsetX; }
  float getOffsetY() const { return offsetY; }
  float getOffsetZ() const { return offsetZ; }

  ProjectionType getProjectionType() const { return projectionType; }
  float getAspectRatio() const { return aspectRatio; }

  QMatrix4x4 getModelMatrix();
  QMatrix4x4 getProjectionMatrix() { return projection; }
  QMatrix4x4 getViewMatrix() { return view; }

  float getCameraDistance() const { return cameraDistance; }

  void setAxisAngles(float x, float y, float z);

  void setOffsetX(float x);
  void setOffsetY(float y);
  void setOffsetZ(float z);

  void resetTransform();
  void setCameraDistance(float d) { cameraDistance = d; }

  void setProjectionType(ProjectionType type);
  void setAspectRatio(float ar) { aspectRatio = ar; }

  void setupView();
  void setProjection();

 private:
  // Трансформационные параметры
  float offsetX{};
  float offsetY{};
  float offsetZ{};
  float angleX{};
  float angleY{};
  float angleZ{};
  float cameraDistance{5.0f};  // Расстояние до центра модели

  // Параметры проекции
  ProjectionType projectionType{ProjectionType::Perspective};
  float fieldOfView{45.0f};  // Поле зрения для перспективы (в градусах)
  float orthoSize{10.0f};  // Размер для ортографической проекции
  float nearPlane{0.1f};  // Ближняя плоскость отсечения
  float farPlane{1000.0f};  // Дальняя плоскость отсечения
  float aspectRatio{1};     // Соотношение сторон

  QQuaternion rotation;
  QMatrix4x4 projection;
  QMatrix4x4 view;

};  // class Transform3D

}  // namespace s21

#endif  // TRANSFORM_H