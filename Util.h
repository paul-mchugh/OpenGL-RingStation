#ifndef UTIL_H
#define UTIL_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
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
};

#endif
