#include "mainwindow.h"

#include <QColorDialog>
#include <QComboBox>
#include <QDateTime>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QGroupBox>
#include <QHBoxLayout>  // Горизнтальный макет
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSlider>
#include <QSpacerItem>
#include <QSpinBox>
#include <QStatusBar>
#include <QTabWidget>
#include <QVBoxLayout>  // Вертикальный макет

#include "controller/wcontroller.h"
#include "view/gif_recorder.h"
#include "view/glwidget.h"
#include "view/imagesaver.h"

namespace s21 {

MainWindow::MainWindow(std::shared_ptr<Model3D> m,
                       std::shared_ptr<Transform3D> trans3D,
                       std::shared_ptr<DisplaySettings> dS,
                       std::shared_ptr<MainWindowController> c,
                       std::shared_ptr<ImageSaver> iS, QWidget *parent)
    : QMainWindow(parent),
      model(m),
      trans3D(trans3D),
      displaySettings(dS),
      controller(c),
      imageSaver(iS),
      currentFileName(""),
      ignoreSignals(false) {
  setupUI();
  createGifRecorder();
  setupConnections();

  controller->setView(this);
  showStatus("Готов к работе");
  updateModelInfo("", 0, 0);
  updateDSValues();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI() {
  setupCentralWidget();
  setupLayouts();
  setupInfoPanel(viewLayout);
  setupGLWidget(viewLayout);
  setupRightPanel();
}

void MainWindow::setupCentralWidget() {
  centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);
}

void MainWindow::setupLayouts() {
  mainLayout = new QHBoxLayout(centralWidget);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  viewContainer = new QWidget(this);
  viewLayout = new QVBoxLayout(viewContainer);
  viewLayout->setContentsMargins(5, 5, 5, 5);

  mainLayout->addWidget(viewContainer, 1);
}

void MainWindow::setupRightPanel() {
  QWidget *rightPanelContainer = new QWidget(this);
  QHBoxLayout *rightPanelLayout = new QHBoxLayout(rightPanelContainer);
  rightPanelLayout->setContentsMargins(0, 0, 0, 0);
  rightPanelLayout->setSpacing(0);

  rightTabWidget = new QTabWidget(this);
  rightTabWidget->setFixedWidth(320);
  setupRightTabWidgetStyle();

  createTransformTab();
  createDisplaySettingsTab();

  rightPanelLayout->addWidget(rightTabWidget);
  mainLayout->addWidget(rightPanelContainer);
}

void MainWindow::setupRightTabWidgetStyle() {
  rightTabWidget->setStyleSheet(
      "QTabWidget::pane { "
      "   border: 1px solid palette(mid); "
      "   border-radius: 4px; "
      "} "
      "QTabBar::tab { "
      "   padding: 8px 12px; "
      "   margin-right: 2px; "
      "   border: 1px solid transparent; "
      "   background-color: palette(window); "
      "} "
      "QTabBar::tab:selected { "
      "   border: 2px solid palette(highlight); "
      "   background-color: palette(light); "
      "}");
}

void MainWindow::createTransformTab() {
  QWidget *transformPanel = new QWidget();
  QVBoxLayout *transformLayout = new QVBoxLayout(transformPanel);
  transformLayout->setContentsMargins(10, 10, 10, 10);
  transformLayout->setSpacing(10);

  createLoadSaveControls(transformLayout);
  addSeparator(transformLayout);
  createProjectionControls(transformLayout);
  addSeparator(transformLayout);
  createRotationControls(transformLayout);
  createOffsetControls(transformLayout);

  resetButton = new QPushButton("🔄 Сброс трансформаций", this);
  transformLayout->addWidget(resetButton);
  transformLayout->addStretch();

  rightTabWidget->addTab(transformPanel, "🎛️ Трансформации");
}

void MainWindow::createLoadSaveControls(QVBoxLayout *layout) {
  loadButton = new QPushButton("📂 Загрузить модель", this);
  saveButton = new QPushButton("📸 Сохранить изображение", this);
  gifRecordButton = new QPushButton("🎬 Записать GIF", this);

  layout->addWidget(loadButton);
  layout->addWidget(saveButton);
  layout->addWidget(gifRecordButton);
}

void MainWindow::createProjectionControls(QVBoxLayout *layout) {
  QLabel *projectionLabel = new QLabel("Тип проекции", this);
  projectionLabel->setStyleSheet("font-weight: bold; font-size: 11pt;");
  layout->addWidget(projectionLabel);

  QHBoxLayout *projectionLayout = new QHBoxLayout();
  perspectiveRadio = new QRadioButton("Центральная", this);
  orthographicRadio = new QRadioButton("Параллельная", this);
  perspectiveRadio->setChecked(true);
  projectionLayout->addWidget(perspectiveRadio);
  projectionLayout->addWidget(orthographicRadio);
  projectionLayout->addStretch();
  layout->addLayout(projectionLayout);
}

void MainWindow::createRotationControls(QVBoxLayout *layout) {
  QLabel *rotationLabel = new QLabel("Повороты", this);
  rotationLabel->setStyleSheet(
      "font-weight: bold; font-size: 11pt; margin-top: 5px;");
  layout->addWidget(rotationLabel);

  QWidget *xControl = createAxisControl("X", sliderX, spinBoxX, -180, 180, 1.0);
  QWidget *yControl = createAxisControl("Y", sliderY, spinBoxY, -180, 180, 1.0);
  QWidget *zControl = createAxisControl("Z", sliderZ, spinBoxZ, -180, 180, 1.0);

  layout->addWidget(xControl);
  layout->addWidget(yControl);
  layout->addWidget(zControl);
}

void MainWindow::createOffsetControls(QVBoxLayout *layout) {
  QLabel *offsetLabel = new QLabel("Смещения", this);
  offsetLabel->setStyleSheet(
      "font-weight: bold; font-size: 11pt; margin-top: 10px;");
  layout->addWidget(offsetLabel);

  QWidget *xOffsetControl =
      createAxisControl("X", sliderOffsetX, spinBoxOffsetX, -10, 10, 0.1);
  QWidget *yOffsetControl =
      createAxisControl("Y", sliderOffsetY, spinBoxOffsetY, -10, 10, 0.1);
  QWidget *zOffsetControl =
      createAxisControl("Z", sliderOffsetZ, spinBoxOffsetZ, -10, 10, 0.1);

  layout->addWidget(xOffsetControl);
  layout->addWidget(yOffsetControl);
  layout->addWidget(zOffsetControl);
}

void MainWindow::createDisplaySettingsTab() {
  QWidget *displayPanel = new QWidget();
  QVBoxLayout *displayLayout = new QVBoxLayout(displayPanel);
  displayLayout->setContentsMargins(10, 10, 10, 10);
  displayLayout->setSpacing(10);

  createColorControls(displayLayout);
  addSeparator(displayLayout);
  createVertexControls(displayLayout);
  createEdgeControls(displayLayout);
  addSeparator(displayLayout);

  resetDisplayButton = new QPushButton("🔄 Сброс настроек отображения", this);
  resetDisplayButton->setMinimumHeight(35);
  displayLayout->addWidget(resetDisplayButton);
  displayLayout->addStretch();

  rightTabWidget->addTab(displayPanel, "🎨 Настройки");
}

void MainWindow::createColorControls(QVBoxLayout *layout) {
  bgColorBtn = new QPushButton("🎨 Цвет фона", this);
  vertexColorBtn = new QPushButton("🔴 Цвет вершин", this);
  edgeColorBtn = new QPushButton("📏 Цвет ребер", this);

  layout->addWidget(bgColorBtn);
  layout->addWidget(vertexColorBtn);
  layout->addWidget(edgeColorBtn);
}

void MainWindow::createVertexControls(QVBoxLayout *layout) {
  // Размер вершин
  QHBoxLayout *vertexSizeLayout = new QHBoxLayout();
  QLabel *vertexSizeLabel = new QLabel("Размер вершин:", this);
  vertexSizeLabel->setMinimumWidth(100);
  vertexSizeSpin = new QSpinBox(this);
  vertexSizeSpin->setRange(1, 20);
  vertexSizeSpin->setValue(8);
  vertexSizeSpin->setSuffix(" px");
  vertexSizeLayout->addWidget(vertexSizeLabel);
  vertexSizeLayout->addWidget(vertexSizeSpin);
  vertexSizeLayout->addStretch();
  layout->addLayout(vertexSizeLayout);

  // Тип вершин
  QHBoxLayout *vertexTypeLayout = new QHBoxLayout();
  QLabel *vertexTypeLabel = new QLabel("Тип вершин:", this);
  vertexTypeLabel->setMinimumWidth(100);
  vertexTypeCombo = new QComboBox(this);
  vertexTypeCombo->addItem("Нет", static_cast<int>(VertexDisplayType::None));
  vertexTypeCombo->addItem("Круг", static_cast<int>(VertexDisplayType::Circle));
  vertexTypeCombo->addItem("Квадрат",
                           static_cast<int>(VertexDisplayType::Square));
  vertexTypeLayout->addWidget(vertexTypeLabel);
  vertexTypeLayout->addWidget(vertexTypeCombo);
  vertexTypeLayout->addStretch();
  layout->addLayout(vertexTypeLayout);
}

void MainWindow::createEdgeControls(QVBoxLayout *layout) {
  // Толщина ребер
  QHBoxLayout *edgeWidthLayout = new QHBoxLayout();
  QLabel *edgeWidthLabel = new QLabel("Толщина ребер:", this);
  edgeWidthLabel->setMinimumWidth(100);
  edgeWidthSpin = new QDoubleSpinBox(this);
  edgeWidthSpin->setRange(0.5, 10.0);
  edgeWidthSpin->setSingleStep(0.5);
  edgeWidthSpin->setValue(1.0);
  edgeWidthSpin->setSuffix(" px");
  edgeWidthLayout->addWidget(edgeWidthLabel);
  edgeWidthLayout->addWidget(edgeWidthSpin);
  edgeWidthLayout->addStretch();
  layout->addLayout(edgeWidthLayout);

  // Тип ребер
  QHBoxLayout *edgeTypeLayout = new QHBoxLayout();
  QLabel *edgeTypeLabel = new QLabel("Тип ребер:", this);
  edgeTypeLabel->setMinimumWidth(100);
  edgeTypeCombo = new QComboBox(this);
  edgeTypeCombo->addItem("Сплошная", static_cast<int>(EdgeDisplayType::Solid));
  edgeTypeCombo->addItem("Штриховая",
                         static_cast<int>(EdgeDisplayType::Dashed));
  edgeTypeCombo->addItem("Пунктирная",
                         static_cast<int>(EdgeDisplayType::Dotted));
  edgeTypeLayout->addWidget(edgeTypeLabel);
  edgeTypeLayout->addWidget(edgeTypeCombo);
  edgeTypeLayout->addStretch();
  layout->addLayout(edgeTypeLayout);
}

void MainWindow::addSeparator(QVBoxLayout *layout) {
  QFrame *line = new QFrame();
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  layout->addWidget(line);
}

void MainWindow::setupGLWidget(QVBoxLayout *viewLayout) {
  glWidget = new GLWidget(this);
  glWidget->setModel(model);
  glWidget->setTransform(trans3D);
  glWidget->setDisplaySettings(displaySettings);
  viewLayout->addWidget(glWidget, 1);
}

void MainWindow::setupInfoPanel(QVBoxLayout *viewLayout) {
  QHBoxLayout *infoLayout = new QHBoxLayout();

  modelInfoLabel = new QLabel("Файл: —  Вершин: 0  Ребер: 0", this);
  statusLabel = new QLabel("", this);

  modelInfoLabel->setStyleSheet("font-weight: bold; font-size: 11pt;");
  modelInfoLabel->setAlignment(Qt::AlignCenter);

  infoLayout->addWidget(modelInfoLabel);
  infoLayout->addStretch();
  infoLayout->addWidget(statusLabel);
  viewLayout->addLayout(infoLayout);
}

QWidget *MainWindow::createAxisControl(const QString &axisName,
                                       QSlider *&slider,
                                       QDoubleSpinBox *&spinBox, float min,
                                       float max, float step) {
  QWidget *widget = new QWidget(this);
  QHBoxLayout *layout = new QHBoxLayout(widget);
  layout->setContentsMargins(5, 2, 5, 2);

  QLabel *nameLabel = new QLabel(axisName, this);
  nameLabel->setFixedWidth(40);
  nameLabel->setAlignment(Qt::AlignCenter);
  layout->addWidget(nameLabel);

  slider = new QSlider(Qt::Horizontal, this);
  slider->setRange(static_cast<int>(min), static_cast<int>(max));
  slider->setValue(0);
  slider->setTickPosition(QSlider::TicksBelow);
  slider->setTickInterval(45);
  layout->addWidget(slider, 2);

  spinBox = new QDoubleSpinBox(this);
  spinBox->setRange(min, max);
  spinBox->setValue(0);
  spinBox->setSingleStep(step);
  spinBox->setSuffix(
      axisName.contains("X") ? "" : (axisName.contains("Y") ? "" : ""));
  if (axisName.contains("Смещения")) {
    spinBox->setSuffix("");
  } else {
    spinBox->setSuffix("°");
  }
  spinBox->setFixedWidth(85);
  spinBox->setAlignment(Qt::AlignRight);
  layout->addWidget(spinBox);

  QObject::connect(slider, &QSlider::valueChanged, spinBox,
                   [spinBox](int value) {
                     spinBox->blockSignals(true);
                     spinBox->setValue(value);
                     spinBox->blockSignals(false);
                   });

  QObject::connect(spinBox,
                   QOverload<double>::of(&QDoubleSpinBox::valueChanged), slider,
                   [slider](double value) {
                     slider->blockSignals(true);
                     slider->setValue(static_cast<int>(value));
                     slider->blockSignals(false);
                   });

  return widget;
}

void MainWindow::updateTransValues(float rotX, float rotY, float rotZ,
                                   float ofX, float ofY, float ofZ) {
  ignoreSignals = true;

  sliderX->setValue(static_cast<int>(rotX));
  sliderY->setValue(static_cast<int>(rotY));
  sliderZ->setValue(static_cast<int>(rotZ));

  sliderOffsetX->setValue(static_cast<int>(ofX));
  sliderOffsetY->setValue(static_cast<int>(ofY));
  sliderOffsetZ->setValue(static_cast<int>(ofZ));

  ignoreSignals = false;
}

void MainWindow::updateDSValues() {
  ignoreSignals = true;

  // Обновляем значения в UI из настроек
  vertexSizeSpin->setValue(static_cast<int>(displaySettings->getVertexSize()));

  int vertexTypeIndex = vertexTypeCombo->findData(
      static_cast<int>(displaySettings->getVertexType()));
  if (vertexTypeIndex >= 0) vertexTypeCombo->setCurrentIndex(vertexTypeIndex);

  edgeWidthSpin->setValue(displaySettings->getEdgeWidth());

  int edgeTypeIndex =
      edgeTypeCombo->findData(static_cast<int>(displaySettings->getEdgeType()));
  if (edgeTypeIndex >= 0) edgeTypeCombo->setCurrentIndex(edgeTypeIndex);

  ignoreSignals = false;
}

void MainWindow::updateModelInfo(const QString &fileName, size_t vertices,
                                 size_t edges) {
  currentFileName = fileName;
  QString displayName =
      fileName.isEmpty() ? "—" : QFileInfo(fileName).fileName();
  modelInfoLabel->setText(QString("Файл: %1  |  Вершин: %2  |  Ребер: %3")
                              .arg(displayName)
                              .arg(vertices)
                              .arg(edges));
}

void MainWindow::showError(const QString &message) {
  statusLabel->setText("❌ " + message);
  statusLabel->setStyleSheet("color: red; font-weight: bold;");
}

void MainWindow::showStatus(const QString &message) {
  statusLabel->setText("✓ " + message);
  statusLabel->setStyleSheet("color: green; font-weight: bold;");
}

void MainWindow::onSaveScreenshot() {
  if (!glWidget) {
    showError("Нет виджета для сохранения");
    return;
  }

  // Создаем диалог выбора формата
  QDialog formatDialog(this);
  formatDialog.setWindowTitle("Выберите формат");

  QVBoxLayout layout(&formatDialog);

  QLabel label("Выберите формат изображения:", &formatDialog);
  layout.addWidget(&label);

  QComboBox formatCombo(&formatDialog);
  formatCombo.addItem("BMP", "bmp");
  formatCombo.addItem("JPEG", "jpg");
  layout.addWidget(&formatCombo);

  QPushButton okBtn("OK", &formatDialog);
  QPushButton cancelBtn("Отмена", &formatDialog);

  QHBoxLayout btnLayout;
  btnLayout.addWidget(&okBtn);
  btnLayout.addWidget(&cancelBtn);
  layout.addLayout(&btnLayout);

  connect(&okBtn, &QPushButton::clicked, &formatDialog, &QDialog::accept);
  connect(&cancelBtn, &QPushButton::clicked, &formatDialog, &QDialog::reject);

  if (formatDialog.exec() != QDialog::Accepted) {
    return;
  }

  QString format = formatCombo.currentData().toString();

  // Открываем диалог выбора файла
  QString fileName = QFileDialog::getSaveFileName(
      this, "Сохранить изображение", QString("screenshot.%1").arg(format),
      QString("%1 файлы (*.%2);;Все файлы (*)")
          .arg(format.toUpper())
          .arg(format));

  if (fileName.isEmpty()) {
    return;
  }

  // Получаем изображение из GLWidget в текущем разрешении
  QImage screenshot = glWidget->grabFramebuffer();

  if (screenshot.isNull()) {
    showError("Не удалось получить изображение");
    return;
  }

  // Сохраняем изображение
  if (imageSaver->saveImage(screenshot, fileName)) {
    showStatus(QString("Изображение сохранено: %1")
                   .arg(QFileInfo(fileName).fileName()));
  } else {
    showError("Не удалось сохранить изображение");
  }
}

void MainWindow::onGifRecordClicked() {
  if (!glWidget || !model || model->isEmpty()) {
    showError("Нет загруженной модели для записи");
    return;
  }

  if (gifRecorder->isRecording()) {
    gifRecorder->stopRecording();
    return;
  }

  // Диалог настройки параметров записи
  QDialog settingsDialog(this);
  settingsDialog.setWindowTitle("Настройки записи GIF");
  settingsDialog.setFixedSize(300, 200);

  QVBoxLayout layout(&settingsDialog);

  // FPS
  QHBoxLayout fpsLayout;
  QLabel fpsLabel("FPS (кадров/сек):", &settingsDialog);
  QSpinBox fpsSpin(&settingsDialog);
  fpsSpin.setRange(5, 30);
  fpsSpin.setValue(15);
  fpsSpin.setSuffix(" fps");
  fpsLayout.addWidget(&fpsLabel);
  fpsLayout.addWidget(&fpsSpin);
  layout.addLayout(&fpsLayout);

  // Длительность
  QHBoxLayout durationLayout;
  QLabel durationLabel("Длительность:", &settingsDialog);
  QSpinBox durationSpin(&settingsDialog);
  durationSpin.setRange(1, 60);
  durationSpin.setValue(5);
  durationSpin.setSuffix(" сек");
  durationLayout.addWidget(&durationLabel);
  durationLayout.addWidget(&durationSpin);
  layout.addLayout(&durationLayout);

  // Кнопки
  QHBoxLayout btnLayout;
  QPushButton okBtn("Начать запись", &settingsDialog);
  QPushButton cancelBtn("Отмена", &settingsDialog);
  btnLayout.addWidget(&okBtn);
  btnLayout.addWidget(&cancelBtn);
  layout.addLayout(&btnLayout);

  connect(&okBtn, &QPushButton::clicked, &settingsDialog, &QDialog::accept);
  connect(&cancelBtn, &QPushButton::clicked, &settingsDialog, &QDialog::reject);

  if (settingsDialog.exec() != QDialog::Accepted) {
    return;
  }

  // Выбор пути сохранения
  QString defaultName =
      QString("animation_%1.gif")
          .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
  QString filepath = QFileDialog::getSaveFileName(
      this, "Сохранить GIF анимацию", defaultName, "GIF Files (*.gif)");

  if (filepath.isEmpty()) {
    return;
  }

  // Запуск записи с новым API
  int fps = fpsSpin.value();
  int durationMs = durationSpin.value() * 1000;
  gifRecorder->startRecording(filepath, fps, durationMs);
}

void MainWindow::onGifRecordingStarted() {
  gifRecordButton->setText("⏹️ Остановить запись");
  gifRecordButton->setStyleSheet(
      "QPushButton {"
      "  background-color: #f44336;"
      "  color: white;"
      "  font-weight: bold;"
      "  border-radius: 4px;"
      "  padding: 8px;"
      "}"
      "QPushButton:hover {"
      "  background-color: #d32f2f;"
      "}");
  showStatus("🎬 Запись GIF анимации...");
}

void MainWindow::onGifRecordingStopped(const QString &filepath) {
  gifRecordButton->setText("🎬 Записать GIF");
  gifRecordButton->setStyleSheet("");  // Возвращаем стандартный стиль
  showStatus(
      QString("✅ GIF сохранён: %1").arg(QFileInfo(filepath).fileName()));

  // Показываем уведомление
  QMessageBox::information(
      this, "GIF записан",
      QString("Анимация успешно сохранена:\n%1").arg(filepath));
}

void MainWindow::onGifRecordingProgress(int elapsedMs, int totalMs) {
  int percent = (elapsedMs * 100) / totalMs;
  int secondsRemaining = (totalMs - elapsedMs) / 1000;

  showStatus(QString("🎬 Запись: %1% (%2 сек осталось)")
                 .arg(percent)
                 .arg(secondsRemaining));
}

void MainWindow::onGifRecordingError(const QString &error) {
  showError("Ошибка GIF: " + error);
  gifRecordButton->setText("🎬 Записать GIF");
  gifRecordButton->setStyleSheet("");

  QMessageBox::critical(this, "Ошибка записи GIF", error);
}

void MainWindow::createGifRecorder() {
  gifRecorder = std::make_shared<GIFRecorder>(glWidget, this);
  setupGIFRecorderConnections();
}

// ==================== НАСТРОЙКА СОЕДИНЕНИЙ ====================
void MainWindow::setupConnections() {
  setupTransformConnections();
  setupDisplaySettingsConnections();
  setupGIFRecorderConnections();
}

void MainWindow::setupTransformConnections() {
  connect(loadButton, &QPushButton::clicked, controller.get(),
          &MainWindowController::onLoadButtonClicked);
  connect(saveButton, &QPushButton::clicked, this,
          &MainWindow::onSaveScreenshot);
  connect(gifRecordButton, &QPushButton::clicked, this,
          &MainWindow::onGifRecordClicked);
  connect(resetButton, &QPushButton::clicked, controller.get(),
          &MainWindowController::onResetClicked);

  connect(perspectiveRadio, &QRadioButton::toggled, this, [this](bool checked) {
    if (checked)
      controller->onProjectionTypeChanged(ProjectionType::Perspective);
  });
  connect(orthographicRadio, &QRadioButton::toggled, this,
          [this](bool checked) {
            if (checked)
              controller->onProjectionTypeChanged(ProjectionType::Orthographic);
          });

  connect(sliderX, &QSlider::valueChanged, this, [this](int value) {
    if (!ignoreSignals)
      controller->onRotationXChanged(static_cast<float>(value));
  });
  connect(sliderY, &QSlider::valueChanged, this, [this](int value) {
    if (!ignoreSignals)
      controller->onRotationYChanged(static_cast<float>(value));
  });
  connect(sliderZ, &QSlider::valueChanged, this, [this](int value) {
    if (!ignoreSignals)
      controller->onRotationZChanged(static_cast<float>(value));
  });

  connect(sliderOffsetX, &QSlider::valueChanged, this, [this](int value) {
    if (!ignoreSignals) controller->onOffsetXChanged(static_cast<float>(value));
  });
  connect(sliderOffsetY, &QSlider::valueChanged, this, [this](int value) {
    if (!ignoreSignals) controller->onOffsetYChanged(static_cast<float>(value));
  });
  connect(sliderOffsetZ, &QSlider::valueChanged, this, [this](int value) {
    if (!ignoreSignals) controller->onOffsetZChanged(static_cast<float>(value));
  });

  connect(spinBoxX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
          [this](double value) {
            if (!ignoreSignals)
              controller->onRotationXChanged(static_cast<float>(value));
          });
  connect(spinBoxY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
          [this](double value) {
            if (!ignoreSignals)
              controller->onRotationYChanged(static_cast<float>(value));
          });
  connect(spinBoxZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
          [this](double value) {
            if (!ignoreSignals)
              controller->onRotationZChanged(static_cast<float>(value));
          });

  connect(spinBoxOffsetX, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, [this](double value) {
            if (!ignoreSignals)
              controller->onOffsetXChanged(static_cast<float>(value));
          });
  connect(spinBoxOffsetY, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, [this](double value) {
            if (!ignoreSignals)
              controller->onOffsetYChanged(static_cast<float>(value));
          });
  connect(spinBoxOffsetZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, [this](double value) {
            if (!ignoreSignals)
              controller->onOffsetZChanged(static_cast<float>(value));
          });
}

void MainWindow::setupDisplaySettingsConnections() {
  connect(bgColorBtn, &QPushButton::clicked, this, [this]() {
    QColor color =
        QColorDialog::getColor(displaySettings->getBackgroundColor(), this);
    if (color.isValid()) controller->onBackgroundColorChanged(color);
  });

  connect(vertexColorBtn, &QPushButton::clicked, this, [this]() {
    QColor color =
        QColorDialog::getColor(displaySettings->getVertexColor(), this);
    if (color.isValid()) controller->onVertexColorChanged(color);
  });

  connect(edgeColorBtn, &QPushButton::clicked, this, [this]() {
    QColor color =
        QColorDialog::getColor(displaySettings->getEdgeColor(), this);
    if (color.isValid()) controller->onEdgeColorChanged(color);
  });

  connect(vertexSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
          [this](int value) {
            controller->onVertexSizeChanged(static_cast<float>(value));
          });

  connect(vertexTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, [this](int index) {
            int type = vertexTypeCombo->itemData(index).toInt();
            controller->onVertexTypeChanged(type);
          });

  connect(edgeWidthSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, [this](double value) {
            controller->onEdgeWidthChanged(static_cast<float>(value));
          });

  connect(edgeTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, [this](int index) {
            int type = edgeTypeCombo->itemData(index).toInt();
            controller->onEdgeTypeChanged(type);
          });

  connect(resetDisplayButton, &QPushButton::clicked, controller.get(),
          &MainWindowController::onResetDisplaySettings);
}

void MainWindow::setupGIFRecorderConnections() {
  connect(gifRecorder.get(), &GIFRecorder::recordingStarted, this,
          &MainWindow::onGifRecordingStarted);
  connect(gifRecorder.get(), &GIFRecorder::recordingStopped, this,
          &MainWindow::onGifRecordingStopped);
  connect(gifRecorder.get(), &GIFRecorder::recordingProgress, this,
          &MainWindow::onGifRecordingProgress);
  connect(gifRecorder.get(), &GIFRecorder::recordingError, this,
          &MainWindow::onGifRecordingError);
}

}  // namespace s21