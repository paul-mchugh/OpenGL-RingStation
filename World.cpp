
#include "World.h"

Orbiter::
	Orbiter(World* w, Model m, GLuint shader, GLuint texture,
            double orbitPeriod, double progress, double inclination,
            double intercept, double dist,
            glm::vec3 axis, double rotationPeriod, double rotationProgress)
{
	//initialize vars
	this->w=w;
	this->m=m;
	this->shader=shader;
	this->texture=texture;
	this->orbitRate=1/orbitPeriod;
	this->progress=progress;
	this->angleOfInclination=inclination;
	this->angleOfEclipticIntercept=intercept;
	this->orbitDist=dist;
	this->axisOfRotation=axis;
	this->rotRate=1/rotationPeriod;
	this->rotProgress=rotationProgress;

	//generate/setup VBOs
	glBindVertexArray(w->vaos[0]);
	glGenBuffers(5, vbo);
	int iCnt = m.getNumIndices();
	int vCnt = m.getNumVertices();
	//load vertices
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER,vCnt*3*sizeof(float),&m.getVertices()[0],GL_STATIC_DRAW);
	//load texture coords
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER,vCnt*2*sizeof(float),&m.getTexCoords()[0],GL_STATIC_DRAW);
	//load normals
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glBufferData(GL_ARRAY_BUFFER,vCnt*3*sizeof(float),&m.getNormals()[0],GL_STATIC_DRAW);
	//load tangent
	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
    glBufferData(GL_ARRAY_BUFFER,vCnt*3*sizeof(float),&m.getTangents()[0],GL_STATIC_DRAW);
	//load indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[4]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,iCnt*sizeof(GLuint),&m.getIndices()[0],GL_STATIC_DRAW);

}

void Orbiter::draw(World& w, std::vector<glm::mat4>)
{
	
}
