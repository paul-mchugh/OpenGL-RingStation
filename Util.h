#ifndef UTIL_H
#define UTIL_H

#include "Model.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <utility>
#include <string>

//constants
const GLuint cubemapFaces[] =
{
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};
const glm::vec3 faceDirv[] =
{
	glm::vec3{ 1, 0, 0},
	glm::vec3{-1, 0, 0},
	glm::vec3{ 0, 1, 0},
	glm::vec3{ 0,-1, 0},
	glm::vec3{ 0, 0, 1},
	glm::vec3{ 0, 0,-1}
};

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
	static GLuint
		loadCubemap(const char* xp, const char* xm,
		            const char* yp, const char* ym,
		            const char* zp, const char *zM);
	static void printGLInfo();
};

struct Material
{
	glm::vec4  ambient;
	glm::vec4  diffuse;
	glm::vec4  specular;
	float      shininess;
	//premade materials
	static Material getCanvas();
	static Material getGold();
	static Material getSilver();
	static Material getBronze();
	static Material getPearl();
	static Material getJade();
	//transfer for GL
	void glTransfer(GLuint shader, std::string name);
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

class Skybox
{
private:
	static GLuint vaoLn[1];
	static GLuint vbo[2];
	static GLuint shader;
	static bool   isInit;
	static Model  m;
	GLuint texture;
public:
	Skybox();
	Skybox(std::string path);
	Skybox(Skybox& other) = delete;
	Skybox& operator=(Skybox& other) = delete;
	Skybox(Skybox&& other);
	Skybox& operator=(Skybox&& other);
	void draw(glm::mat4 p, glm::mat4 v, glm::vec3 pos);
	operator bool() const;
	~Skybox();
};

struct ShaderPair
{
	GLuint renderProgram;
	GLuint shadowProgram;
	bool renderTess, shadowTess;
	ShaderPair(GLuint rProgram=0, bool rTess=false);
	ShaderPair(GLuint rProgram, GLuint sProgram, bool rTess=false, bool sTess=false);
	operator bool() const;
};

class NoiseTexturesGen
{
	static GLuint genNoiseTex(int seed=997, int sx=100, int sy=100, int sz=100);
};
#endif
