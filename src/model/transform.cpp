#include "transform.h"

#include <cmath>

namespace s21 {

Transform3D::Transform3D(QObject *parent) : QObject(parent) {}

void Transform3D::setAxisAngles(float x, float y, float z) {
  // Вычисляем дельты изменений
  float dx = x - angleX;
  float dy = y - angleY;
  float dz = z - angleZ;

  // Если есть изменения
  const float eps = 0.0001f;
  if (std::abs(dx) > eps || std::abs(dy) > eps || std::abs(dz) > eps) {
    // Применяем каждый поворот относительно ГЛОБАЛЬНЫХ осей
    QQuaternion deltaRotation{};

    // Применяем все изменения как отдельные глобальные повороты
    if (std::abs(dx) > eps) {
      // Следования параметров x y z angle. x y z это векторы осей врашения,
      // диапазон от -1 до 1. Задает вращение на заданный угол.
      QQuaternion rotX = QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, dx);
      deltaRotation = rotX * deltaRotation;  // Глобальный X
    }

    if (std::abs(dy) > eps) {
      QQuaternion rotY = QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, dy);
      deltaRotation = rotY * deltaRotation;  // Глобальный Y
    }

    if (std::abs(dz) > eps) {
      QQuaternion rotZ = QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, dz);
      deltaRotation = rotZ * deltaRotation;  // Глобальный Z
    }

    // Поворот вокруг глобальной оси
    rotation = (deltaRotation * rotation).normalized();

    // Обновляем сохраненные углы
    angleX = x;
    angleY = y;
    angleZ = z;

    emit transChanged();
  }
}

void Transform3D::setOffsetX(float x) {
  offsetX = x;
  emit transChanged();
}
void Transform3D::setOffsetY(float y) {
  offsetY = y;
  emit transChanged();
}
void Transform3D::setOffsetZ(float z) {
  offsetZ = z;
  emit transChanged();
}

QMatrix4x4 Transform3D::getModelMatrix() {
  QMatrix4x4 modelMatrix;
  modelMatrix.translate(offsetX, offsetY, offsetZ);
  modelMatrix.rotate(rotation);
  return modelMatrix;
}

/**
 * @brief Настраивает матрицу вида для камеры
 */
void Transform3D::setupView() {
  // Сброс матрицы, получение единичной матрицы
  view.setToIdentity();
  // Задать положение камеры
  view.lookAt(QVector3D(0, 0, cameraDistance),  // Положение камеры
              QVector3D(0, 0, 0),   // Куда смотрит камера
              QVector3D(0, 1, 0));  // Ориентация сцены
}

/**
 * @brief Настраивает матрицу перспективы
 * @param w ширина окна
 * @param h высота окна
 */
void Transform3D::setProjection() {
  projection.setToIdentity();

  if (projectionType == ProjectionType::Perspective) {
    // Перспективная проекция
    projection.perspective(fieldOfView, aspectRatio, nearPlane, farPlane);
  } else {
    // Ортографическая проекция
    orthoSize = 2.0f * tan(fieldOfView * 0.5f * M_PI / 180.0f) *
                (cameraDistance - offsetZ);
    float halfSize = orthoSize / 2.0f;
    float halfWidth = halfSize * aspectRatio;
    float halfHeight = halfSize;

    projection.ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, nearPlane,
                     farPlane);
  }
}

void Transform3D::setProjectionType(ProjectionType type) {
  if (projectionType != type) {
    projectionType = type;
    emit transChanged();
  }
}

void Transform3D::resetTransform() {
  rotation = QQuaternion();
  offsetX = .0;
  offsetY = .0;
  offsetZ = .0;
  angleX = .0;
  angleY = .0;
  angleZ = .0;
  cameraDistance = .0;
  emit transChanged();
}

}  // namespace s21