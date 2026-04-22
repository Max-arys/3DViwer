#ifndef WCONTROLLER_H
#define WCONTROLLER_H

#include <QObject>
#include <memory>

#include "model/display_settings.h"
#include "model/model.h"
#include "model/transform.h"
#include "view/mainwindow.h"

namespace s21 {

class MainWindowController : public QObject {
  Q_OBJECT

 public:
  explicit MainWindowController(
      std::shared_ptr<Model3D> model, std::shared_ptr<Transform3D> trans3D,
      std::shared_ptr<DisplaySettings> displaySettings,
      QObject *parent = nullptr);

  void setView(MainWindow *view);

 public slots:
  void onLoadButtonClicked();
  void onProjectionTypeChanged(ProjectionType type);
  void onRotationXChanged(float value);
  void onRotationYChanged(float value);
  void onRotationZChanged(float value);

  void onOffsetXChanged(float value);
  void onOffsetYChanged(float value);
  void onOffsetZChanged(float value);

  void onResetClicked();

  // Слоты для настроек отображения
  void onBackgroundColorChanged(const QColor &color);
  void onVertexColorChanged(const QColor &color);
  void onVertexSizeChanged(float size);
  void onVertexTypeChanged(int type);
  void onEdgeColorChanged(const QColor &color);
  void onEdgeWidthChanged(float width);
  void onEdgeTypeChanged(int type);
  void onResetDisplaySettings();

 private slots:
  void onModelLoaded();
  void onTransChanged();
  void onDSChanged();
  void onModelError(const QString &message);

 private:
  void updateViewInfo();

  std::shared_ptr<Model3D> model;
  std::shared_ptr<Transform3D> trans3D;
  std::shared_ptr<DisplaySettings> displaySettings;
  MainWindow *view;

  QString currentFileName;
};  // class MainWindowController
}  // namespace s21

#endif  // WCONTROLLER_H