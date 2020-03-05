#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Util.h"
#include "Sphere.h"

//globals
const GLsizei numVAOs=1;
const GLsizei numVBOs=1;
GLuint renderingProgram;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];

//forward declarations
void init(GLFWwindow* window);
std::vector<float> setupVertices();
void display(GLFWwindow* window, double currentTime);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow* window, double xScrOff, double yScrOff);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void printHelp(void);

float zoom=0.5;
float zoomDelta=0.05;
Model s;
std::vector<float> sphereData;
glm::vec3 cameraLOC(0,0,8);
glm::vec3 cubeLOC(0,-2,0);
glm::mat4 pMat, vMat, mMat, mvMat;

int main()
{
	if(!glfwInit()) exit(1);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(600, 600, "CSC155: HW2", NULL, NULL);
	glfwMakeContextCurrent(window);
	if(glewInit() != GLEW_OK) exit(1);
	glfwSwapInterval(1);

	init(window);

	while(!glfwWindowShouldClose(window))
	{
		display(window, glfwGetTime());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(0);
}

void init(GLFWwindow* window)
{
	renderingProgram = Util::createShaderProgram("vertShader.glsl", "fragShader.glsl");
	if(!renderingProgram) std::cout << "Could not Load shader" << std::endl;
	glfwSetKeyCallback(window, key_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//sphere data
	s = SphereGenerator::generateSphere();
	sphereData=setupVertices();

	glGenVertexArrays(numVAOs, vao);
	glBindVertexArray(vao[0]);
	glGenBuffers(numVBOs, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER,sphereData.size()*sizeof(float),&sphereData[0],GL_STATIC_DRAW);

	Util::printGLInfo();
	printHelp();
}

std::vector<float> setupVertices()
{
	std::vector<glm::vec3> sphereVertices = s.getVertices();
	//preallocate the array and convert to float array
	std::vector<float> result;
	for(int ind : s.getIndices())
	{
		result.push_back(sphereVertices[ind].x);
		result.push_back(sphereVertices[ind].y);
		result.push_back(sphereVertices[ind].z);
	}
	return result;
}

void display(GLFWwindow* window, double currentTime)
{
	glClear(GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0,0.0,0.0,1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(renderingProgram);

	//setup the vertex array
	glBindVertexArray(vao[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	//build perspective matrix
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float aspect = (float)width/(float)height;
	pMat = glm::perspective(glm::degrees(60.0f), aspect, 0.1f, 1000.0f);

	//build vMatrix, mMatrix, and mvMatrix
	vMat = glm::translate(glm::mat4(1.0f), -cameraLOC);
	mMat = glm::translate(glm::mat4(1.0f), cubeLOC);
	mvMat = vMat * mMat;

	//send the uniforms to the GPU
	GLuint mvHandle = glGetUniformLocation(renderingProgram, "mv_matrix");
	GLuint projHandle = glGetUniformLocation(renderingProgram, "proj_matrix");
	glUniformMatrix4fv(mvHandle, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projHandle, 1, GL_FALSE, glm::value_ptr(pMat));

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, s.getNumIndices());
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if((key == GLFW_KEY_Q||key == GLFW_KEY_W) &&
		action == GLFW_PRESS && mods == GLFW_MOD_CONTROL)
	{
		printf("Pressed exit key: Bye Bye.\n");
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

	if((key == GLFW_KEY_SLASH || key == GLFW_KEY_F1) && action == GLFW_PRESS)
	{
		//I know that I am skipping the shift+/ part of the ? key, but I
		//want it to be easy to press the help key
		printHelp();
	}
	else if((key == GLFW_KEY_0 || key == GLFW_KEY_KP_0) && action == GLFW_PRESS)
	{
		zoom = 0.5;
		printf("Pressed key 0: Zoom reset\n");
	}
	else if((key == GLFW_KEY_KP_ADD || key == GLFW_KEY_EQUAL) && action != GLFW_RELEASE)
	{
		zoom += zoomDelta;
		printf("Pressed key +: Zoom %s\n", (zoom>1.025?"at max":"increased"));
	}
	else if((key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT) && action != GLFW_RELEASE)
	{
		zoom -= zoomDelta;
		printf("Pressed key -: Zoom %s\n", (zoom<-0.025?"at min":"decreased"));
	}
}

void scroll_callback(GLFWwindow* window, double xScrOff, double yScrOff)
{
	if(yScrOff>0)		zoom += zoomDelta;
	else if(yScrOff<0)	zoom -= zoomDelta;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void printHelp(void)
{
	printf(
		"Usage:\n"
		"Press any of the following keys while the window is selected\n"
		" ?:  View this help message in console(F1 also works)\n"
		" 0:  Reset Zoom\n"
		" +:  Increase Zoom(or scroll)\n"
		" -:  Decrease Zoom(or scroll)\n"
		"^Q:  Quit program(^W also works)\n"
	);
}
