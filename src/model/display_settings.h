#ifndef DISPLAY_SETTINGS_H
#define DISPLAY_SETTINGS_H

#include <QColor>
#include <QCoreApplication>
#include <QObject>
#include <QSettings>

namespace s21 {

enum class VertexDisplayType { None, Circle, Square };
enum class EdgeDisplayType { Solid, Dashed, Dotted };

class DisplaySettings : public QObject {
  Q_OBJECT

 public:
  explicit DisplaySettings(QObject* parent = nullptr);
  ~DisplaySettings();

  // Геттеры
  QColor getBackgroundColor() const { return backgroundColor; }
  QColor getVertexColor() const { return vertexColor; }
  float getVertexSize() const { return vertexSize; }
  VertexDisplayType getVertexType() const { return vertexType; }

  QColor getEdgeColor() const { return edgeColor; }
  float getEdgeWidth() const { return edgeWidth; }
  EdgeDisplayType getEdgeType() const { return edgeType; }

 public slots:
  void setBackgroundColor(const QColor& color);
  void setVertexColor(const QColor& color);
  void setVertexSize(float size);
  void setVertexType(VertexDisplayType type);

  void setEdgeColor(const QColor& color);
  void setEdgeWidth(float width);
  void setEdgeType(EdgeDisplayType type);

  // Загрузка/сохранение
  void loadSettings();
  void saveSettings() const;

  // Сброс к значениям по умолчанию
  void resetToDefaults();

 signals:
  void dSChanged();

 private:
  // Значения по умолчанию
  static constexpr float DEFAULT_VERTEX_SIZE = 8.0f;
  static constexpr float DEFAULT_EDGE_WIDTH = 1.0f;

  QColor backgroundColor{Qt::black};
  QColor vertexColor{Qt::green};
  float vertexSize{DEFAULT_VERTEX_SIZE};
  VertexDisplayType vertexType{VertexDisplayType::Circle};

  QColor edgeColor{Qt::white};
  float edgeWidth{DEFAULT_EDGE_WIDTH};
  EdgeDisplayType edgeType{EdgeDisplayType::Solid};
};

}  // namespace s21

#endif  // DISPLAY_SETTINGS_H