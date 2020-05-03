
#include "World.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdint>
#include <cmath>
#include <utility>

//consts
const glm::mat4 B=
	glm::scale(glm::translate(glm::mat4{1.0f},glm::vec3{0.5}), glm::vec3{0.5});
/*{
0.5f,0.0f,0.0f,0.0f,
0.0f,0.5f,0.0f,0.0f,
0.0f,0.0f,0.5f,0.0f,
0.5f,0.5f,0.5f,1.0f,
};*/

const GLuint flatDisabled = 8;
const GLuint cubeDisabled = 9;
const GLuint tuOff = 10;
const GLsizei shadRes=1024;//all opengl GPUs are guaranteed to support 1024x1024 textures

//dirty hack global var
GLint currentShadowLight=-1;
glm::mat4 drawvMat;

void World::init()
{
	glGenVertexArrays(VAOcnt, vao);
	for(int i=0; i<MAX_LIGHTS; ++i)
	{
		lights[i]={.enabled=false,.type=LightType::NO_LIGHT};
		shadowTextures[i]=0;
	}
	glGenFramebuffers(1,&framebuffer);
	ambient={.ambient=glm::vec4{0.1,0.1,0.1,1},.enabled=true,.type=LightType::AMBIENT};
	replaceLight(ambient,0);
	directional={.ambient=glm::vec4{0,0,0,1}, .diffuse=glm::vec4{1,1,1,1},
	             .specular=glm::vec4{1,1,1,1},.direction=glm::vec3{0,0,1},
	             .enabled=true,.type=LightType::DIRECTIONAL};
	replaceLight(directional,1);

	//create a cubemap for all the disabled cubes to access
	glGenTextures(1, &sinkCM);
	glBindTexture(GL_TEXTURE_CUBE_MAP, sinkCM);
	for(GLuint i = 0; i<6; ++i)
		glTexImage2D(cubemapFaces[i], 0, GL_DEPTH_COMPONENT, shadRes, shadRes,
		             0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	//and bind it
	glActiveTexture(GL_TEXTURE0+cubeDisabled);
   	glBindTexture(GL_TEXTURE_CUBE_MAP, sinkCM);

	//create a 2dmap for all the disabled flats to access
	glGenTextures(1, &sinkFM);
	glBindTexture(GL_TEXTURE_2D, sinkFM);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
	             shadRes, shadRes, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//and bind it
	glActiveTexture(GL_TEXTURE0+flatDisabled);
   	glBindTexture(GL_TEXTURE_2D, sinkFM);
}

void World::draw(glm::mat4 vMat)
{
//	glClear(GL_DEPTH_BUFFER_BIT);
//	glClearColor(bgColor.x,bgColor.y,bgColor.z,1.0);
//	glClear(GL_COLOR_BUFFER_BIT);

	drawvMat=vMat;

	std::stack<glm::mat4> mst;
	mst.push(glm::mat4{1});

	//draw absolutely positioned objects
	for(int i=0;i<objects.size();++i)
	{
		std::unique_ptr<Object>& obj = objects[i];
		if (obj->posType == PosType::ABSOLUTE)
		{
			//add translation for the absolute positioning
			mst.push(mst.top()*glm::translate(glm::mat4{1.0f}, obj->p.a.pos));
			obj->walk<&Object::drawAction>(mst);
			//remove translation for the absolute positioning
			mst.pop();
		}
	}
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
			obj->walk<&Object::relightAction>(mst);
			//remove translation for the absolute positioning
			mst.pop();
		}
	}
}

