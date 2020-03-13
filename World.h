#ifndef WORLD_H
#define WORLD_H

#include "Model.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdint>
#include <cmath>
#include <stack>
#include <memory>
#include <utility>

class Orbiter;
class World
{
	friend class Orbiter;
	static const GLuint VAOcnt = 1;
	std::vector<std::unique_ptr<Orbiter>> orbiters;
	GLuint vao[VAOcnt];
	std::size_t rootOrbiter=0;
public:
	glm::vec3 bgColor;
	void init();
	void setRoot(Orbiter& o);
	void draw(std::stack<glm::mat4> mst);
	void update(double timePassed);
};

class Orbiter
{
	friend class World;
private:
	World* w;
	Model m;
	GLuint shader;
	GLuint texture;
	GLuint vbo[5];
	std::vector<int> children;
	double scale;
	double orbitRate; //orbitRate=1/orbitPeriod
	double revProgress; //progress through the orbit
	glm::vec3 axisOfRevolution;
	double ecIntercept;
	double orbitDist;
	glm::vec3 axisOfRotation;
	double rotRate;
	double rotProgress;
	std::size_t id;
public:
	Orbiter(World* w, Model m, GLuint shader, GLuint texture, double scale,
	        double orbitPeriod, double revProgress, double incline, double intercept,
	        double dist, glm::vec3 axisOfRot, double rotationPeriod, double rotationProgress);
	bool addChild(Orbiter& child);
	void draw(std::stack<glm::mat4>& mstack);
	void updatePos(double timePassed);
};

#endif
