#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <stack>
#include <glm/gtc/type_ptr.hpp>
#include "Util.h"
#include "Generator.h"
#include "World.h"
#include "Camera.h"

//globals
const float rotateMagnitude = 30;
const float moveMagnitude = 10;
GLuint renderingProgram;

//forward declarations
void init(GLFWwindow* window);
void display(GLFWwindow* window, double currentTime);
void handleKeys(GLFWwindow* window, double time);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow* window, double xScrOff, double yScrOff);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void printHelp(void);

//global variables
bool axesEnabled=false;
bool paused=false;
double lastTime=0;
glm::vec3 cameraLOC(0,0,80);
Camera c;
World wld;

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
		glfwPollEvents();
		display(window, glfwGetTime());
		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(0);
}

void init(GLFWwindow* window)
{
	renderingProgram = Util::createShaderProgram("modVertShader.glsl", "modFragShader.glsl");
	if(!renderingProgram) std::cout << "Could not Load shader" << std::endl;
	glfwSetKeyCallback(window, key_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	wld.init();

	//setup camera
	c.setPos(cameraLOC);

	//model data
	GLuint sunTexture     = Util::loadTexture("img/sun_euv.png");
	GLuint marsTexture    = Util::loadTexture("img/mars.jpg");
	GLuint europaTexture  = Util::loadTexture("img/europa.jpg");
	GLuint ringWldTexture = Util::loadTexture("img/rw_test.png");
	GLuint earthTexture   = Util::loadTexture("img/earth.jpg");
	GLuint shuttleTexture = Util::loadTexture("img/spstob_1.jpg");

	Model s    = Generator::generateSphere();
	Model s1   = Generator::generateSphere();
	Model s11  = Generator::generateSphere();
	//habitat internal region is height=habwidth, width=2*pi*(1-wallThick)
	//texture aspect ratio = 62.517777
	Model s111 = Generator::generateRingHab(0.1,0.005,0.01,500);
	Model s2   = Generator::generateSphere();
	Model s21  = ModelImporter::parseOBJ("models/shuttle.obj");

	Material canvas = Material::getCanvas();

//	Light sunlight{};

	Object* obs =
		Object::makeAbsolute(&wld, s, renderingProgram, sunTexture, canvas, 3,
		                     glm::vec3{0,0,0},
		                     glm::vec3{0.1f,1,0}, 15, 0.6f);
/*	Orbiter* obs1   = new Orbiter{&wld, s1, renderingProgram, marsTexture, 2,
	                              20, 0, glm::radians(20.0f), glm::radians(280.0f),
	                              15, glm::vec3{0,1,0}, 5, 0.6f};
	Orbiter* obs11  = new Orbiter{&wld, s11, renderingProgram, europaTexture, 1,
	                              25, 0, glm::radians(20.0f), glm::radians(15.0f),
	                              6, glm::vec3{0,1,1}, 5, 0.6f};*/
	Object* obs111 =
		Object::makeAbsolute(&wld, s111, renderingProgram, ringWldTexture, canvas, 100,
		                     glm::vec3{0,0,0},
		                     glm::vec3{0,1,0}, 60, 0);
/*	Orbiter* obs2   = new Orbiter{&wld, s2, renderingProgram, earthTexture, 2,
	                              25, 0, glm::radians(20.0f), glm::radians(180.0f),
	                              20, glm::vec3{0,1,0}, 30, 0.6f};*/
	Object* obs21 =
		Object::makeRelative(&wld, s21, renderingProgram, shuttleTexture, canvas, 1,
		                     glm::vec3{ 0,0,0.95},
		                     glm::vec3{-1,0,0   }, 10, 0.25);

	obs111->addChild(*obs21);

	Util::printGLInfo();
	printHelp();
}

void display(GLFWwindow* window, double currentTime)
{
	double timeDiff=currentTime-lastTime;
	//handle keyboard input
	handleKeys(window, timeDiff);
	//update the objects and relight
	if(!paused)
		wld.update(timeDiff);
	wld.relight();
	lastTime = currentTime;

	//build perspective and view matrices
	glm::mat4 pMat, vMat;
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float aspect = (float)width/(float)height;
	pMat = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 1000.0f);
	vMat = c.getTransform();
	glm::mat4 invvMat = glm::transpose(glm::inverse(vMat));

	//send the uniforms to the GPU
	glUseProgram(renderingProgram);
	GLuint projHandle = glGetUniformLocation(renderingProgram, "proj_matrix");
	glUniformMatrix4fv(projHandle, 1, GL_FALSE, glm::value_ptr(pMat));
	GLuint invvHandle = glGetUniformLocation(renderingProgram, "invv_matrix");
	glUniformMatrix4fv(invvHandle, 1, GL_FALSE, glm::value_ptr(invvMat));
	wld.glTransferLights(vMat, renderingProgram, "lights");

	std::stack<glm::mat4> mst;
	mst.push(glm::mat4(1.0f));
	mst.push(mst.top()*vMat);
	wld.draw(mst);

	if(axesEnabled)
	{
		//draw axes
		LineDrawer::draw(pMat, vMat, glm::vec3(0,0,0), glm::vec3(9,0,0), glm::vec3(1,0,0));
		LineDrawer::draw(pMat, vMat, glm::vec3(0,0,0), glm::vec3(0,9,0), glm::vec3(0,1,0));
		LineDrawer::draw(pMat, vMat, glm::vec3(0,0,0), glm::vec3(0,0,9), glm::vec3(0,0,1));
	}
}

