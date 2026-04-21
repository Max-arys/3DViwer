#include "model.h"

#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <QDebug>
#include <assimp/Importer.hpp>
#include <set>

namespace s21 {

Model3D::Model3D(QObject *parent) : QObject(parent) {}

void Model3D::centerModel() {
  if (vertices.empty()) return;

  QVector3D minBounds = vertices[0].position;
  QVector3D maxBounds = vertices[0].position;

  for (const auto &vertex : vertices) {
    minBounds.setX(std::min(minBounds.x(), vertex.position.x()));
    minBounds.setY(std::min(minBounds.y(), vertex.position.y()));
    minBounds.setZ(std::min(minBounds.z(), vertex.position.z()));

    maxBounds.setX(std::max(maxBounds.x(), vertex.position.x()));
    maxBounds.setY(std::max(maxBounds.y(), vertex.position.y()));
    maxBounds.setZ(std::max(maxBounds.z(), vertex.position.z()));
  }

  QVector3D center = (minBounds + maxBounds) / 2.0f;

  for (auto &vertex : vertices) {
    vertex.position -= center;
  }
}

bool Model3D::loadFromFile(const std::string &filename) {
  Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(
      filename,
      aiProcess_Triangulate |                // Триангуляция
          aiProcess_FlipUVs |                // Переворот по оси Y
          aiProcess_JoinIdenticalVertices |  // Слить одинаковые вершины
          aiProcess_OptimizeMeshes |  // Оптимизировать меши
          aiProcess_OptimizeGraph |  // Оптимизировать граф сцены
          aiProcess_ImproveCacheLocality |  // Улучшает кэширование
          aiProcess_SortByPType  // Сортировка по типам примитивов
  );

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    QString error =
        QString("Ошибка загрузки: %1").arg(importer.GetErrorString());
    emit errorOccurred(error);
    return false;
  }

  vertices.clear();
  edges.clear();

  // Множество для хранения уникальных ребер
  std::set<std::pair<unsigned int, unsigned int>> uniqueEdges;

  for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
    aiMesh *mesh = scene->mMeshes[i];
    unsigned int vertexOffset = vertices.size();

    // Сначала загружаем все вершины
    for (unsigned int j = 0; j < mesh->mNumVertices; ++j) {
      Vertex vertex;
      vertex.position = QVector3D(mesh->mVertices[j].x, mesh->mVertices[j].y,
                                  mesh->mVertices[j].z);
      vertices.push_back(vertex);
    }

    // Загружаем индексы ребер
    for (unsigned int j = 0; j < mesh->mNumFaces; ++j) {
      aiFace face = mesh->mFaces[j];
      for (unsigned int k = 0; k < face.mNumIndices; ++k) {
        unsigned int idx1 = face.mIndices[k] + vertexOffset;
        unsigned int idx2 =
            face.mIndices[(k + 1) % face.mNumIndices] + vertexOffset;

        // Сохраняем ребро в канонической форме (меньший индекс первым)
        if (idx1 > idx2) {
          std::swap(idx1, idx2);
        }

        // Вставляем в set - дубликаты автоматически отсеются
        if (uniqueEdges.insert({idx1, idx2}).second) {
          edges.push_back(idx1);
          edges.push_back(idx2);
        }
      }
    }
  }

  centerModel();

  emit modelLoaded();
  return true;
}

}  // namespace s21