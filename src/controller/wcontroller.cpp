#include "controller/wcontroller.h"

#include <QColor>
#include <QFileDialog>
#include <QFileInfo>

#include "view/mainwindow.h"

namespace s21 {

MainWindowController::MainWindowController(std::shared_ptr<Model3D> m,
                                           std::shared_ptr<Transform3D> t,
                                           std::shared_ptr<DisplaySettings> dS,
                                           QObject* parent)
    : QObject(parent),
      model(m),
      trans3D(t),
      displaySettings(dS),
      view(nullptr),
      currentFileName("") {
  connect(model.get(), &Model3D::modelLoaded, this,
          &MainWindowController::onModelLoaded);
  connect(trans3D.get(), &Transform3D::transChanged, this,
          &MainWindowController::onTransChanged);
  // ИСПРАВЛЕНИЕ: убраны лишние скобки
  connect(displaySettings.get(), &DisplaySettings::dSChanged, this,
          &MainWindowController::onDSChanged);
  connect(model.get(), &Model3D::errorOccurred, this,
          &MainWindowController::onModelError);
}

void MainWindowController::setView(MainWindow* v) { view = v; }

void MainWindowController::onLoadButtonClicked() {
  QString fileName = QFileDialog::getOpenFileName(view, "Выберите 3D модель",
                                                  "", "3D Files (*.obj)");

  if (!fileName.isEmpty()) {
    currentFileName = fileName;
    model->loadFromFile(fileName.toStdString());
    trans3D->setAxisAngles(.0, .0, .0);
    trans3D->resetTransform();
  }
}

void MainWindowController::onProjectionTypeChanged(ProjectionType type) {
  trans3D->setProjectionType(type);
}

void MainWindowController::onRotationXChanged(float value) {
  float currentY = trans3D->getRotationY();
  float currentZ = trans3D->getRotationZ();
  trans3D->setAxisAngles(value, currentY, currentZ);
}

void MainWindowController::onRotationYChanged(float value) {
  float currentX = trans3D->getRotationX();
  float currentZ = trans3D->getRotationZ();
  trans3D->setAxisAngles(currentX, value, currentZ);
}

void MainWindowController::onRotationZChanged(float value) {
  float currentX = trans3D->getRotationX();
  float currentY = trans3D->getRotationY();
  trans3D->setAxisAngles(currentX, currentY, value);
}

void MainWindowController::onOffsetXChanged(float value) {
  trans3D->setOffsetX(value);
}
void MainWindowController::onOffsetYChanged(float value) {
  trans3D->setOffsetY(value);
}
void MainWindowController::onOffsetZChanged(float value) {
  trans3D->setOffsetZ(value);
}

void MainWindowController::onResetClicked() { trans3D->resetTransform(); }

// Новые слоты для настроек отображения
void MainWindowController::onBackgroundColorChanged(const QColor& color) {
  displaySettings->setBackgroundColor(color);
}

void MainWindowController::onVertexColorChanged(const QColor& color) {
  displaySettings->setVertexColor(color);
}

void MainWindowController::onVertexSizeChanged(float size) {
  displaySettings->setVertexSize(size);
}

void MainWindowController::onVertexTypeChanged(int type) {
  displaySettings->setVertexType(static_cast<VertexDisplayType>(type));
}

void MainWindowController::onEdgeColorChanged(const QColor& color) {
  displaySettings->setEdgeColor(color);
}

void MainWindowController::onEdgeWidthChanged(float width) {
  displaySettings->setEdgeWidth(width);
}

void MainWindowController::onEdgeTypeChanged(int type) {
  displaySettings->setEdgeType(static_cast<EdgeDisplayType>(type));
}

void MainWindowController::onResetDisplaySettings() {
  displaySettings->resetToDefaults();
}

void MainWindowController::onTransChanged() {
  if (view) {
    view->updateTransValues(trans3D->getRotationX(), trans3D->getRotationY(),
                            trans3D->getRotationZ(), trans3D->getOffsetX(),
                            trans3D->getOffsetY(), trans3D->getOffsetZ());
  }
}

// ИСПРАВЛЕНИЕ: метод onDSChanged теперь правильно обновляет настройки
void MainWindowController::onDSChanged() {
  if (view) {
    view->updateDSValues();
  }
}

void MainWindowController::updateViewInfo() {
  if (!view) return;

  size_t edgeCount = model->getEdges().size() / 2;

  view->updateModelInfo(currentFileName, model->getVertices().size(),
                        edgeCount);
}

void MainWindowController::onModelLoaded() {
  if (view) {
    updateViewInfo();
    view->showStatus("Модель загружена успешно");
    view->updateTransValues(trans3D->getRotationX(), trans3D->getRotationY(),
                            trans3D->getRotationZ(), trans3D->getOffsetX(),
                            trans3D->getOffsetY(), trans3D->getOffsetZ());
  }
}

void MainWindowController::onModelError(const QString& message) {
  if (view) {
    view->showError(message);
    currentFileName = "";
    updateViewInfo();
  }
}

}  // namespace s21