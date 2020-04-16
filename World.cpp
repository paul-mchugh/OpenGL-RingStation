
#include "World.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdint>
#include <cmath>
#include <utility>

void World::init()
{
	glGenVertexArrays(VAOcnt, vao);
	ambient={.ambient=glm::vec4{0.1,0.1,0.1,1},.enabled=true,.type=LightType::AMBIENT};
	lights[0]=ambient;
	directional={.ambient=glm::vec4{0,0,0,1}, .diffuse=glm::vec4{1,1,1,1},
	             .specular=glm::vec4{1,1,1,1},.direction=glm::vec3{0,0,1},
	             .enabled=true,.type=LightType::DIRECTIONAL};
	lights[1]=directional;
}

void World::draw(std::stack<glm::mat4> mst)
{
	glClear(GL_DEPTH_BUFFER_BIT);
	glClearColor(bgColor.x,bgColor.y,bgColor.z,1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	//draw absolutely positioned objects
	for(int i=0;i<objects.size();++i)
	{
		std::unique_ptr<Object>& obj = objects[i];
		if (obj->posType == PosType::ABSOLUTE)
		{
			//add translation for the absolute positioning
			mst.push(mst.top()*glm::translate(glm::mat4{1.0f}, obj->p.a.pos));
			obj->draw(mst);
			//remove translation for the absolute positioning
			mst.pop();
		}
	}
}

void World::update(double timePassed)
{
	for(int i=0;i<objects.size();++i)
		objects[i]->updatePos(timePassed);
}

void World::relight()
{
	std::stack<glm::mat4> mst;
	mst.push(glm::mat4{1});

	//relight absolutely positioned objects
	for(int i=0;i<objects.size();++i)
	{
		std::unique_ptr<Object>& obj = objects[i];
		if (obj->posType == PosType::ABSOLUTE)
		{
			//add translation for the absolute positioning
			mst.push(mst.top()*glm::translate(glm::mat4{1.0f}, obj->p.a.pos));
			obj->relight(mst);
			//remove translation for the absolute positioning
			mst.pop();
		}
	}
}

void World::glTransferLights(glm::mat4 vMat, GLuint shader, std::string name)
{
	lights[0]=ambient;
	lights[1]=directional;

	for(GLuint i=0; i<World::MAX_LIGHTS; ++i)
	{
		lights[i].glTransfer(vMat, shader, name, i);
	}
}

void World::drawVecToLight(glm::mat4 pMat, glm::mat4 vMat)
{
	for(int i=0;i<MAX_LIGHTS;++i)
	{
		Light l = lights[i];
		if(l.type==LightType::POSITIONAL||l.type==LightType::SPOTLIGHT)
			LineDrawer::draw(pMat, vMat, glm::vec3{0}, l.position, glm::vec3{0.5,0,0.5});
	}
}

void Light::glTransfer(glm::mat4 vMat, GLuint shader, std::string name, GLuint indx)
{
	GLuint ambLOC, diffLOC, specLOC, posLOC, dirLOC, cutLOC, expLOC, enLOC, typeLOC;
	glm::vec3 viewPos = glm::vec3{vMat * glm::vec4{position,1}};
	std::string fName = name+"["+std::to_string(indx)+"]";
	ambLOC  = glGetUniformLocation(shader, (fName+".ambient").c_str());
	diffLOC = glGetUniformLocation(shader, (fName+".diffuse").c_str());
	specLOC = glGetUniformLocation(shader, (fName+".specular").c_str());
	posLOC  = glGetUniformLocation(shader, (fName+".position").c_str());
	dirLOC  = glGetUniformLocation(shader, (fName+".direction").c_str());
	cutLOC  = glGetUniformLocation(shader, (fName+".cutoff").c_str());
	expLOC  = glGetUniformLocation(shader, (fName+".exponent").c_str());
	enLOC   = glGetUniformLocation(shader, (fName+".enabled").c_str());
	typeLOC = glGetUniformLocation(shader, (fName+".type").c_str());
	glProgramUniform4fv(shader,  ambLOC, 1, glm::value_ptr(ambient));
	glProgramUniform4fv(shader, diffLOC, 1, glm::value_ptr(diffuse));
	glProgramUniform4fv(shader, specLOC, 1, glm::value_ptr(specular));
	glProgramUniform3fv(shader,  posLOC, 1, glm::value_ptr(viewPos));
	glProgramUniform3fv(shader,  dirLOC, 1, glm::value_ptr(direction));
	glProgramUniform1f (shader,  cutLOC, (GLfloat)cutoff);
	glProgramUniform1f (shader,  expLOC, (GLfloat)exponent);
	glProgramUniform1ui(shader,   enLOC, (GLuint) enabled);
	glProgramUniform1ui(shader, typeLOC, (GLuint) type);
}

Object::~Object()
{
	if(lightIndx==-1)
		w->lights[lightIndx].type=LightType::NO_LIGHT;
}

Object::
	Object(World* w, Model m, GLuint shader, GLuint texture, Material mat, double scale,
	       glm::vec3 axisOfRot, double rotationPeriod, double rotationProgress)
{
	//initialize vars
	this->w=w;
	this->m=m;
	this->shader=shader;
	this->texture=texture;
	this->mat=mat;
	this->scale=scale;
	this->axisOfRotation=glm::normalize(axisOfRot);
	this->rotRate=(rotationPeriod==0)?0:1/rotationPeriod;
	this->rotProgress=fmod(rotationProgress,1);

	//add to world
	this->w->objects.push_back(std::unique_ptr<Object>(this));
	this->id=this->w->objects.size()-1;

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

Object*
Object::
	makeOrbiter(World* w, Model m, GLuint shader, GLuint texture, Material mat, double scale,
	            double orbitPeriod, double revProgress,
	            double incline, double intercept, double dist,
	            glm::vec3 axisOfRot, double rotationPeriod, double rotationProgress)
{
	Object* obj = new Object(w, m, shader, texture, mat, scale,
	                         axisOfRot, rotationPeriod, rotationProgress);
	//set positioning type
	obj->posType = PosType::ORBITING;

	//calculate axis of revolution
	glm::vec4 aor {0,1,0,1};
	aor = glm::rotate<float>(glm::mat4{1.0f}, incline,   glm::vec3{1,0,0}) * aor;
	aor = glm::rotate<float>(glm::mat4{1.0f}, intercept, glm::vec3{0,1,0}) * aor;

	obj->p.o.orbitRate=(orbitPeriod==0)?0:1/orbitPeriod;
	obj->p.o.revProgress=fmod(revProgress,1);
	obj->p.o.axisOfRevolution=glm::normalize(aor);
	obj->p.o.ecIntercept=intercept;
	obj->p.o.orbitDist=dist;

	return obj;
}

Object*
Object::
	makeAbsolute(World* w, Model m, GLuint shader, GLuint texture, Material mat, double scale,
	             glm::vec3 pos,
	             glm::vec3 axisOfRot, double rotationPeriod, double rotationProgress)
{
	Object* obj = new Object(w, m, shader, texture, mat, scale,
	                         axisOfRot, rotationPeriod, rotationProgress);
	//set positioning type
	obj->posType = PosType::ABSOLUTE;

	obj->p.a.pos=pos;

	return obj;
}

Object*
Object::
	makeRelative(World* w, Model m, GLuint shader, GLuint texture, Material mat, double scale,
	             glm::vec3 rPos,
	             glm::vec3 axisOfRot, double rotationPeriod, double rotationProgress)
{
	Object* obj = new Object(w, m, shader, texture, mat, scale,
	                         axisOfRot, rotationPeriod, rotationProgress);

	//set positioning type
	obj->posType = PosType::RELATIVE;

	obj->p.r.rPos=rPos;

	return obj;
}

bool Object::addChild(Object& child)
{
	//children must be in same world and not absolutely positioned
	if(child.w!=w || child.posType==PosType::ABSOLUTE) return false;

	children.push_back(child.id);
	return true;
}

bool Object::attachLight(Light& light)
{
	//deal with removing lights
	if(light.type==LightType::NO_LIGHT)
	{
		if(lightIndx!=-1)
			w->lights[lightIndx].type=LightType::NO_LIGHT;
		lightIndx=-1;
		return true;
	}

	//find an index for the light (if it does not have one)
	if(lightIndx==-1)
		for(int i=0;i<World::MAX_LIGHTS;++i)
			if(w->lights[i].type==LightType::NO_LIGHT)
				lightIndx=i;
			else if(i==World::MAX_LIGHTS-i)
				return false;

	//copy light return true
	w->lights[lightIndx]=light;
	return true;
}

void Object::setLightEnabled(bool en)
{
	if(lightIndx!=-1)
		w->lights[lightIndx].enabled=en;
}
bool Object::getLightEnabled(){ return lightIndx!=-1 && w->lights[lightIndx].enabled; }

void Object::draw(std::stack<glm::mat4>& mstack)
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
	glUniformMatrix4fv(mvHandle,   1, GL_FALSE, glm::value_ptr(mstack.top()));
	GLuint normHandle = glGetUniformLocation(shader, "norm_matrix");
	glm::mat4 normMat = glm::transpose(glm::inverse(mstack.top()));
	glUniformMatrix4fv(normHandle, 1, GL_FALSE, glm::value_ptr(normMat));
	GLuint texEnHandle = glGetUniformLocation(shader, "texEn");
	glProgramUniform1i(shader, texEnHandle, texture!=0);
	GLuint atLightHandle = glGetUniformLocation(shader, "atLight");
	glProgramUniform1i(shader, atLightHandle, lightIndx);
	mat.glTransfer(shader, "material");

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
		Object& child = *this->w->objects[indx];
		//if the child is relatively positioned put the rotation back
		//and translate by the relative position
		if(child.posType==PosType::RELATIVE)
		{
			mstack.push(mstack.top()*rotation);
			mstack.push(mstack.top()*glm::translate(glm::mat4{1.0f},(float)scale*child.p.r.rPos));
		}

		glm::mat4 revolution{1.0f};
		//rotate around the axis of revolution
		revolution *= glm::rotate<float>(glm::mat4{1.0f}, child.p.o.revProgress*2*glm::pi<float>(),
		                                 child.p.o.axisOfRevolution);
		glm::vec3 planetPos{child.p.o.orbitDist,0,0};
		planetPos =
			glm::vec3(glm::rotate<float>(glm::mat4{1.0f}, child.p.o.ecIntercept,
			                             glm::vec3{0,1,0}) * glm::vec4{planetPos,1});
		planetPos = glm::vec3(revolution * glm::vec4(planetPos,1));

		mstack.push(mstack.top()*glm::translate(glm::mat4(1.0f),planetPos));
		child.draw(mstack);
		mstack.pop();
		//if the child is relatively positioned put the rotation back
		//remove the translations and rotations
		if(child.posType==PosType::RELATIVE)
		{
			mstack.pop();
			mstack.pop();
		}
	}
}

