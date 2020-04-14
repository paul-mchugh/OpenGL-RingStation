#ifndef WORLD_H
#define WORLD_H

#include "Model.h"
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
	bool enabled;
	LightType type;
	//transfer for GL
	void glTransfer(GLuint shader, std::string name, GLuint indx);
};

class Object;
class World
{
	friend class Object;
	static const GLuint VAOcnt = 1;
	static const int MAX_LIGHTS = 10;
	std::vector<std::unique_ptr<Object>> objects;
	GLuint vao[VAOcnt];
	Light lights[MAX_LIGHTS];
public:
	glm::vec3 bgColor;
	void init();
	void draw(std::stack<glm::mat4> mst);
	void update(double timePassed);
	void relight(bool newLight=false);
};

class Object
{
	friend class World;
private:
	World* w;
	Model m;
	GLuint shader;
	GLuint texture;
	GLuint vbo[5];
	std::vector<int> children;
	int lightIndx = -1;
	bool lightChild = false;
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
	Object(World* w, Model m, GLuint shader, GLuint texture, double scale,
	       glm::vec3 axisOfRot, double rotationPeriod, double rotationProgress);
public:
	~Object();
	static Object*
	makeOrbiter(World* w, Model m, GLuint shader, GLuint texture, double scale,
				double orbitPeriod, double revProgress,
	            double incline, double intercept, double dist,
	            glm::vec3 axisOfRot, double rotationPeriod, double rotationProgress);
	static Object*
	makeAbsolute(World* w, Model m, GLuint shader, GLuint texture, double scale,
	             glm::vec3 pos,
	             glm::vec3 axisOfRot, double rotationPeriod, double rotationProgress);
	static Object*
	makeRelative(World* w, Model m, GLuint shader, GLuint texture, double scale,
	             glm::vec3 rPos,
	             glm::vec3 axisOfRot, double rotationPeriod, double rotationProgress);
	bool addChild(Object& child);
	bool attachLight(Light& light);
	void setLightEnabled(bool en);
	bool getLightEnabled();
	void draw(std::stack<glm::mat4>& mstack);
	void updatePos(double timePassed);
	bool relight(std::stack<glm::mat4>& mstack, bool newLight);
};

#endif
