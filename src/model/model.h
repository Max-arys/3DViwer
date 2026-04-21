#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <QQuaternion>
#include <QVector3D>
#include <memory>
#include <string>
#include <vector>

namespace s21 {

struct Vertex {
  QVector3D position;
};

class Model3D : public QObject {
  Q_OBJECT

 public:
  explicit Model3D(QObject *parent = nullptr);

  bool loadFromFile(const std::string &filename);
  const std::vector<Vertex> &getVertices() const { return vertices; }
  const std::vector<unsigned int> &getEdges() const { return edges; }
  bool isEmpty() const { return vertices.empty(); }

 private:
  void centerModel();

 public:
 signals:
  void modelLoaded();
  void errorOccurred(const QString &message);

 private:
  std::vector<Vertex> vertices;
  std::vector<unsigned int>
      edges;  // Индексы вершин в vertices, пара индексов образует одно ребро
};  // class Model3D
}  // namespace s21

#endif  // MODEL_H