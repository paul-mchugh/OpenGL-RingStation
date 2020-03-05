#include "Sphere.h"
#include <iostream>
#include <cmath>
using namespace std;


Model SphereGenerator::generateSphere(int prec) {
	Model m;
	m.numVertices = (prec + 1) * (prec + 1);
	m.numIndices = prec * prec * 6;
	for (int i = 0; i < m.numVertices; i++) { m.vertices.push_back(glm::vec3()); }
	for (int i = 0; i < m.numVertices; i++) { m.texCoords.push_back(glm::vec2()); }
	for (int i = 0; i < m.numVertices; i++) { m.normals.push_back(glm::vec3()); }
	for (int i = 0; i < m.numVertices; i++) { m.tangents.push_back(glm::vec3()); }
	for (int i = 0; i < m.numIndices; i++) { m.indices.push_back(0); }

	// calculate triangle vertices
	for (int i = 0; i <= prec; i++) {
		for (int j = 0; j <= prec; j++) {
			float y = (float)cos(glm::radians(180.0f - i * 180.0f / prec));
			float x = -(float)cos(glm::radians(j*360.0f / prec))*(float)abs(cos(asin(y)));
			float z = (float)sin(glm::radians(j*360.0f / (float)(prec)))*(float)abs(cos(asin(y)));
			m.vertices[i*(prec + 1) + j] = glm::vec3(x, y, z);
			m.texCoords[i*(prec + 1) + j] = glm::vec2(((float)j / prec), ((float)i / prec));
			m.normals[i*(prec + 1) + j] = glm::vec3(x, y, z);

			// calculate tangent vector
			if (((x == 0) && (y == 1) && (z == 0)) || ((x == 0) && (y == -1) && (z == 0))) {
				m.tangents[i*(prec + 1) + j] = glm::vec3(0.0f, 0.0f, -1.0f);
			}
			else {
				m.tangents[i*(prec + 1) + j] = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(x, y, z));
			}
		}
	}
	// calculate triangle indices
	for (int i = 0; i<prec; i++) {
		for (int j = 0; j<prec; j++) {
			m.indices[6 * (i*prec + j) + 0] = i*(prec + 1) + j;
			m.indices[6 * (i*prec + j) + 1] = i*(prec + 1) + j + 1;
			m.indices[6 * (i*prec + j) + 2] = (i + 1)*(prec + 1) + j;
			m.indices[6 * (i*prec + j) + 3] = i*(prec + 1) + j + 1;
			m.indices[6 * (i*prec + j) + 4] = (i + 1)*(prec + 1) + j + 1;
			m.indices[6 * (i*prec + j) + 5] = (i + 1)*(prec + 1) + j;
		}
	}
	return m;
}

