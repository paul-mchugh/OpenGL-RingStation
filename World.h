#ifndef WORLD_H
#define WORLD_H

#include "Model.h"
#include "Util.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <stack>
#include <memory>

enum class PosType
{
	ABSOLUTE,
	ORBITING,
	RELATIVE
};

enum class LightType : GLuint
{
	NO_LIGHT    = 0,
	AMBIENT     = 1,
	POSITIONAL  = 2,
	DIRECTIONAL = 3,
	SPOTLIGHT   = 4
};

struct Light
{
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
	glm::vec3 position;
	glm::vec3 direction;
	float cutoff;
	float exponent;
	bool enabled=false;
	LightType type=LightType::NO_LIGHT;
	//transfer for GL
	void glTransfer(glm::mat4 vMat, GLuint shader, std::string name, GLuint indx);
};

class Object;
class World
{
	friend class Object;
	static const GLuint VAOcnt = 1;
	static const int MAX_LIGHTS = 10;
	std::vector<std::unique_ptr<Object>> objects;
	GLuint vao[VAOcnt];
	GLuint framebuffer;
	GLuint sinkCM;
	GLuint sinkFM;
	Light lights[MAX_LIGHTS];
	glm::mat4 vpMats[MAX_LIGHTS];
	GLuint shadowTextures[MAX_LIGHTS];
	void replaceLight(Light lNew, GLuint indx);

public:
	glm::vec3 bgColor;
	Light ambient;
	Light directional;
	void init();
	void draw(glm::mat4 vMat);
	void relight();
	void buildShadowBuffers(GLint viewMap, GLint viewFace);
	void update(double timePassed);
	void glTransferLights(glm::mat4 vMat, GLuint shader, std::string name);
	void drawLightVecs(glm::mat4 pMat, glm::mat4 vMat);
};

class Object
{
	friend class World;
private:
	World* w;
	Model m;
	ShaderPair shader;
	GLuint texture;
	GLuint depthMap=0;
	GLuint normalMap=0;
	Material mat;
	GLuint vbo[5];
	std::vector<int> children;
	GLint lightIndx = -1;
	PosType posType;
	double scale;
	union PositionData
	{
		struct Orbit
		{
			double orbitRate; //orbitRate=1/orbitPeriod
			double revProgress; //progress through the orbit
			glm::vec3 axisOfRevolution;
			double ecIntercept;
			double orbitDist;
		} o;
		struct Absolute
		{
			glm::vec3 pos;
		} a;
		struct Relative
		{
			glm::vec3 rPos;
		} r;
	} p;
	glm::vec3 axisOfRotation;
	double rotRate;
	double rotProgress;
	std::size_t id;
	Object(World* w, Model m, ShaderPair shader, GLuint texture, Material mat, double scale,
	       glm::vec3 axisOfRot, double rotationPeriod, double rotationProgress);
public:
	~Object();
	static Object*
	makeOrbiter(World* w, Model m, ShaderPair shader, GLuint texture, Material mat, double scale,
				double orbitPeriod, double revProgress,
	            double incline, double intercept, double dist,
	            glm::vec3 axisOfRot, double rotationPeriod, double rotationProgress);
	static Object*
	makeAbsolute(World* w, Model m, ShaderPair shader, GLuint texture, Material mat, double scale,
	             glm::vec3 pos,
	             glm::vec3 axisOfRot, double rotationPeriod, double rotationProgress);
	static Object*
	makeRelative(World* w, Model m, ShaderPair shader, GLuint texture, Material mat, double scale,
	             glm::vec3 rPos,
	             glm::vec3 axisOfRot, double rotationPeriod, double rotationProgress);
	bool addChild(Object& child);
	bool attachLight(Light& light);
	void setLightEnabled(bool en);
	bool getLightEnabled();
	template<void (Object::*action)(std::stack<glm::mat4>& mstack)>
	void walk(std::stack<glm::mat4>& mstack);
	void drawAction(std::stack<glm::mat4>& mstack);
	void relightAction(std::stack<glm::mat4>& mstack);
	void shadowAction(std::stack<glm::mat4>& mstack);
	void updatePos(double timePassed);
	void overrideAbsPos(glm::vec3 newPos);
	void attachDepthMap(GLuint depthMap);
	void attachNormalMap(GLuint normalMap);
};

#endif
