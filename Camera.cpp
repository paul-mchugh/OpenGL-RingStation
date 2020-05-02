
#include "Camera.h"
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>


Camera::Camera() : U{1,0,0}, V{0,1,0}, N{0,0,1}, C{0,0,0}
{}


glm::mat4 Camera::getTransform(float offset)
{
	//rotate
	glm::mat4 rot{1.0f};
	rot = glm::row(rot,0,glm::vec4{U,0});
	rot = glm::row(rot,1,glm::vec4{V,0});
	rot = glm::row(rot,2,glm::vec4{N,0});

	//translate
	glm::mat4 trans{1.0f};
	trans = glm::translate(trans, -(C+glm::normalize(U)*offset));

	return rot*trans;
}
void Camera::setPos(glm::vec3 pos)
{
	C=glm::vec4{pos,1};
}

void Camera::moveBy(glm::vec3 chPos)
{
	float x = chPos.x, y = chPos.y, z = chPos.z;
	C -= z*N + y*V + x*U;
}

void Camera::pan(float radians)
{
	glm::mat4 panM = glm::rotate<float>(glm::mat4{1.0f}, radians, V);
	U = glm::vec3{panM * glm::vec4{U,1}};
	N = glm::vec3{panM * glm::vec4{N,1}};
}

void Camera::pitch(float radians)
{
	glm::mat4 pitchM = glm::rotate<float>(glm::mat4{1.0f}, radians, U);
	V = glm::vec3{pitchM * glm::vec4{V,1}};
	N = glm::vec3{pitchM * glm::vec4{N,1}};
}

void Camera::roll(float radians)
{
	glm::mat4 rollM = glm::rotate<float>(glm::mat4{1.0f}, radians, N);
	U = glm::vec3{rollM * glm::vec4{U,1}};
	V = glm::vec3{rollM * glm::vec4{V,1}};
}

glm::vec3 Camera::getPos()   { return C; }

glm::vec3 Camera::getDir()   { return N; }