void Object::updatePos(double timePassed)
{
	if(posType==PosType::ORBITING)
		p.o.revProgress = fmod(p.o.revProgress + p.o.orbitRate*timePassed,1);
	rotProgress = fmod(rotProgress +   rotRate*timePassed,1);
}

void Object::relight(std::stack<glm::mat4>& mstack)
{
	//account for scale
	mstack.push(mstack.top()*glm::scale<float>(glm::mat4{1.0f},glm::vec3{scale,scale,scale}));

	//account for rotation
	glm::mat4 rotation{1.0f};
	glm::vec3& arot=axisOfRotation;
	rotation *= glm::rotate<float>(glm::mat4{1.0f}, rotProgress*2*glm::pi<float>(), arot);

	mstack.push(mstack.top()*rotation);

	//compute light position if this is a lighted object
	if(lightIndx!=-1)
	{
		w->lights[lightIndx].position = glm::vec3{mstack.top() * glm::vec4{0,0,0,1}};
	}

	//undo accounting for scale and rotation
	mstack.pop();
	mstack.pop();

	//Draw children
	for (std::size_t indx : children)
	{
		Object& child = *this->w->objects[indx];

		//if the child is relatively positioned put the rotation back
		//and translate by the relative position
		if(child.posType==PosType::RELATIVE)
		{
			mstack.push(mstack.top()*rotation);
			mstack.push(mstack.top()*glm::translate(glm::mat4{1.0f},(float)scale*child.p.r.rPos));
		}

		glm::mat4 revolution{1.0f};
		//rotate around the axis of revolution
		revolution *= glm::rotate<float>(glm::mat4{1.0f}, child.p.o.revProgress*2*glm::pi<float>(),
		                                 child.p.o.axisOfRevolution);
		glm::vec3 planetPos{child.p.o.orbitDist,0,0};
		planetPos =
			glm::vec3(glm::rotate<float>(glm::mat4{1.0f}, child.p.o.ecIntercept,
			                             glm::vec3{0,1,0}) * glm::vec4{planetPos,1});
		planetPos = glm::vec3(revolution * glm::vec4(planetPos,1));

		mstack.push(mstack.top()*glm::translate(glm::mat4(1.0f),planetPos));

		//walk children and update whether we have a lit child
		child.relight(mstack);

		mstack.pop();
		//if the child is relatively positioned put the rotation back
		//remove the translations and rotations
		if(child.posType==PosType::RELATIVE)
		{
			mstack.pop();
			mstack.pop();
		}
	}
}
