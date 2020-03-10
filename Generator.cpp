#include "Generator.h"
#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;


Model Generator::generateSphere(int prec) {
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

Model Generator::generateRingHab(double habWidth, double wallThick, double wallHeight, int prec)
{
	Model m;

	//setup convienence variables
    int& numVertices = m.numVertices;
    int&  numIndices = m.numIndices;

	m.numVertices = 16*(prec+1);
	m.numIndices  = 8*prec*6; // not sure this works

	//initialize vectors
	m.vertices  = std::vector<glm::vec3>(m.numVertices);
	m.texCoords = std::vector<glm::vec2>(m.numVertices);
	m.normals   = std::vector<glm::vec3>(m.numVertices);
	m.tangents  = std::vector<glm::vec3>(m.numVertices);
	m.indices   = std::vector<GLuint>(m.numIndices);


    std::vector<GLuint>&    indx = m.indices;
    std::vector<glm::vec3>& vert = m.vertices;
    std::vector<glm::vec2>& texc = m.texCoords;
    std::vector<glm::vec3>& norm = m.normals;
    std::vector<glm::vec3>& tans = m.tangents;


	//Compute useful things now
	double angStepRad = glm::radians(360.0/prec);
	double halfHabW   = habWidth/2;

	//build the first slice of the ringHab directly
	//top inner wall ground
	vert[0]  = glm::vec3{1-wallThick, halfHabW, 0};
	texc[0]  = glm::vec2{0,0};
	norm[0]  = glm::vec3{0,-1,0};
	tans[0]  = glm::vec3{-1,0,0};
	//top inner wall rim
	vert[1]  = glm::vec3{1-wallThick-wallHeight, halfHabW, 0};
	texc[1]  = glm::vec2{0,0.1};
	norm[1]  = glm::vec3{0,-1,0};
	tans[1]  = glm::vec3{-1,0,0};
	//top rim inner
	vert[2]  = glm::vec3{1-wallThick-wallHeight, halfHabW, 0};
	texc[2]  = glm::vec2{0,0.1};
	norm[2]  = glm::vec3{-1,0,0};
	tans[2]  = glm::vec3{0,1,0};
	//top rim outer
	vert[3]  = glm::vec3{1-wallThick-wallHeight, halfHabW+wallThick, 0};
	texc[3]  = glm::vec2{0,0.2};
	norm[3]  = glm::vec3{-1,0,0};
	tans[3]  = glm::vec3{0,1,0};
	//top outer wall rim
	vert[4]  = glm::vec3{1-wallThick-wallHeight, halfHabW+wallThick, 0};
	texc[4]  = glm::vec2{0,0.2};
	norm[4]  = glm::vec3{0,1,0};
	tans[4]  = glm::vec3{1,0,0};
	//top outer wall back adj
	vert[5]  = glm::vec3{1, halfHabW+wallThick, 0};
	texc[5]  = glm::vec2{0,0.3};
	norm[5]  = glm::vec3{0,1,0};
	tans[5]  = glm::vec3{1,0,0};
	//back top
	vert[6]  = glm::vec3{1, halfHabW+wallThick, 0};
	texc[6]  = glm::vec2{0,0.3};
	norm[6]  = glm::vec3{1,0,0};
	tans[6]  = glm::vec3{0,-1,0};
	//back bot
	vert[7]  = glm::vec3{1, -halfHabW-wallThick, 0};
	texc[7]  = glm::vec2{0,0.5};
	norm[7]  = glm::vec3{1,0,0};
	tans[7]  = glm::vec3{0,-1,0};
	//bot outer wall back adj
	vert[8]  = glm::vec3{1, -halfHabW-wallThick, 0};
	texc[8]  = glm::vec2{0,0.5};
	norm[8]  = glm::vec3{0,-1,0};
	tans[8]  = glm::vec3{-1,0,0};
	//bot outer wall rim
	vert[9]  = glm::vec3{1-wallThick-wallHeight, -halfHabW-wallThick, 0};
	texc[9]  = glm::vec2{0,0.6};
	norm[9]  = glm::vec3{0,-1,0};
	tans[9]  = glm::vec3{-1,0,0};
	//bot rim outer
	vert[10] = glm::vec3{1-wallThick-wallHeight, -halfHabW-wallThick, 0};
	texc[10] = glm::vec2{0,0.6};
	norm[10] = glm::vec3{-1,0,0};
	tans[10] = glm::vec3{0,1,0};
	//bot rim inner
	vert[11] = glm::vec3{1-wallThick-wallHeight, -halfHabW, 0};
	texc[11] = glm::vec2{0,0.7};
	norm[11] = glm::vec3{-1,0,0};
	tans[11] = glm::vec3{0,1,0};
	//bot inner wall rim
	vert[12] = glm::vec3{1-wallThick-wallHeight, -halfHabW, 0};
	texc[12] = glm::vec2{0,0.7};
	norm[12] = glm::vec3{0,1,0};
	tans[12] = glm::vec3{1,0,0};
	//bot inner wall ground
	vert[13] = glm::vec3{1-wallThick, -halfHabW, 0};
	texc[13] = glm::vec2{0,0.8};
	norm[13] = glm::vec3{0,1,0};
	tans[13] = glm::vec3{1,0,0};
	//bot ground
	vert[14] = glm::vec3{1-wallThick, -halfHabW, 0};
	texc[14] = glm::vec2{0,0.8};
	norm[14] = glm::vec3{-1,0,0};
	tans[14] = glm::vec3{0,1,0};
	//top ground
	vert[15] = glm::vec3{1-wallThick, halfHabW, 0};
	texc[15] = glm::vec2{0,1};
	norm[15] = glm::vec3{-1,0,0};
	tans[15] = glm::vec3{0,1,0};

	//finally done w/ the creation of the first slice.
	//now we rotate it around the y axis each step until
	//we have finished making all vertices
	for(int slice=1; slice<=prec; ++slice)
	{
		int sOff=slice*16;
		int sLastOff = sOff-16;
		glm::mat4 rot = glm::rotate<float>(glm::mat4{1.0f}, slice*angStepRad, glm::vec3{0,1,0});
		for(int vi = 0; vi<16; ++vi)
		{
			//rotate vertices
			vert[sOff+vi] = glm::vec3{rot * glm::vec4{vert[vi],1}};
			//copy t component of last texc, calculate s coord
			texc[sOff+vi] = glm::vec2{((float)slice)/prec, texc[vi].t};
			//rotate the normals and tangents
			norm[sOff+vi] = glm::vec3{rot * glm::vec4{norm[vi],1}};
			tans[sOff+vi] = glm::vec3{rot * glm::vec4{tans[vi],1}};
		}
	}

	//calculate triangle indices
	for(int slice=0; slice < prec; ++slice)
	{
		//here is where we generate the square panels
		for(int vi=0; vi<8; ++vi)
		{
			indx[((slice*8 + vi) * 2 + 0) * 3 + 0] = slice*16     + vi*2 + 0;
			indx[((slice*8 + vi) * 2 + 0) * 3 + 1] = slice*16     + vi*2 + 1;
			indx[((slice*8 + vi) * 2 + 0) * 3 + 2] = (slice+1)*16 + vi*2 + 0;
			indx[((slice*8 + vi) * 2 + 1) * 3 + 0] = slice*16     + vi*2 + 1;
			indx[((slice*8 + vi) * 2 + 1) * 3 + 1] = (slice+1)*16 + vi*2 + 1;
			indx[((slice*8 + vi) * 2 + 1) * 3 + 2] = (slice+1)*16 + vi*2 + 0;
		}
	}



	return m;
}
