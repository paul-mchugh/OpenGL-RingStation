#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>

class Model
{
public:
    int numVertices;
    int numIndices;
    std::vector<GLuint> indices;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> tangents;

    int getNumVertices();
    int getNumIndices();
    std::vector<GLuint> getIndices();
    std::vector<glm::vec3> getVertices();
    std::vector<glm::vec2> getTexCoords();
    std::vector<glm::vec3> getNormals();
    std::vector<glm::vec3> getTangents();
};

class ModelImporter
{
public:
	static Model parseOBJ(const char* filePath);
};

#endif
