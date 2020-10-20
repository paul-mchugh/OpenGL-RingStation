#include <fstream>
#include <string>
#include <regex>
#include <cstdlib>
#include "Model.h"

int Model::getNumVertices() { return numVertices; }
int Model::getNumIndices()  { return numIndices; }
std::vector<GLuint> Model::getIndices() { return indices; }
std::vector<glm::vec3> Model::getVertices() { return vertices; }
std::vector<glm::vec2> Model::getTexCoords() { return texCoords; }
std::vector<glm::vec3> Model::getNormals() { return normals; }
std::vector<glm::vec3> Model::getTangents() { return tangents; }

//this model importer only imports .obj tiles that consist exclusively of triangles
Model ModelImporter::parseOBJ(const char* filePath)
{
	Model m;
	std::vector<glm::vec3> vertices, normals;
	std::vector<glm::vec2> texCoords;

	std::ifstream modelFile(filePath);
	std::string line;
	std::regex  vtxLn{"^v\\s+"};
	std::regex  texLn{"^vt\\s+"};
	std::regex normLn{"^vn\\s+"};
	std::regex faceLn{"^f\\s+(\\d+)\\/(\\d+)\\/(\\d+)\\s+(\\d+)\\/(\\d+)\\/(\\d+)\\s+(\\d+)\\/(\\d+)\\/(\\d+)"};

	std::smatch m2;
	std::string test="f 8021/4274/8291 8023/4276/8291 8022/4275/8291";
	std::regex_search(test, m2, faceLn);

	//read each line
	for(std::getline(modelFile,line); !modelFile.eof(); std::getline(modelFile,line))
	{
		//variables for matches of the lines
		std::smatch match;
		char* text = const_cast<char*>(line.c_str())+2;

		if(std::regex_search(line, match, vtxLn))
		{
			double x{strtod(text, &text)}, y{strtod(text, &text)}, z{strtod(text, &text)};
			vertices.push_back(glm::vec3{x,y,z});
		}
		else if(std::regex_search(line, match, texLn))
		{
			double s{strtod(text, &text)}, t{strtod(text, &text)};
			texCoords.push_back(glm::vec2{s,t});
		}
		else if(std::regex_search(line, match, normLn))
		{
			double x{strtod(text, &text)}, y{strtod(text, &text)}, z{strtod(text, &text)};
			normals.push_back(glm::vec3{x,y,z});
		}
		else if(std::regex_search(line, match, faceLn))
		{
			//extract each corner's values
			for(int i=0; i<3; ++i)
			{
				int off=i*3;
				int cVtx{stoi(match[off+1].str())-1}, cTX{stoi(match[off+2].str())-1}, cNorm{stoi(match[off+3].str())-1};
				m.vertices.push_back(vertices[cVtx]);
				m.texCoords.push_back(texCoords[cTX]);
				m.normals.push_back(normals[cNorm]);
			}
		}
		else
			continue;
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

	return m;
}
