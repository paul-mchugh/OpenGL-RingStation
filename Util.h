#ifndef UTIL_H
#define UTIL_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <utility>

class Util
{
private:
	static std::pair<char**, GLsizei> readShader(const char* path);
	static void deleteShaderFromMainMemory(std::pair<char**, GLsizei> shader);
	static GLuint createShader(GLenum type, const char* path);
public:
	static bool checkOpenGLError();
	static void printShaderLog(GLuint shader);
	static void printProgramLog(GLuint prog);
	constexpr static const char* errCodeToString(GLenum err);
	static GLuint createShaderProgram(	const char* vp, const char* fp,
										const char* tCS=NULL, const char* tES=NULL,
										const char* gp=NULL);
	static GLuint loadTexture(const char* filename);
	static void printGLInfo();
};

class LineDrawer
{
private:
	static GLuint vaoLn[1];
	static GLuint shader;
	static bool isInit;
public:
	static void draw(glm::mat4 p, glm::mat4 v, glm::vec3 src, glm::vec3 dst, glm::vec3 color);
};

#endif
