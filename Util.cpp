
#include "Util.h"
#include "Generator.h"
#include <SOIL2/SOIL2.h>
#include <glm/gtc/type_ptr.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>

GLuint LineDrawer::vaoLn[1]={0};
GLuint LineDrawer::shader=0;
bool   LineDrawer::isInit=false;
GLuint Skybox::vaoLn[1]={0};
GLuint Skybox::vbo[2]={0};
GLuint Skybox::shader=0;
bool   Skybox::isInit=false;
Model  Skybox::m;

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
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
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

GLuint
	Util::loadCubemap(const char* xp, const char* xm,
	                  const char* yp, const char* ym,
	                  const char* zp, const char* zm)
{
	if(!xp|!xm|!yp|!ym|!zp|!zm)
		return 0;

	const char* ffName[6]={xp,xm,yp,ym,zp,zm};

	//create and bind the cubemap
	GLuint texID;
	glGenTextures(1,&texID);
	if(Util::checkOpenGLError())
		printf("Error: loading Cubemap\n");

	glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

	//add the cubemap faces
	for(int f=0;f<6;++f)
	{
		int width,height,channels;
		unsigned char* img =
			SOIL_load_image(ffName[f], &width, &height, &channels, SOIL_LOAD_AUTO );
		//if we could not load the image stop and delete the texture
		if(!img)
		{
			glDeleteTextures(1,&texID);
			printf("Error: loading file %s\n",ffName[f]);
			return 0;
		}

		GLint fmt;
		switch(channels)
		{
			case 1: fmt=GL_RED;  break;
			case 2: fmt=GL_RG;   break;
			case 3: fmt=GL_RGB;  break;
			default:
			case 4: fmt=GL_RGBA; break;
		}
		glTexImage2D(cubemapFaces[f], 0, fmt, width, height, 0, fmt,
		             GL_UNSIGNED_BYTE, ((unsigned char*)img));

		//delete the image data
		SOIL_free_image_data(img);
	}

	return texID;
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
	glProgramUniform4fv(shader,  ambLOC, 1, glm::value_ptr(ambient));
	glProgramUniform4fv(shader, diffLOC, 1, glm::value_ptr(diffuse));
	glProgramUniform4fv(shader, specLOC, 1, glm::value_ptr(specular));
	glProgramUniform1f (shader,  shiLOC, (GLfloat)shininess);
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

Skybox::Skybox(): texture(0) {}
Skybox::Skybox(std::string path)
{
	if(!isInit)
	{
		m = Generator::generateBox();
		glGenVertexArrays(1, vaoLn);
	    glBindVertexArray(vaoLn[0]);
		glGenBuffers(2, vbo);
	    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, 3*m.getNumVertices()*sizeof(float),
		             &m.getVertices()[0],GL_STATIC_DRAW);
	    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3*m.getNumIndices()*sizeof(float),
		             &m.getIndices()[0],GL_STATIC_DRAW);
		shader = Util::createShaderProgram("skyboxVertShader.glsl", "skyboxFragShader.glsl");
		if(!shader)
		{
			printf("Could not load cubemap skybox shader\n");
			return;
		}
		isInit = true;
	}

	std::string right  = path+"/right.png";
	std::string left   = path+"/left.png";
	std::string top    = path+"/top.png";
	std::string bottom = path+"/bottom.png";
	std::string front  = path+"/front.png";
	std::string back   = path+"/back.png";

	texture =
		Util::loadCubemap(
			right.c_str(), left.c_str(),
			top.c_str(),   bottom.c_str(),
			front.c_str(), back.c_str());

	if(!texture)
	{
		printf("Could not load cubemap skybox image\n");
		return;
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}
Skybox::Skybox(Skybox&& other): texture(other.texture) {other.texture=0;};
Skybox& Skybox::operator=(Skybox&& other)
{
	texture = other.texture;
	other.texture=0;
	return *this;
}
void Skybox::draw(glm::mat4 p, glm::mat4 v, glm::vec3 pos)
{
	glUseProgram(shader);
    //send vertices
	glBindVertexArray(vaoLn[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

	//send uniforms
	GLuint pMatHandle  = glGetUniformLocation(shader, "pMat");
	GLuint vMatHandle  = glGetUniformLocation(shader, "vMat");
	glUniformMatrix4fv(pMatHandle, 1, GL_FALSE, glm::value_ptr(p));
	glUniformMatrix4fv(vMatHandle, 1, GL_FALSE, glm::value_ptr(v));

	//activate/send texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

	//render
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glDisable(GL_DEPTH_TEST);
	glDrawElements(GL_TRIANGLES, m.getNumIndices(), GL_UNSIGNED_INT, 0);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CW);

}
Skybox::operator bool() const
{
	return isInit && texture;
}
Skybox::~Skybox()
{
	glDeleteTextures(1, &texture);
}


ShaderPair::ShaderPair(GLuint rProgram, bool rTess) : ShaderPair{rProgram,rProgram} {};
ShaderPair::ShaderPair(GLuint rProgram, GLuint sProgram, bool rTess, bool sTess):
	renderProgram{rProgram}, shadowProgram{sProgram},
	renderTess{rTess}, shadowTess{sTess} {};
ShaderPair::operator bool() const
{
	return renderProgram || shadowProgram;
}

