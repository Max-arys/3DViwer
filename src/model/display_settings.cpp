#include "display_settings.h"

#include <QCoreApplication>
#include <QDebug>

namespace s21 {

#define APPNAME "3DViewer"

DisplaySettings::DisplaySettings(QObject* parent) : QObject(parent) {
  loadSettings();
}

DisplaySettings::~DisplaySettings() { saveSettings(); }

void DisplaySettings::setBackgroundColor(const QColor& color) {
  if (backgroundColor != color) {
    backgroundColor = color;
    emit dSChanged();
    saveSettings();
  }
}

void DisplaySettings::setVertexColor(const QColor& color) {
  if (vertexColor != color) {
    vertexColor = color;
    emit dSChanged();
    saveSettings();
  }
}

void DisplaySettings::setVertexSize(float size) {
  if (qAbs(vertexSize - size) > 0.01f) {
    vertexSize = size;
    emit dSChanged();
    saveSettings();
  }
}

void DisplaySettings::setVertexType(VertexDisplayType type) {
  if (vertexType != type) {
    vertexType = type;
    emit dSChanged();
    saveSettings();
  }
}

void DisplaySettings::setEdgeColor(const QColor& color) {
  if (edgeColor != color) {
    edgeColor = color;
    emit dSChanged();
    saveSettings();
  }
}

void DisplaySettings::setEdgeWidth(float width) {
  if (qAbs(edgeWidth - width) > 0.01f) {
    edgeWidth = width;
    emit dSChanged();
    saveSettings();
  }
}

void DisplaySettings::setEdgeType(EdgeDisplayType type) {
  if (edgeType != type) {
    edgeType = type;
    emit dSChanged();
    saveSettings();
  }
}

void DisplaySettings::loadSettings() {
  QString exePath = QCoreApplication::applicationDirPath();
  QString settingsFilePath = exePath + "/" + APPNAME + ".ini";
  QSettings settings(settingsFilePath, QSettings::IniFormat);

  backgroundColor = settings.value("display/backgroundColor", QColor(Qt::black))
                        .value<QColor>();
  vertexColor =
      settings.value("display/vertexColor", QColor(Qt::green)).value<QColor>();
  vertexSize =
      settings.value("display/vertexSize", DEFAULT_VERTEX_SIZE).toFloat();
  vertexType = static_cast<VertexDisplayType>(
      settings
          .value("display/vertexType",
                 static_cast<int>(VertexDisplayType::Circle))
          .toInt());

  edgeColor =
      settings.value("display/edgeColor", QColor(Qt::white)).value<QColor>();
  edgeWidth = settings.value("display/edgeWidth", DEFAULT_EDGE_WIDTH).toFloat();
  edgeType = static_cast<EdgeDisplayType>(
      settings
          .value("display/edgeType", static_cast<int>(EdgeDisplayType::Solid))
          .toInt());

  qDebug() << "Display settings loaded from:" << settingsFilePath;
}

void DisplaySettings::saveSettings() const {
  QString exePath = QCoreApplication::applicationDirPath();
  QString settingsFilePath = exePath + "/" + APPNAME + ".ini";
  QSettings settings(settingsFilePath, QSettings::IniFormat);

  settings.setValue("display/backgroundColor", backgroundColor);
  settings.setValue("display/vertexColor", vertexColor);
  settings.setValue("display/vertexSize", vertexSize);
  settings.setValue("display/vertexType", static_cast<int>(vertexType));

  settings.setValue("display/edgeColor", edgeColor);
  settings.setValue("display/edgeWidth", edgeWidth);
  settings.setValue("display/edgeType", static_cast<int>(edgeType));

  qDebug() << "Display settings saved to:" << settingsFilePath;
}

void DisplaySettings::resetToDefaults() {
  backgroundColor = Qt::black;
  vertexColor = Qt::green;
  vertexSize = DEFAULT_VERTEX_SIZE;
  vertexType = VertexDisplayType::Circle;

  edgeColor = Qt::white;
  edgeWidth = DEFAULT_EDGE_WIDTH;
  edgeType = EdgeDisplayType::Solid;

  saveSettings();
  emit dSChanged();
}

}  // namespace s21