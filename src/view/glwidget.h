#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QCoreApplication>
#include <QFileInfo>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <memory>

#include "../model/display_settings.h"
#include "../model/model.h"
#include "../model/transform.h"

namespace s21 {

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
  Q_OBJECT

 public:
  explicit GLWidget(QWidget *parent = nullptr);
  ~GLWidget();

  void setModel(std::shared_ptr<Model3D> model);
  void setTransform(std::shared_ptr<Transform3D> trans3D);
  void setDisplaySettings(std::shared_ptr<DisplaySettings> settings);

 protected slots:
  void onModelChanged();

 protected:
  void initializeGL() override;
  void paintGL() override;
  void resizeGL(int w, int h) override;

 private:
  GLenum mapEdgeTypeToGL(EdgeDisplayType type);
  GLenum mapVertexTypeToGL(VertexDisplayType type);

  void setupShaders();
  void updateBuffers();
  void calculateBoundingBox();

  std::shared_ptr<Model3D> model;
  std::shared_ptr<Transform3D> trans3D;
  std::shared_ptr<DisplaySettings> displaySettings;

  QOpenGLShaderProgram *shaderProgram;
  QOpenGLBuffer vbo;
  QOpenGLBuffer edgeBuffer;
  QOpenGLVertexArrayObject vao;

  bool buffersInitialized;

  QVector3D minBounds;
  QVector3D maxBounds;

};  // class GLWidget
}  // namespace s21

#endif  // GLWIDGET_H