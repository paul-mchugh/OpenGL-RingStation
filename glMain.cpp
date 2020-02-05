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
void scroll_callback(GLFWwindow* window, double xScrOff, double yScrOff);
void printHelp(void);

//triangle action
enum class TriAction
{
	HORIZONTAL,
	VERTICAL,
	CIRCULAR
};
TriAction state = TriAction::HORIZONTAL;
bool colorIntr = true;
double lastTime=0;
double zoomDelta=0.05;
double zoom = 0.5;
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
	GLFWwindow* window = glfwCreateWindow(600, 600, "CSC155", NULL, NULL);
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
	glGenVertexArrays(numVAOs, vao);
	glBindVertexArray(vao[0]);
	Util::printGLInfo();
	printHelp();
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
	zoom=std::min<double>(1,std::max<double>(0,zoom));

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
	GLuint intrHandle = glGetUniformLocation(renderingProgram, "intr");
	GLuint zoomHandle = glGetUniformLocation(renderingProgram, "zoom");
	glProgramUniform1f(renderingProgram, xOffHandle, x);
	glProgramUniform1f(renderingProgram, yOffHandle, y);
	glProgramUniform1i(renderingProgram, intrHandle, colorIntr);
	glProgramUniform1f(renderingProgram, zoomHandle, zoom);

	glDrawArrays(GL_TRIANGLES, 0,3);
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
		//I know that I am skippiny the shift+/ part of the ? key, but I
		//want it to be easy to press the help key
		printHelp();
	}
	else if(key == GLFW_KEY_H && action == GLFW_PRESS)
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
	else if(key == GLFW_KEY_I && action== GLFW_PRESS)
	{
		colorIntr = !colorIntr;
		printf("Pressed key I: %s color interpolation\n", (colorIntr ? "Enabled":"Disabled"));
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

void printHelp(void)
{
	printf(
		"Usage:\n"
		"Press any of the following keys while the window is selected\n"
		" ?:  View this help message in console(F1 also works)\n"
		" H:  Switch triangle to horizontal movement mode(default mode)\n"
		" V:  Switch triangle to vertical movement mode\n"
		" C:  Switch triangle to circular movement mode\n"
		" I:  Toggle Color Interpolation(default on)\n"
		" 0:  Reset Zoom\n"
		" +:  Increase Zoom(or scroll)\n"
		" -:  Decrease Zoom(or scroll)\n"
		"^Q:  Quit program(^W also works)\n"
	);
}
