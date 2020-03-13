#include <fstream>
#include <sstream>
#include "Model.h"

int Model::getNumVertices() { return numVertices; }
int Model::getNumIndices()  { return numIndices; }
std::vector<GLuint> Model::getIndices() { return indices; }
std::vector<glm::vec3> Model::getVertices() { return vertices; }
std::vector<glm::vec2> Model::getTexCoords() { return texCoords; }
std::vector<glm::vec3> Model::getNormals() { return normals; }
std::vector<glm::vec3> Model::getTangents() { return tangents; }

Model ModelImporter::parseOBJ(const char* filePath) {
	using namespace std;
	Model m;
	std::vector<float> vertVals, stVals, normVals;
	float x, y, z;
	string content;
	ifstream fileStream(filePath, ios::in);
	string line = "";
	while (!fileStream.eof()) {
		getline(fileStream, line);
		if (line.compare(0, 2, "v ") == 0) {
			stringstream ss(line.erase(0, 1));
			ss >> x; ss >> y; ss >> z;
			vertVals.push_back(x);
			vertVals.push_back(y);
			vertVals.push_back(z);
		}
		if (line.compare(0, 2, "vt") == 0) {
			stringstream ss(line.erase(0, 2));
			ss >> x; ss >> y;
			stVals.push_back(x);
			stVals.push_back(y);
		}
		if (line.compare(0, 2, "vn") == 0) {
			stringstream ss(line.erase(0, 2));
			ss >> x; ss >> y; ss >> z;
			normVals.push_back(x);
			normVals.push_back(y);
			normVals.push_back(z);
		}
		if (line.compare(0, 2, "f ") == 0) {
			string oneCorner, v, t, n;
			stringstream ss(line.erase(0, 2));
			for (int i = 0; i < 3; i++) {
				getline(ss, oneCorner, ' ');
				stringstream oneCornerSS(oneCorner);
				getline(oneCornerSS, v, '/');
				getline(oneCornerSS, t, '/');
				getline(oneCornerSS, n, '/');

				int vertRef = (stoi(v) - 1) * 3;
				int tcRef = (stoi(t) - 1) * 2;
				int normRef = (stoi(n) - 1) * 3;

				m.vertices.push_back(glm::vec3{vertVals[vertRef+0], vertVals[vertRef+1],
				                               vertVals[vertRef+2]});

				m.texCoords.push_back(glm::vec2{stVals[tcRef],stVals[tcRef+1]});

				m.normals.push_back(glm::vec3{normVals[normRef+0], normVals[normRef+1],
				                              normVals[normRef+2]});
			}
		}
	}

	//configure sizes
	m.numVertices = m.numIndices = m.vertices.size();

	//configure indices.  This is wasteful, since I the vertices will be a 1to1 mapping
	//but I don't want to write another version of the rendering code for the case where
	//we don't have indices.
	m.indices.resize(m.numIndices);
	for(GLuint i=0;i<m.numIndices;++i)
		m.indices[i]=i;

	//fill the tangents array with garbage date, because we don't have tangents in the model
	m.tangents.resize(m.numVertices, glm::vec3{0,0,0});

	//return result
	return m;
}