#define GK(key) (glfwGetKey(window,GLFW_KEY_ ## key ) == GLFW_PRESS)

void handleKeys(GLFWwindow* window, double time)
{
	if(GK(W)) c.moveBy(glm::vec3{0,0, moveMagnitude*time});
	if(GK(S)) c.moveBy(glm::vec3{0,0,-moveMagnitude*time});
	if(GK(E)) c.moveBy(glm::vec3{0, moveMagnitude*time,0});
	if(GK(Q)) c.moveBy(glm::vec3{0,-moveMagnitude*time,0});
	if(GK(A)) c.moveBy(glm::vec3{ moveMagnitude*time,0,0});
	if(GK(D)) c.moveBy(glm::vec3{-moveMagnitude*time,0,0});
	if(GK(UP)            ) c.pitch(glm::radians( rotateMagnitude*time));
	if(GK(DOWN)          ) c.pitch(glm::radians(-rotateMagnitude*time));
	if(GK(LEFT)          ) c.pan  (glm::radians( rotateMagnitude*time));
	if(GK(RIGHT)         ) c.pan  (glm::radians(-rotateMagnitude*time));
	if(GK(LEFT_BRACKET)  ) c.roll (glm::radians( rotateMagnitude*time));
	if(GK(RIGHT_BRACKET) ) c.roll (glm::radians(-rotateMagnitude*time));
}

#undef GK

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//these keys are special and deserve a callback
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
	else if(key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		c = Camera();
		c.setPos(cameraLOC);
		printf("Pressed key  R: Camera reset\n");
	}
	else if(key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		paused = !paused;
		printf("Pressed key  P: Time %s\n",(paused?"Paused":"Unpaused"));
	}
	else if(key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		axesEnabled = !axesEnabled;
		printf("Pressed key SP: Toggled World axes to %s\n", (axesEnabled?"on":"off"));
	}
}

void scroll_callback(GLFWwindow* window, double xScrOff, double yScrOff)
{
	//do nothing for now
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
		"  ?:  View this help message in console(F1 also works)\n"
		" ^Q:  Quit program(^W also works)\n"
		"  R:  Reset camera to original position ((0,0,30), looking a the origin)\n"
		"  P:  Toggle Pause time(default: off)\n"
		" SP:  Use space bar to toggle axes(default: off)\n"
		"W/S:  Forward/Backward\n"
		"A/D:  Strafe left/Strafe right\n"
		"Q/E:  Up/Down\n"
		" /\\:  Use up arrow key to pitch up\n"
		"</>:  Use Left/Right arrow keys to pan Left/Right\n"
		" \\/:  Use down arrow key to pitch down\n"
		"[/]:  Use the open and close brackets to roll Left/Right\n"
	);
}
