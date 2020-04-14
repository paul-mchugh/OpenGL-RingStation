
#include "Util.h"
#include <SOIL2/SOIL2.h>
#include <glm/gtc/type_ptr.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>

GLuint LineDrawer::vaoLn[1]={0};
GLuint LineDrawer::shader=0;
bool LineDrawer::isInit=false;

bool Util::checkOpenGLError()
{
	bool hasErr = false;
	for(int glErr = glGetError();glErr!=GL_NO_ERROR;glErr=glGetError())
	{
		hasErr = true;
		printf("glError:\n%d: %s\n", glErr, Util::errCodeToString(glErr));
	}
	return hasErr;
}

void Util::printShaderLog(GLuint shader)
{
	GLint len;
	GLsizei lenRV;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
	if(len>0)
	{
		char* log=(char*)malloc(len*sizeof(*log));
		glGetShaderInfoLog(shader, len, &lenRV, log);
		printf("Shader Info Log:\n%s\n", log);
		free(log);
	}
}

void Util::printProgramLog(GLuint prog)
{
	GLint len;
	GLsizei lenRV;
	glGetShaderiv(prog, GL_INFO_LOG_LENGTH, &len);
	if(len>0)
	{
		char* log=(char*)malloc(len*sizeof(*log));
		glGetProgramInfoLog(prog, len, &lenRV, log);
		printf("Program Info Log:\n%s\n", log);
		free(log);
	}
}

constexpr const char* Util::errCodeToString(GLenum err)
{
	switch(err)
	{
	case GL_NO_ERROR:
		return "No Error";
	case GL_INVALID_ENUM:
		return "Invalid Enum";
	case GL_INVALID_VALUE:
		return "Invalid Value";
	case GL_INVALID_OPERATION:
		return "Invalid Operation";
	case GL_STACK_OVERFLOW:
		return "Stack Overflow";
	case GL_STACK_UNDERFLOW:
		return "Stack Underflow";
	case GL_OUT_OF_MEMORY:
		return "Out of Memory";
	default:
		return "Unknown Error";
	}
}

std::pair<char**, GLsizei> Util::readShader(const char* path)
{
	//check path for null
	if(path==NULL)
	{
		return std::make_pair<char**,GLsizei>(NULL,0);
	}
	GLsizei lineCnt=0, lineMax=0x10;
	std::string line;
	std::ifstream fStr{path};
	if(!fStr.is_open())	//if the file can't be opened then return {NULL,0}
	{
		return std::make_pair<char**,GLsizei>(NULL,0);
	}
	char** lines = (char**)malloc(lineMax*sizeof(*lines));
	while (!fStr.eof())
	{
		getline(fStr, line);
		line+='\n';
		lines[lineCnt++]=(char*)malloc(line.length()+1);
		strcpy(lines[lineCnt-1], line.c_str());
		if(lineCnt==lineMax)
		{
			lineMax += 0x10;
			lines = (char**)realloc(lines,lineMax*sizeof(*lines));
		}
	}
	return std::make_pair(lines,lineCnt);
}

void Util::deleteShaderFromMainMemory(std::pair<char**, GLsizei> shader)
{
	for(int i=0;i<shader.second;++i)
	{
		free(shader.first[i]);
	}
	free(shader.first);
}

GLuint Util::createShader(GLenum type, const char* path)
{
	GLint compiled=0;
	//create shader
	std::pair<char**,GLsizei> shadFile = Util::readShader(path);
	if(shadFile.first==NULL) return 0;
	GLuint shaderID = glCreateShader(type);
	glShaderSource(shaderID, shadFile.second, shadFile.first, NULL);
	glCompileShader(shaderID);
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compiled);
	Util::deleteShaderFromMainMemory(shadFile);
	if(compiled)
	{
		return shaderID;
	}
	else
	{
		Util::printShaderLog(shaderID);
		glDeleteShader(shaderID);
		return 0;
	}
}

GLuint Util::createShaderProgram(	const char* vp, const char* fp,
									const char* tCS, const char* tES,
									const char* gp)
{
	GLuint vShader=0;
	GLuint fShader=0;
	GLuint tCSShader=0;
	GLuint tESShader=0;
	GLuint gShader=0;

	if(vp==NULL||fp==NULL)
	{
		return 0;
	}

	vShader   =Util::createShader(GL_VERTEX_SHADER, vp);
	fShader   =Util::createShader(GL_FRAGMENT_SHADER, fp);
	tCSShader =Util::createShader(GL_TESS_CONTROL_SHADER, tCS);
	tESShader =Util::createShader(GL_TESS_EVALUATION_SHADER, tES);
	gShader   =Util::createShader(GL_GEOMETRY_SHADER, gp);

	GLuint program = glCreateProgram();
	if(!program||!vShader||!fShader||(!tCSShader&&tCS)||(!tESShader&&tES)||(!gShader&&gp))
	{
		Util::checkOpenGLError();
		if(vShader)glDeleteShader(vShader);
		if(fShader)glDeleteShader(fShader);
		if(tCSShader)glDeleteShader(tCSShader);
		if(tESShader)glDeleteShader(tESShader);
		if(gShader)glDeleteShader(gShader);
		if(program)glDeleteProgram(program);
		return 0;
	}
	glAttachShader(program,vShader);
	glAttachShader(program,fShader);
	if(tCSShader)
		glAttachShader(program,tCSShader);
	if(tESShader)
		glAttachShader(program,tESShader);
	if(gShader)
		glAttachShader(program,gShader);

	GLint linked=0;
	glLinkProgram(program);
	//check for errors
	Util::checkOpenGLError();
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if(linked)
		return program;
	else
	{
		Util::printProgramLog(program);
		glDeleteProgram(program);
		glDeleteShader(vShader);
		glDeleteShader(fShader);
		if(tCSShader)glDeleteShader(tCSShader);
		if(tESShader)glDeleteShader(tESShader);
		if(gShader)glDeleteShader(gShader);
		return 0;
	}

}