void World::buildShadowBuffers(GLint viewMap, GLint viewFace)
{
	for(GLint i=0;i<MAX_LIGHTS;++i)
	{
		Light l = lights[i];
		//no shadows for empth light slots, ambient lights, or disabled lights
		if(l.type==LightType::NO_LIGHT||l.type==LightType::AMBIENT||!l.enabled)
			continue;
		else if(l.type==LightType::DIRECTIONAL||l.type==LightType::SPOTLIGHT||
		        l.type==LightType::POSITIONAL)
		{
			for(int f=0; f<(l.type==LightType::POSITIONAL?6:1);++f)
			{
				bool view = (viewMap==i&&viewFace==f);
				//switch to the shadow framebuffer and connect it to the shadow texture
				glBindFramebuffer(GL_FRAMEBUFFER,!view?framebuffer:0);
				if(!view)
				{
					if(l.type!=LightType::POSITIONAL)
						glFramebufferTexture(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,
						                     shadowTextures[i],0);
					else
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
						                       cubemapFaces[f], shadowTextures[i], 0);
				}
				glViewport(0,0,shadRes,shadRes);
				//drawing setting depends on if we are debugging this texture, depth on
				glDrawBuffer(view?GL_FRONT:GL_NONE);
				glEnable(GL_DEPTH_TEST);
				//clean out the old color and depth buffer
				glClear(GL_DEPTH_BUFFER_BIT);
				glClearColor(bgColor.x,bgColor.y,bgColor.z,1.0);
				glClear(GL_COLOR_BUFFER_BIT);

				//setup the matrices
				glm::mat4 pMat, vMat;
				if(l.type==LightType::DIRECTIONAL)
				{
					GLfloat hShadRes=shadRes/10.0f;
					pMat = glm::ortho(-hShadRes,hShadRes,-hShadRes,hShadRes,-hShadRes*2,hShadRes*2);
					vMat = glm::lookAt(glm::normalize(-l.direction)*hShadRes,
				                   glm::vec3{0},glm::vec3{0,1,0});
				}
				else if(l.type==LightType::SPOTLIGHT)
				{
					pMat = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f);
					vMat = glm::lookAt(l.position,l.position+l.direction,glm::vec3{0,1,0});
				}
				else if(l.type==LightType::POSITIONAL)
				{
					pMat = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f);
					switch(cubemapFaces[f])
					{
					case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
						vMat = glm::lookAt(l.position,l.position+faceDirv[f],glm::vec3{0,0,1});
						break;
					case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
						vMat = glm::lookAt(l.position,l.position+faceDirv[f],glm::vec3{0,0,-1});
						break;
					default:
						vMat = glm::lookAt(l.position,l.position+faceDirv[f],glm::vec3{0,-1,0});
					}
				}
				vpMats[i]=pMat*vMat;
				currentShadowLight=i;

				//setup matrix stack and start walking the world
				std::stack<glm::mat4> mst;
				mst.push(glm::mat4{1});

				//draw absolutely positioned objects
				for(int o=0;o<objects.size();++o)
				{
					std::unique_ptr<Object>& obj = objects[o];
					if (obj->posType == PosType::ABSOLUTE)
					{
						//add translation for the absolute positioning
						mst.push(mst.top()*glm::translate(glm::mat4{1.0f}, obj->p.a.pos));
						obj->walk<&Object::shadowAction>(mst);
						//remove translation for the absolute positioning
						mst.pop();
					}
				}
			}
		}
	}
}

void World::update(double timePassed)
{
	for(int i=0;i<objects.size();++i)
		objects[i]->updatePos(timePassed);
}

