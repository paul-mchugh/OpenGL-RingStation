
#include "Util.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>

bool Util::checkOpenGLError()
{
	bool hasErr = false;
	for(int glErr = glGetError();glErr!=GL_NO_ERROR;glErr=glGetError())
	{
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
	GLsizei lineCnt=0, lineMax=0xf;
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
		lines[lineCnt++]=strdup(line.c_str());
		if(lineCnt==lineMax)
		{
			lineMax = lineCnt & 0xf;
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
	std::pair<char**,int> shadFile = Util::readShader(path);
	if(shadFile.first==NULL) return 0;
	GLuint shaderID = glCreateShader(type);
	glShaderSource(shaderID, shadFile.second, shadFile.first, NULL);
	glCompileShader(shaderID);
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compiled);
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
