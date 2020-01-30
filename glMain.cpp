#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

//globals
#define numVAOs 1
GLuint renderingProgram;
GLuint vao[numVAOs];

//forward declarations
void init(GLFWwindow* window);
void display(GLFWwindow* window, double currentTime);
GLuint createShaderProgram();

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
	renderingProgram = createShaderProgram();
	glGenVertexArrays(numVAOs, vao);
	glBindVertexArray(vao[0]);
}

void display(GLFWwindow* window, double currentTime)
{
	//glClearColor(1.0,0.0,0.0,1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(renderingProgram);
	glPointSize(30.0f);
	glDrawArrays(GL_POINTS, 0,1);
}

GLuint createShaderProgram()
{
	const char* vshaderSource =
	"#version 430 \n"
	"void main(void) \n"
	"{ \n"
	"gl_Position = vec4(0.0,0.0,0.0,1.0); \n"
	"} \n";

	const char* fshaderSource =
	"#version 430 \n"
	"out vec4 color; \n"
	"void main(void) \n"
	"{ \n"
	"color = vec4(0.0,0.0,1.0,1.0); \n"
	"} \n";

	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vShader,1,&vshaderSource,NULL);
	glShaderSource(fShader,1,&fshaderSource,NULL);
	glCompileShader(vShader);
	glCompileShader(fShader);

	GLuint vfProgram = glCreateProgram();
	glAttachShader(vfProgram, vShader);
	glAttachShader(vfProgram, fShader);
	glLinkProgram(vfProgram);

	return vfProgram;
}
