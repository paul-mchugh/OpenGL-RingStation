
#include "Model.h"

int Model::getNumVertices() { return numVertices; }
int Model::getNumIndices()  { return numIndices; }
std::vector<GLuint> Model::getIndices() { return indices; }
std::vector<glm::vec3> Model::getVertices() { return vertices; }
std::vector<glm::vec2> Model::getTexCoords() { return texCoords; }
std::vector<glm::vec3> Model::getNormals() { return normals; }
std::vector<glm::vec3> Model::getTangents() { return tangents; }
