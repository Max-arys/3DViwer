#include <QApplication>
#include <memory>

#include "controller/wcontroller.h"
#include "gif_recorder.h"
#include "imagesaver.h"
#include "mainwindow.h"
#include "model/display_settings.h"
#include "model/model.h"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  auto model = std::make_shared<s21::Model3D>();
  auto trans3D = std::make_shared<s21::Transform3D>();
  auto displaySettings = std::make_shared<s21::DisplaySettings>();
  auto imageSaver = std::make_shared<s21::ImageSaver>();

  // GIFRecorder создаётся в MainWindow, передаём только GLWidget позже
  auto controller = std::make_shared<s21::MainWindowController>(
      model, trans3D, displaySettings);

  s21::MainWindow window(model, trans3D, displaySettings, controller,
                         imageSaver);
  window.setWindowTitle("3D Model Viewer - MVC Architecture");
  window.resize(1200, 700);
  window.show();

  return app.exec();
}