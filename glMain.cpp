#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Util.h"

//globals
#define numVAOs 1
GLuint renderingProgram;
GLuint vao[numVAOs];
float x =  0.0f;
float inc = 0.01f;

//forward declarations
void init(GLFWwindow* window);
void display(GLFWwindow* window, double currentTime);

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
	glGenVertexArrays(numVAOs, vao);
	glBindVertexArray(vao[0]);
}

void display(GLFWwindow* window, double currentTime)
{
	glClear(GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0,0.0,0.0,1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(renderingProgram);

	x+=inc;
	if(x>1.0f)			inc=-0.01f;
	else if (x<-1.0f)	inc= 0.01f;
	GLuint offsetHandle = glGetUniformLocation(renderingProgram, "offset");
	glProgramUniform1f(renderingProgram, offsetHandle, x);

	glDrawArrays(GL_TRIANGLES, 0,3);
}