void World::replaceLight(Light lNew, GLuint indx)
{
	Light lOld = lights[indx];
	int oldTexType=0;
	oldTexType+=(lOld.type!=LightType::NO_LIGHT&lOld.type!=LightType::AMBIENT);
	oldTexType+=(lOld.type==LightType::POSITIONAL);
	int newTexType=0;
	newTexType+=(lNew.type!=LightType::NO_LIGHT&lNew.type!=LightType::AMBIENT);
	newTexType+=(lNew.type==LightType::POSITIONAL);

	if(oldTexType>0 && oldTexType!=newTexType)
		glDeleteTextures(1,&shadowTextures[indx]);

	if(newTexType>0 && oldTexType!=newTexType)
	{
		if(lNew.type==LightType::POSITIONAL)
		{
			//create the cudemap texture
			glGenTextures(1, &shadowTextures[indx]);
			glBindTexture(GL_TEXTURE_CUBE_MAP, shadowTextures[indx]);
			for(GLuint i = 0; i<6; ++i)
				glTexImage2D(cubemapFaces[i], 0, GL_DEPTH_COMPONENT, shadRes, shadRes,
				             0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE,
			                GL_COMPARE_R_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		}
		else if(lNew.type==LightType::DIRECTIONAL||lNew.type==LightType::SPOTLIGHT)
		{
			//create a 2d texture
			glGenTextures(1, &shadowTextures[indx]);
			glBindTexture(GL_TEXTURE_2D, shadowTextures[indx]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
			             shadRes, shadRes, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
	}

	//replace the light entry.
	lights[indx]=lNew;

}

void World::glTransferLights(glm::mat4 vMat, GLuint shader, std::string name)
{
	ambient.type=LightType::AMBIENT;
	directional.type=LightType::DIRECTIONAL;
	replaceLight(ambient,0);
	replaceLight(directional,1);

	for(GLuint i=0; i<World::MAX_LIGHTS; ++i)
	{
		lights[i].glTransfer(vMat, shader, name, i);
	}

	//transfer the shadow textures
	for(GLuint i=0; i<World::MAX_LIGHTS; ++i)
	{
		Light l = lights[i];
		GLuint flatSamp =
			glGetUniformLocation(shader, ("flats["+std::to_string(i)+"]").c_str());
		GLuint cubeSamp =
			glGetUniformLocation(shader, ("cubes["+std::to_string(i)+"]").c_str());
		if(l.type==LightType::NO_LIGHT||l.type==LightType::AMBIENT)
		{
			glUniform1i(flatSamp, flatDisabled);
			glUniform1i(cubeSamp, cubeDisabled);
		}
		else if(l.type==LightType::DIRECTIONAL||l.type==LightType::SPOTLIGHT)
		{
			//set the samplers to the appropriate TUs
			glUniform1i(flatSamp, tuOff+i);
			glUniform1i(cubeSamp, cubeDisabled);

			glActiveTexture(GL_TEXTURE0+i+tuOff);
        	glBindTexture(GL_TEXTURE_2D, shadowTextures[i]);

		}
		else if(l.type==LightType::POSITIONAL)
		{
			glUniform1i(flatSamp, flatDisabled);
			glUniform1i(cubeSamp, tuOff+i);

			glActiveTexture(GL_TEXTURE0+i+tuOff);
        	glBindTexture(GL_TEXTURE_CUBE_MAP, shadowTextures[i]);
		}
	}
}

void World::drawLightVecs(glm::mat4 pMat, glm::mat4 vMat)
{
	for(int i=0;i<MAX_LIGHTS;++i)
	{
		Light l = lights[i];
		if(l.type==LightType::POSITIONAL||l.type==LightType::SPOTLIGHT)
			LineDrawer::draw(pMat, vMat, glm::vec3{0}, l.position, glm::vec3{1,0,1});
		if(l.type==LightType::SPOTLIGHT)
		{
			glm::vec3 sp = l.position;
			glm::vec3 ep = sp+glm::normalize(l.direction)*5.0f;
			LineDrawer::draw(pMat, vMat, sp, ep, glm::vec3{1,1,0});
		}
	}
}

void Light::glTransfer(glm::mat4 vMat, GLuint shader, std::string name, GLuint indx)
{
	GLuint ambLOC, diffLOC, specLOC, posLOC, absPosLOC,  dirLOC, cutLOC, expLOC, enLOC, typeLOC;
	glm::vec3 viewPos = glm::vec3{vMat * glm::vec4{position,1}};
	std::string fName = name+"["+std::to_string(indx)+"]";
	ambLOC    = glGetUniformLocation(shader, (fName+".ambient").c_str());
	diffLOC   = glGetUniformLocation(shader, (fName+".diffuse").c_str());
	specLOC   = glGetUniformLocation(shader, (fName+".specular").c_str());
	posLOC    = glGetUniformLocation(shader, (fName+".position").c_str());
	absPosLOC = glGetUniformLocation(shader, (fName+".absPosition").c_str());
	dirLOC    = glGetUniformLocation(shader, (fName+".direction").c_str());
	cutLOC    = glGetUniformLocation(shader, (fName+".cutoff").c_str());
	expLOC    = glGetUniformLocation(shader, (fName+".exponent").c_str());
	enLOC     = glGetUniformLocation(shader, (fName+".enabled").c_str());
	typeLOC   = glGetUniformLocation(shader, (fName+".type").c_str());
	glProgramUniform4fv(shader,    ambLOC, 1, glm::value_ptr(ambient));
	glProgramUniform4fv(shader,   diffLOC, 1, glm::value_ptr(diffuse));
	glProgramUniform4fv(shader,   specLOC, 1, glm::value_ptr(specular));
	glProgramUniform3fv(shader,    posLOC, 1, glm::value_ptr(viewPos));
	glProgramUniform3fv(shader, absPosLOC, 1, glm::value_ptr(position));
	glProgramUniform3fv(shader,    dirLOC, 1, glm::value_ptr(direction));
	glProgramUniform1f (shader,    cutLOC, (GLfloat)cutoff);
	glProgramUniform1f (shader,    expLOC, (GLfloat)exponent);
	glProgramUniform1ui(shader,     enLOC, (GLuint) enabled);
	glProgramUniform1ui(shader,   typeLOC, (GLuint) type);
}

Object::~Object()
{
	if(lightIndx!=-1)
		w->replaceLight({.type=LightType::NO_LIGHT},lightIndx);
}

Object::
	Object(World* w, Model m, ShaderPair shader, GLuint texture, Material mat, double scale,
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
	makeOrbiter(World* w, Model m, ShaderPair shader, GLuint texture, Material mat, double scale,
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
	makeAbsolute(World* w, Model m, ShaderPair shader, GLuint texture, Material mat, double scale,
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
	makeRelative(World* w, Model m, ShaderPair shader, GLuint texture, Material mat, double scale,
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
			w->replaceLight({.type=LightType::NO_LIGHT},lightIndx);
		lightIndx=-1;
		return true;
	}

	//find an index for the light (if it does not have one)
	if(lightIndx==-1)
		for(int i=0;i<World::MAX_LIGHTS;++i)
			if(w->lights[i].type==LightType::NO_LIGHT)
			{
				lightIndx=i;
				break;
			}
			else if(i==World::MAX_LIGHTS-1)
				return false;

	//copy light return true
	w->replaceLight(light,lightIndx);
	return true;
}

void Object::setLightEnabled(bool en)
{
	if(lightIndx!=-1)
		w->lights[lightIndx].enabled=en;
}
bool Object::getLightEnabled(){ return lightIndx!=-1 && w->lights[lightIndx].enabled; }

template<void (Object::*action)(std::stack<glm::mat4>& mstack)>
void Object::walk(std::stack<glm::mat4>& mstack)
{
	//account for scale
	mstack.push(mstack.top()*glm::scale<float>(glm::mat4{1.0f},glm::vec3{scale,scale,scale}));

	//account for rotation
	glm::mat4 rotation{1.0f};
	glm::vec3& arot=axisOfRotation;
	rotation *= glm::rotate<float>(glm::mat4{1.0f}, rotProgress*2*glm::pi<float>(), arot);

	mstack.push(mstack.top()*rotation);

	//perform the node action draw/relight
	(this->*action)(mstack);

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
			mstack.push(mstack.top()*rotation*glm::translate(glm::mat4{1.0f},(float)scale*child.p.r.rPos));
		}
		else if(child.posType==PosType::ORBITING)
		{
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
		}
		child.walk<action>(mstack);
		//remove the top matrix whether it cam from the relative or orbiting properties
		if(child.posType==PosType::RELATIVE||child.posType==PosType::ORBITING)
			mstack.pop();
	}
}

void Object::drawAction(std::stack<glm::mat4>& mstack)
{
	GLuint rShader = shader.renderProgram;
	//Draw self
	glUseProgram(rShader);
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


	glm::mat4 mvMat= drawvMat * mstack.top();
	//send the uniforms to the GPU
	GLuint mvHandle = glGetUniformLocation(rShader, "mv_matrix");
	glUniformMatrix4fv(mvHandle,   1, GL_FALSE, glm::value_ptr(mvMat));
	GLuint normHandle = glGetUniformLocation(rShader, "norm_matrix");
	glm::mat4 normMat = glm::transpose(glm::inverse(mvMat));
	glUniformMatrix4fv(normHandle, 1, GL_FALSE, glm::value_ptr(normMat));
	GLuint texEnHandle = glGetUniformLocation(rShader, "texEn");
	glProgramUniform1i(rShader, texEnHandle, texture!=0);
	GLuint atLightHandle = glGetUniformLocation(rShader, "atLight");
	glProgramUniform1i(rShader, atLightHandle, lightIndx);
	mat.glTransfer(rShader, "material");
	//calculate and send the shadow MVP matrices to the GPU
	for(int i=0; i<World::MAX_LIGHTS; ++i)
	{
		//calculate aspect ratio correction factor
		GLfloat res[4];
		glGetFloatv(GL_VIEWPORT, res);
		glm::mat4 shadMVP = B*w->vpMats[i]*mstack.top();
		if(w->lights[i].type==LightType::POSITIONAL)
			shadMVP=mstack.top();

		GLuint shadMVPLOC =
			glGetUniformLocation(rShader,("shadMVP["+std::to_string(i)+"]").c_str());

		glUniformMatrix4fv(shadMVPLOC, 1, GL_FALSE, glm::value_ptr(shadMVP));
	}

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
	glPatchParameteri(GL_PATCH_VERTICES, 3);
//	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	glDrawElements(GL_PATCHES, m.getNumIndices(), GL_UNSIGNED_INT, 0);
}

void Object::relightAction(std::stack<glm::mat4>& mstack)
{
	//compute light position if this is a lighted object
	if(lightIndx!=-1)
	{
		w->lights[lightIndx].position = glm::vec3{mstack.top() * glm::vec4{0,0,0,1}};
	}
}

void Object::shadowAction(std::stack<glm::mat4>& mstack)
{
	GLuint sShader = shader.shadowProgram;
	//Draw self
	glUseProgram(sShader);
	glBindVertexArray(w->vao[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);


	//compute model coords of texture
	glm::mat4 mvp = w->vpMats[currentShadowLight] * mstack.top();

	//send the uniforms to the GPU
	GLuint mvpHandle = glGetUniformLocation(sShader, "mvp_matrix");
	glUniformMatrix4fv(mvpHandle,   1, GL_FALSE, glm::value_ptr(mvp));

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
}

void Object::updatePos(double timePassed)
{
	if(posType==PosType::ORBITING)
		p.o.revProgress = fmod(p.o.revProgress + p.o.orbitRate*timePassed,1);
	rotProgress = fmod(rotProgress +   rotRate*timePassed,1);
}

void Object::overrideAbsPos(glm::vec3 newPos)
{
	if(posType==PosType::ABSOLUTE)
	{
		p.a.pos=newPos;
	}
}
