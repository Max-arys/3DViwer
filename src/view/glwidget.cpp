#include "glwidget.h"

#include <QDebug>
#include <cmath>

namespace s21 {

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      shaderProgram(nullptr),
      vbo(QOpenGLBuffer::VertexBuffer),
      edgeBuffer(QOpenGLBuffer::IndexBuffer),
      buffersInitialized(false) {}

GLWidget::~GLWidget() {
  makeCurrent();
  vbo.destroy();
  edgeBuffer.destroy();
  vao.destroy();
  delete shaderProgram;
  doneCurrent();
}

void GLWidget::setModel(std::shared_ptr<Model3D> newModel) {
  if (model) {
    disconnect(model.get(), nullptr, this, nullptr);
  }

  model = newModel;

  if (model) {
    connect(model.get(), &Model3D::modelLoaded, this,
            &GLWidget::onModelChanged);
    updateBuffers();
  }
}

void GLWidget::setTransform(std::shared_ptr<Transform3D> newt) {
  if (trans3D) {
    disconnect(trans3D.get(), nullptr, this, nullptr);
  }

  trans3D = newt;

  if (trans3D) {
    connect(trans3D.get(), &Transform3D::transChanged, this,
            &GLWidget::onModelChanged);
    updateBuffers();
  }
}

void GLWidget::setDisplaySettings(
    std::shared_ptr<DisplaySettings> new_settings) {
  if (new_settings) {
    disconnect(new_settings.get(), nullptr, this, nullptr);
  }

  displaySettings = new_settings;

  if (displaySettings) {
    connect(displaySettings.get(), &DisplaySettings::dSChanged, this,
            &GLWidget::onModelChanged);
    updateBuffers();
  }
}

void GLWidget::calculateBoundingBox() {
  if (!model || model->getVertices().empty()) {
    minBounds = QVector3D(-1, -1, -1);
    maxBounds = QVector3D(1, 1, 1);
    return;
  }

  const auto &vertices = model->getVertices();

  minBounds = vertices[0].position;
  maxBounds = vertices[0].position;

  for (const auto &vertex : vertices) {
    float x = vertex.position.x();
    float y = vertex.position.y();
    float z = vertex.position.z();

    minBounds.setX(std::min(minBounds.x(), x));
    minBounds.setY(std::min(minBounds.y(), y));
    minBounds.setZ(std::min(minBounds.z(), z));

    maxBounds.setX(std::max(maxBounds.x(), x));
    maxBounds.setY(std::max(maxBounds.y(), y));
    maxBounds.setZ(std::max(maxBounds.z(), z));
  }

  QVector3D size = maxBounds - minBounds;
  float modelSize = std::max({size.x(), size.y(), size.z()});

  trans3D->setCameraDistance(modelSize * 1.5f);

  qDebug() << "Model size:" << modelSize
           << "Camera distance:" << trans3D->getCameraDistance();
}

void GLWidget::initializeGL() {
  initializeOpenGLFunctions();

  QColor bgColor = displaySettings->getBackgroundColor();
  glClearColor(bgColor.redF(), bgColor.greenF(), bgColor.blueF(),
               bgColor.alphaF());
  glEnable(GL_DEPTH_TEST);
  glPointSize(displaySettings->getVertexSize());

  setupShaders();
}

void GLWidget::setupShaders() {
  shaderProgram = new QOpenGLShaderProgram();

  shaderProgram->addShaderFromSourceCode(
      QOpenGLShader::Vertex,
      "#version 330\n"
      "layout(location = 0) in vec3 position;\n"
      "uniform mat4 model;\n"
      "uniform mat4 view;\n"
      "uniform mat4 projection;\n"
      "void main() {\n"
      "    gl_Position = projection * view * model * vec4(position, 1.0);\n"
      "}\n");

  shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                         "#version 330\n"
                                         "out vec4 color;\n"
                                         "uniform bool isVertex;\n"
                                         "uniform vec4 vertexColor;\n"
                                         "uniform vec4 edgeColor;\n"
                                         "void main() {\n"
                                         "    if (isVertex) {\n"
                                         "        color = vertexColor;\n"
                                         "    } else {\n"
                                         "        color = edgeColor;\n"
                                         "    }\n"
                                         "}\n");

  shaderProgram->link();
}

void GLWidget::updateBuffers() {
  if (!model || model->isEmpty()) {
    return;
  }

  makeCurrent();

  if (vao.isCreated()) {
    vao.destroy();
  }

  vao.create();
  vao.bind();

  vbo.create();
  vbo.bind();
  vbo.allocate(model->getVertices().data(),
               model->getVertices().size() * sizeof(Vertex));

  edgeBuffer.create();
  edgeBuffer.bind();
  edgeBuffer.allocate(model->getEdges().data(),
                      model->getEdges().size() * sizeof(unsigned int));

  shaderProgram->enableAttributeArray(0);
  shaderProgram->setAttributeBuffer(0, GL_FLOAT, offsetof(Vertex, position), 3,
                                    sizeof(Vertex));

  vao.release();
  vbo.release();
  edgeBuffer.release();

  buffersInitialized = true;
}

