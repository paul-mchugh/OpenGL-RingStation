#ifndef WORLD_H
#define WORLD_H

#include "Model.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <memory>
#include <utility>

#define VAOcnt 1

class Orbiter;
class World
{
public:
	std::vector<std::unique_ptr<Orbiter>> orbiters;
	GLuint vaos[VAOcnt];
	glm::vec3 bgColor;
};

class Orbiter
{
private:
	World* w;
	Model m;
	GLuint shader;
	GLuint texture;
	GLuint vbo[5];
	std::vector<int> children;
	double orbitRate; //orbitRate=1/orbitPeriod
	double progress; //progress through the orbit
	double angleOfInclination;
	double angleOfEclipticIntercept;
	double orbitDist;
	glm::vec3 axisOfRotation;
	double rotRate;
	double rotProgress;
	int copyCnt=1;
public:
	Orbiter(World* w, Model m, GLuint shader, GLuint texture,
	        double orbitPeriod, double progress, double inclination,
	        double intercept, double dist,
	        glm::vec3 axis, double rotationPeriod, double rotationProgress);
	void addChildByIndex();
	std::vector<int> getChildren();
	void draw(World& w, std::vector<glm::mat4>);
	virtual void updatePos()=0;
};

#endif