GLuint Util::loadTexture(const char* filename)
{
	GLuint textureID =
		SOIL_load_OGL_texture(filename, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	if(textureID == 0) printf("Could not find texture file '%s'\n", filename);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	return textureID;
}

void Util::printGLInfo()
{
	const GLubyte* vendorStr	= glGetString(GL_VENDOR);
	const GLubyte* rendererStr	= glGetString(GL_RENDERER);
	const GLubyte* versionStr	= glGetString(GL_VERSION);
	const GLubyte* GLSLVerStr	= glGetString(GL_SHADING_LANGUAGE_VERSION);

	printf(	"Vendor:       %s\n"
			"Renderer:     %s\n"
			"Version:      %s\n"
			"GLSL Version: %s\n",
			vendorStr, rendererStr, versionStr, GLSLVerStr);
}

Material Material::getCanvas(){
	return Material{
			.ambient{1.0f, 1.0f, 1.0f, 1}, .diffuse{1.0f, 1.0f, 1.0f, 1},
			.specular{1.0f, 1.0f, 1.0f, 1}, .shininess{25.6f}};
}
Material Material::getGold(){
	return Material{
			.ambient{0.2473f, 0.1995f, 0.0745f, 1}, .diffuse{0.7516f, 0.6065f, 0.2265f, 1},
			.specular{0.6283f, 0.5559f, 0.3661f, 1}, .shininess{51.2f}};
}
Material Material::getSilver(){
	return Material{
			.ambient{0.1923f, 0.1923f, 0.1923f, 1}, .diffuse{0.5075f, 0.5075f, 0.5075f, 1},
			.specular{0.5083f, 0.5083f, 0.5083f, 1}, .shininess{51.2f}};
}
Material Material::getBronze(){
	return Material{
			.ambient{0.2125f, 0.1275f, 0.0540f, 1}, .diffuse{0.7140f, 0.4284f, 0.1814f, 1},
			.specular{0.3936f, 0.2719f, 0.1667f, 1}, .shininess{51.2f}};
}
Material Material::getPearl(){
	return Material{
			.ambient{0.25f, 0.20725f, 0.20725f, 0.9222f}, .diffuse{1.0f, 0.829f, 0.829f, 0.922f},
			.specular{0.2966f, 0.2966f, 0.2966f, 0.922f}, .shininess{51.2f}};
}
Material Material::getJade(){
	return Material{
			.ambient{0.135f, 0.2225f, 0.1575f, 0.95}, .diffuse{0.54f, 0.89f, 0.63f, 0.95f},
			.specular{0.3162f, 0.3162, 0.3162f, 0.95f}, .shininess{12.8f}};
}

void Material::glTransfer(GLuint shader, std::string name)
{
	GLuint ambLOC, diffLOC, specLOC, shiLOC;
	ambLOC  = glGetUniformLocation(shader, (name+".ambient").c_str());
	diffLOC = glGetUniformLocation(shader, (name+".diffuse").c_str());
	specLOC = glGetUniformLocation(shader, (name+".specular").c_str());
	shiLOC  = glGetUniformLocation(shader, (name+".shininess").c_str());
	glProgramUniform4fv(shader,  ambLOC, 1, (GLfloat*)&ambient);
	glProgramUniform4fv(shader, diffLOC, 1, (GLfloat*)&diffuse);
	glProgramUniform4fv(shader, specLOC, 1, (GLfloat*)&specular);
	glProgramUniform4fv(shader,  shiLOC, 1, (GLfloat*)&shininess);
}

void LineDrawer::draw(glm::mat4 p, glm::mat4 v, glm::vec3 src, glm::vec3 dst, glm::vec3 color)
{
	if(!isInit)
	{
		glGenVertexArrays(1, vaoLn);
		shader = Util::createShaderProgram("lineVertShader.glsl", "lineFragShader.glsl");
		isInit = true;
	}

	GLuint pMatHandle  = glGetUniformLocation(shader, "pMat");
	GLuint vMatHandle  = glGetUniformLocation(shader, "vMat");
	GLuint srcHandle   = glGetUniformLocation(shader, "src");
	GLuint dstHandle   = glGetUniformLocation(shader, "dst");
	GLuint colorHandle = glGetUniformLocation(shader, "color");
	glUseProgram(shader);
	glBindVertexArray(vaoLn[0]);
	glUniformMatrix4fv(pMatHandle, 1, GL_FALSE, glm::value_ptr(p));
	glUniformMatrix4fv(vMatHandle, 1, GL_FALSE, glm::value_ptr(v));
	glUniform3fv(srcHandle, 1, glm::value_ptr(src));
	glUniform3fv(dstHandle, 1, glm::value_ptr(dst));
	glUniform3fv(colorHandle, 1, glm::value_ptr(color));

	glLineWidth(5);
	glDrawArrays(GL_LINES,0,2);
}