void GLWidget::paintGL() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (!model || model->isEmpty() || !buffersInitialized) {
    return;
  }

  shaderProgram->bind();
  vao.bind();

  QMatrix4x4 modelMatrix = trans3D->getModelMatrix();
  QMatrix4x4 projectionMatrix = trans3D->getProjectionMatrix();
  QMatrix4x4 viewMatrix = trans3D->getViewMatrix();

  shaderProgram->setUniformValue("model", modelMatrix);
  shaderProgram->setUniformValue("view", viewMatrix);
  shaderProgram->setUniformValue("projection", projectionMatrix);

  // Установка цвета
  QColor vertexColor = displaySettings->getVertexColor();
  QColor edgeColor = displaySettings->getEdgeColor();

  shaderProgram->setUniformValue(
      "vertexColor", QVector4D(vertexColor.redF(), vertexColor.greenF(),
                               vertexColor.blueF(), vertexColor.alphaF()));
  shaderProgram->setUniformValue(
      "edgeColor", QVector4D(edgeColor.redF(), edgeColor.greenF(),
                             edgeColor.blueF(), edgeColor.alphaF()));

  // Рисуем ребра
  shaderProgram->setUniformValue("isVertex", false);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // Включаем соответствующий режим отрисовки ребер
  GLenum edgeMode = mapEdgeTypeToGL(displaySettings->getEdgeType());
  if (edgeMode != 0) {
    glEnable(edgeMode);
  }

  if (edgeMode != 0) {
    glEnable(edgeMode);
    // Устанавливаем шаблон штриховки в зависимости от типа линий
    switch (displaySettings->getEdgeType()) {
      case EdgeDisplayType::Dashed:
        glLineStipple(
            1, 0x0F0F);  // Штриховая линия: 4 пикселя пунктира, 4 пропуска
        break;
      case EdgeDisplayType::Dotted:
        glLineStipple(
            1, 0x0101);  // Пунктирная линия: 1 пиксель пунктира, 7 пропуска
        break;
      default:
        break;  // Для сплошных линий ничего не делаем
    }
  }
  glLineWidth(displaySettings->getEdgeWidth());
  glDrawElements(GL_LINES, model->getEdges().size(), GL_UNSIGNED_INT, nullptr);

  if (edgeMode != 0) {
    glDisable(edgeMode);
  }

  VertexDisplayType vertexType = displaySettings->getVertexType();
  if (vertexType != VertexDisplayType::None) {
    // Рисуем вершины
    shaderProgram->setUniformValue("isVertex", true);

    // Включаем соответствующий режим отрисовки вершин
    GLenum vertexMode = mapVertexTypeToGL(displaySettings->getVertexType());
    if (vertexMode != 0) {
      glEnable(vertexMode);
    }

    glPointSize(displaySettings->getVertexSize());
    glDrawArrays(GL_POINTS, 0, model->getVertices().size());

    if (vertexMode != 0) {
      glDisable(vertexMode);
    }
  }

  vao.release();
  shaderProgram->release();
}

/**
 * @brief Метод resizeGL вызывается автоматически каждый раз, когда изменяется
 * размер виджета (например, при изменении размера окна).
 * @param w ширина окна
 * @param h высота окна
 */
void GLWidget::resizeGL(int w, int h) {
  trans3D->setAspectRatio(h == 0 ? w : (float)w / h);
  trans3D->setProjection();

  if (model && !model->isEmpty()) {
    trans3D->setupView();
  }
}

void GLWidget::onModelChanged() {
  makeCurrent();

  if (displaySettings) {
    QColor bgColor = displaySettings->getBackgroundColor();
    glClearColor(bgColor.redF(), bgColor.greenF(), bgColor.blueF(),
                 bgColor.alphaF());
  }

  if (!model->getVertices().empty()) {
    calculateBoundingBox();
    trans3D->setupView();
    trans3D->setProjection();
  }
  updateBuffers();
  update();
}

GLenum GLWidget::mapEdgeTypeToGL(EdgeDisplayType type) {
  switch (type) {
    case EdgeDisplayType::Solid:
      return 0;  // Сплошные линии не требуют специального режима
    case EdgeDisplayType::Dashed:
      return GL_LINE_STIPPLE;  // Для штриховых линий
    case EdgeDisplayType::Dotted:
      return GL_LINE_STIPPLE;  // Для пунктирных линий
    default:
      return 0;
  }
}

GLenum GLWidget::mapVertexTypeToGL(VertexDisplayType type) {
  switch (type) {
    case VertexDisplayType::None:
      return 0;  // Не включаем специальный режим
    case VertexDisplayType::Circle:
      return GL_POINT_SMOOTH;  // Для круглых точек
    case VertexDisplayType::Square:
      return 0;  // Квадратные точки - это стандартный режим
    default:
      return 0;
  }
}

}  // namespace s21