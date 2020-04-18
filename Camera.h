#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

class Camera
{
	glm::vec3 U; // rightward orientation
	glm::vec3 V; // upward orientation
	glm::vec3 N; // camera direction
	glm::vec3 C; // position

public:
	Camera();
	glm::mat4 getTransform();
	void setPos(glm::vec3 pos);
	void moveBy(glm::vec3 chPos);
	void pan(float radians);
	void pitch(float radians);
	void roll(float radians);
	glm::vec3 getPos();
	glm::vec3 getDir();
};

#endif
