
#include "World.h"

void World::init()
{
	glGenVertexArrays(VAOcnt, vao);
}

void World::setRoot(Orbiter& o)
{
	if(o.w==this)
		rootOrbiter=o.id;
}

void World::draw(std::stack<glm::mat4> mst)
{
	glClear(GL_DEPTH_BUFFER_BIT);
	glClearColor(bgColor.x,bgColor.y,bgColor.z,1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	//the root orbiter is special because it doesn't orbit it just rotates
	//in the center of the world.  We don't have to account for its position
	//because it is always at the origin
	if (rootOrbiter<orbiters.size())
		orbiters[rootOrbiter]->draw(mst);
}

void World::update(double timePassed)
{
	for(int i=0;i<orbiters.size();++i)
		orbiters[i]->updatePos(timePassed);
}


Orbiter::
	Orbiter(World* w, Model m, GLuint shader, GLuint texture, double scale,
	        double orbitPeriod, double revProgress, double incline, double intercept, double dist,
	        glm::vec3 axisOfRot, double rotationPeriod, double rotationProgress)
{
	//calculate axis of revolution
	glm::vec4 aor {0,1,0,1};
	aor = glm::rotate<float>(glm::mat4{1.0f}, incline, glm::vec3{1,0,0}) * aor;
	aor = glm::rotate<float>(glm::mat4{1.0f}, intercept, glm::vec3{0,1,0}) * aor;

	//initialize vars
	this->w=w;
	this->m=m;
	this->shader=shader;
	this->texture=texture;
	this->scale=scale;
	this->orbitRate=1/orbitPeriod;
	this->revProgress=revProgress;
	this->axisOfRevolution=glm::normalize(aor);
	this->ecIntercept=intercept;
	this->orbitDist=dist;
	this->axisOfRotation=glm::normalize(axisOfRot);
	this->rotRate=1/rotationPeriod;
	this->rotProgress=rotationProgress;

	//add to world
	this->w->orbiters.push_back(std::unique_ptr<Orbiter>(this));
	this->id=this->w->orbiters.size()-1;

	//generate/setup VBOs
	glBindVertexArray(w->vao[0]);
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

bool Orbiter::addChild(Orbiter& child)
{
	//children must be in same world
	if(child.w!=w) return false;

	children.push_back(child.id);
	return true;
}

void Orbiter::draw(std::stack<glm::mat4>& mstack)
{
	//account for scale
	mstack.push(mstack.top()*glm::scale<float>(glm::mat4{1.0f},glm::vec3{scale,scale,scale}));

	//account for rotation
	glm::mat4 rotation{1.0f};
	glm::vec3& arot=axisOfRotation;
	rotation *= glm::rotate<float>(glm::mat4{1.0f}, rotProgress*2*glm::pi<float>(), arot);

	mstack.push(mstack.top()*rotation);

	//Draw self
	glUseProgram(shader);
	glBindVertexArray(w->vao[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);

	//send the uniforms to the GPU
	GLuint mvHandle = glGetUniformLocation(shader, "mv_matrix");
	glUniformMatrix4fv(mvHandle, 1, GL_FALSE, glm::value_ptr(mstack.top()));
	GLuint texEnHandle = glGetUniformLocation(shader, "texEn");
	glProgramUniform1i(shader, texEnHandle, texture!=0);

	//send the texture to the GPU
	if(texture!=0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
	}

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[4]);
	glDrawElements(GL_TRIANGLES, m.getNumIndices(), GL_UNSIGNED_INT, 0);

	//undo accounting for scale and rotation
	mstack.pop();
	mstack.pop();

	//Draw children
	for (std::size_t indx : children)
	{
		Orbiter& child = *this->w->orbiters[indx];
		glm::mat4 revolution{1.0f};
		//rotate around the axis of revolution
		revolution *= glm::rotate<float>(glm::mat4{1.0f}, child.revProgress*2*glm::pi<float>(),
		                                 child.axisOfRevolution);
		glm::vec3 planetPos{child.orbitDist,0,0};
		planetPos =
			glm::vec3(glm::rotate<float>(glm::mat4{1.0f}, child.ecIntercept,
			                             glm::vec3{0,1,0}) * glm::vec4{planetPos,1});
		planetPos = glm::vec3(revolution * glm::vec4(planetPos,1));

		mstack.push(mstack.top()*glm::translate(glm::mat4(1.0f),planetPos));
		child.draw(mstack);
		mstack.pop();
	}
}

void Orbiter::updatePos(double timePassed)
{
	revProgress = fmod(revProgress + orbitRate*timePassed,1);
	rotProgress = fmod(rotProgress +   rotRate*timePassed,1);
}
