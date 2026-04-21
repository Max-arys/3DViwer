#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QRadioButton>
#include <memory>

class QSlider;
class QPushButton;
class QRadioButton;
class QLabel;
class QDoubleSpinBox;
class QVBoxLayout;
class QComboBox;
class QSpinBox;
class QTabWidget;
class QVBoxLayout;
class QHBoxLayout;

namespace s21 {
class GLWidget;
class Model3D;
class MainWindowController;
class Transform3D;
class DisplaySettings;
class ImageSaver;
class GIFRecorder;

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(std::shared_ptr<Model3D> model,
                      std::shared_ptr<Transform3D> trans3D,
                      std::shared_ptr<DisplaySettings> displaySettings,
                      std::shared_ptr<MainWindowController> controller,
                      std::shared_ptr<ImageSaver> imageSaver,
                      QWidget *parent = nullptr);
  ~MainWindow();

  // Методы для обновления UI (вызываются контроллером)
  void updateTransValues(float rotX, float rotY, float rotZ, float ofX,
                         float ofY, float ofZ);
  void updateDSValues();
  void updateModelInfo(const QString &fileName, size_t vertices, size_t edges);
  void showError(const QString &message);
  void showStatus(const QString &message);

 private:
  void onSaveScreenshot();
  void onGifRecordClicked();
  void onGifRecordingStarted();
  void onGifRecordingStopped(const QString &filepath);
  void onGifRecordingProgress(int elapsedMs, int totalMs);
  void onGifRecordingError(const QString &error);

  void createGifRecorder();

  void setupUI();
  // Вспомогательные методы для создания UI
  void setupCentralWidget();
  void setupLayouts();
  void setupInfoPanel(QVBoxLayout *viewLayout);
  void setupGLWidget(QVBoxLayout *viewLayout);
  void setupRightPanel();

  // Создание вкладок
  void createTransformTab();
  void createDisplaySettingsTab();

  // Создание групп элементов
  void createLoadSaveControls(QVBoxLayout *layout);
  void createProjectionControls(QVBoxLayout *layout);
  void createRotationControls(QVBoxLayout *layout);
  void createOffsetControls(QVBoxLayout *layout);
  void createColorControls(QVBoxLayout *layout);
  void createVertexControls(QVBoxLayout *layout);
  void createEdgeControls(QVBoxLayout *layout);

  // Настройка соединений (только подключение сигналов)
  void setupConnections();
  void setupTransformConnections();
  void setupDisplaySettingsConnections();
  void setupGIFRecorderConnections();

  void createControlPanel();
  void createDisplaySettingsPanel();

  // Создание элементов управления для одной оси
  QWidget *createAxisControl(const QString &axisName, QSlider *&slider,
                             QDoubleSpinBox *&spinBox, float min, float max,
                             float step);

  void setupRightTabWidgetStyle();
  void addSeparator(QVBoxLayout *layout);

  std::shared_ptr<Model3D> model;
  std::shared_ptr<Transform3D> trans3D;
  std::shared_ptr<DisplaySettings> displaySettings;
  std::shared_ptr<MainWindowController> controller;
  std::shared_ptr<ImageSaver> imageSaver;
  std::shared_ptr<GIFRecorder> gifRecorder;

  QWidget *centralWidget;
  QHBoxLayout *mainLayout;
  QWidget *viewContainer;
  QVBoxLayout *viewLayout;

  GLWidget *glWidget;
  QTabWidget *rightTabWidget;

  // Элементы управления для 3 осей
  QSlider *sliderX, *sliderY, *sliderZ;
  QSlider *sliderOffsetX, *sliderOffsetY, *sliderOffsetZ;

  QDoubleSpinBox *spinBoxX, *spinBoxY, *spinBoxZ;
  QDoubleSpinBox *spinBoxOffsetX, *spinBoxOffsetY, *spinBoxOffsetZ;

  QPushButton *loadButton;
  QPushButton *saveButton;
  QPushButton *gifRecordButton;
  QPushButton *resetButton;

  QRadioButton *perspectiveRadio;
  QRadioButton *orthographicRadio;

  // Элементы управления для настроек отображения
  QComboBox *vertexTypeCombo;
  QComboBox *edgeTypeCombo;
  QSpinBox *vertexSizeSpin;
  QDoubleSpinBox *edgeWidthSpin;
  QPushButton *resetDisplayButton;
  QPushButton *bgColorBtn;
  QPushButton *vertexColorBtn;
  QPushButton *edgeColorBtn;

  QLabel *statusLabel;
  QLabel *modelInfoLabel;
  QString currentFileName;

  bool ignoreSignals;
};  // class MainWindow
}  // namespace s21

#endif  // MAINWINDOW_H