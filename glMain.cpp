#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <cmath>
#include "Util.h"

//globals
#define numVAOs 1
GLuint renderingProgram;
GLuint vao[numVAOs];

//forward declarations
void init(GLFWwindow* window);
void display(GLFWwindow* window, double currentTime);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

//triangle action
enum class TriAction
{
	HORIZONTAL,
	VERTICAL,
	CIRCULAR
};
TriAction state = TriAction::HORIZONTAL;
double lastTime=0;
double deltaPerSec=0.1;
double hProgress=1.0;
double vProgress=0.0;
double cProgress=0.0;
double radius=0.75;

int main()
{
	if(!glfwInit()) exit(1);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(600, 600, "Example CSC155", NULL, NULL);
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
	glGenVertexArrays(numVAOs, vao);
	glBindVertexArray(vao[0]);
}

void display(GLFWwindow* window, double currentTime)
{
	glClear(GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0,0.0,0.0,1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(renderingProgram);


	//update transition progress
	double change = (currentTime-lastTime)*deltaPerSec;
	lastTime=currentTime;

	hProgress-=change;
	vProgress-=change;
	cProgress-=change;
	switch(state)
	{
	case TriAction::HORIZONTAL:
		hProgress+=2*change;
		break;
	case TriAction::VERTICAL:
		vProgress+=2*change;
		break;
	case TriAction::CIRCULAR:
		cProgress+=2*change;
		break;
	}

	hProgress=std::min<double>(1,std::max<double>(0,hProgress));
	vProgress=std::min<double>(1,std::max<double>(0,vProgress));
	cProgress=std::min<double>(1,std::max<double>(0,cProgress));

	//position the triangle based off the progress, etc.
	double periodProgress = fmod(currentTime, 4);
	double m = (periodProgress<1 ?
				periodProgress :
				(periodProgress<3 ?
					2-periodProgress :
					periodProgress-4));
	m*=0.75;
	double radians = (periodProgress/2)*M_PI;
	double x = m*hProgress + cProgress*radius*cos(radians);
	double y = m*vProgress + cProgress*radius*sin(radians);

	GLuint xOffHandle = glGetUniformLocation(renderingProgram, "xOff");
	GLuint yOffHandle = glGetUniformLocation(renderingProgram, "yOff");
	glProgramUniform1f(renderingProgram, xOffHandle, x);
	glProgramUniform1f(renderingProgram, yOffHandle, y);

	glDrawArrays(GL_TRIANGLES, 0,3);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if((key == GLFW_KEY_Q||key == GLFW_KEY_W) &&
		action == GLFW_RELEASE && mods == GLFW_MOD_CONTROL)
	{
		printf("Pressed exit key. Bye Bye.\n");
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

	if(key == GLFW_KEY_H && action == GLFW_PRESS)
	{
		state = TriAction::HORIZONTAL;
		printf("Pressed key H: Horizontal triangle movement\n");
	}
	else if(key == GLFW_KEY_V && action == GLFW_PRESS)
	{
		state = TriAction::VERTICAL;
		printf("Pressed key V: Vertical triangle movement\n");
	}
	else if(key == GLFW_KEY_C && action == GLFW_PRESS)
	{
		state = TriAction::CIRCULAR;
		printf("Pressed key C: Circular triangle movement\n");
	}
}
