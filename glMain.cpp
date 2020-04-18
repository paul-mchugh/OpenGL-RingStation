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
const float spotOff = 5;
ShaderPair renderingPrograms;

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
GLint viewMap = -1;
double lastTime=0;
glm::vec3 cameraLOC(0,0,-80);
Camera c;
World wld;

int main()
{
	if(!glfwInit()) exit(1);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(600, 600, "CSC155: HW3", NULL, NULL);
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
	renderingPrograms =
		ShaderPair{Util::createShaderProgram("modVertShader.glsl", "modFragShader.glsl"),
		           Util::createShaderProgram("shadVertShader.glsl", "shadFragShader.glsl")};
	if(!renderingPrograms) printf("Could not Load shader\n");
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

	//habitat internal region is height=habwidth, width=2*pi*(1-wallThick)
	//texture aspect ratio = 62.517777
	Model sunm  = Generator::generateSphere();
	Model rh   = Generator::generateRingHab(0.1,0.005,0.01,500);
	Model shu  = ModelImporter::parseOBJ("models/shuttle.obj");
	Model s2   = Generator::generateSphere();
	Model spot  = Generator::generateSphere();

	Material canvas = Material::getCanvas();
	Material silver = Material::getSilver();
	Material   gold = Material::getGold();

	Light sunlight{.ambient=glm::vec4{0,0,0,1}, .diffuse=glm::vec4{1,1,1,1},
	               .specular=glm::vec4{0.5,0.5,0.5,1},
	               .enabled=true,.type=LightType::POSITIONAL};
	Light sllzp{.ambient=glm::vec4{0,0,0,1}, .diffuse=glm::vec4{1,1,1,1},
	            .specular=glm::vec4{0.5,0.5,0.5,1},.direction=glm::vec4{ 0,0, 1,1},
	            .cutoff=glm::radians(22.5f),.exponent=30,
	            .enabled=true,.type=LightType::SPOTLIGHT};
	Light sllzm{.ambient=glm::vec4{0,0,0,1}, .diffuse=glm::vec4{1,1,1,1},
	            .specular=glm::vec4{0.5,0.5,0.5,1},.direction=glm::vec4{ 0,0,-1,1},
	            .cutoff=glm::radians(22.5f),.exponent=30,
	            .enabled=true,.type=LightType::SPOTLIGHT};
	Light sllxp{.ambient=glm::vec4{0,0,0,1}, .diffuse=glm::vec4{1,1,1,1},
	            .specular=glm::vec4{0.5,0.5,0.5,1},.direction=glm::vec4{ 1,0, 0,1},
	            .cutoff=glm::radians(22.5f),.exponent=30,
	            .enabled=true,.type=LightType::SPOTLIGHT};
	Light sllxm{.ambient=glm::vec4{0,0,0,1}, .diffuse=glm::vec4{1,1,1,1},
	            .specular=glm::vec4{0.5,0.5,0.5,1},.direction=glm::vec4{-1,0, 0,1},
	            .cutoff=glm::radians(22.5f),.exponent=30,
	            .enabled=true,.type=LightType::SPOTLIGHT};

	float zOff = -0.1;
	Object* slzp =
		Object::makeAbsolute(&wld, spot, renderingPrograms, 0, silver, 1,
		                     glm::vec3{       0, zOff, spotOff},
		                     glm::vec3{0,1,0}, 0, 0.0f);
	Object* slzm =
		Object::makeAbsolute(&wld, spot, renderingPrograms, 0, silver, 1,
		                     glm::vec3{       0, zOff,-spotOff},
		                     glm::vec3{0,1,0}, 0, 0.25f);
	Object* slxp =
		Object::makeAbsolute(&wld, spot, renderingPrograms, 0, silver, 1,
		                     glm::vec3{ spotOff, zOff, 0},
		                     glm::vec3{0,1,0}, 0, 0.5f);
	Object* slxm =
		Object::makeAbsolute(&wld, spot, renderingPrograms, 0, silver, 1,
		                     glm::vec3{-spotOff, zOff, 0},
		                     glm::vec3{0,1,0}, 0, 0.75f);
	Object* sun =
		Object::makeAbsolute(&wld, sunm, renderingPrograms, 0, gold, 1,
		                     glm::vec3{-64,0,-64},
		                     glm::vec3{0.1f,1,0}, 15, 0.6f);
	Object* ringHab =
		Object::makeAbsolute(&wld, rh, renderingPrograms, ringWldTexture, canvas, 100,
		                     glm::vec3{0,0,0},
		                     glm::vec3{0,1,0}, 60, 0);
	Object* shuttle =
		Object::makeRelative(&wld, shu, renderingPrograms, shuttleTexture, canvas, 1,
		                     glm::vec3{ 0,0,0.95},
		                     glm::vec3{-1,0,0   }, 10, 0.25);

	sun->attachLight(sunlight);
	slxp->attachLight(sllxp);
	slzm->attachLight(sllzm);
	slxm->attachLight(sllxm);
	slzp->attachLight(sllzp);

	ringHab->addChild(*shuttle);


	Util::printGLInfo();
	printHelp();
	Util::checkOpenGLError();
	printf("init end\n");
}

void display(GLFWwindow* window, double currentTime)
{
	double timeDiff=currentTime-lastTime;
	//handle keyboard input
	handleKeys(window, timeDiff);
	//update the objects and relight
	if(!paused)
		wld.update(timeDiff);
	lastTime = currentTime;

	//build perspective and view matrices
	glm::mat4 pMat, vMat;
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float aspect = (float)width/(float)height;
	pMat = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 1000.0f);
	vMat = c.getTransform();
	glm::mat4 invvMat = glm::transpose(glm::inverse(vMat));

	//relight computes the actual positions of all the lights(not just relative positions)
	wld.relight();
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(2.0f,4.0f);
	wld.buildShadowBuffers(viewMap);
	glDisable(GL_POLYGON_OFFSET_FILL);


	//restore program to state for rendering after running shadow programs
	if(viewMap==-1)glBindFramebuffer(GL_FRAMEBUFFER,0);
	glDrawBuffer(viewMap==-1?GL_FRONT:GL_NONE);
	if(viewMap==-1)glViewport(0,0,width,height);

	//send the uniforms to the GPU
	glUseProgram(renderingPrograms.renderProgram);
	GLuint projHandleR = glGetUniformLocation(renderingPrograms.renderProgram, "proj_matrix");
	glUniformMatrix4fv(projHandleR, 1, GL_FALSE, glm::value_ptr(pMat));
	GLuint invvHandleR = glGetUniformLocation(renderingPrograms.renderProgram, "invv_matrix");
	glUniformMatrix4fv(invvHandleR, 1, GL_FALSE, glm::value_ptr(invvMat));
	wld.glTransferLights(vMat, renderingPrograms.renderProgram, "lights");

	wld.draw(vMat);

	if(axesEnabled)
	{
		//draw axes
		LineDrawer::draw(pMat, vMat, glm::vec3(0,0,0), glm::vec3(9,0,0), glm::vec3(1,0,0));
		LineDrawer::draw(pMat, vMat, glm::vec3(0,0,0), glm::vec3(0,9,0), glm::vec3(0,1,0));
		LineDrawer::draw(pMat, vMat, glm::vec3(0,0,0), glm::vec3(0,0,9), glm::vec3(0,0,1));
		wld.drawLightVecs(pMat, vMat);
	}
	Util::checkOpenGLError();
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
		printf("Pressed key   R: Camera reset\n");
	}
	else if(key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		paused = !paused;
		printf("Pressed key   P: Time %s\n",(paused?"Paused":"Unpaused"));
	}
	else if(key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		axesEnabled = !axesEnabled;
		printf("Pressed key  SP: Toggled World axes to %s\n", (axesEnabled?"on":"off"));
	}
	else if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		printf("Pressed key ESC: %sendering\n", (viewMap==-1?"Already r":"R"));
		viewMap=-1;
	}
	else if(key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_9 &&
	        (mods & GLFW_MOD_CONTROL) && action == GLFW_PRESS)
	{
		int newMap =(key-GLFW_KEY_KP_0);
		printf("Pressed key  ^%d: Viewing shadow map for light %d\n", newMap, newMap);
		viewMap=(GLint)newMap;
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
		" SP:  Use space bar to toggle axes and other debug info(default: off)\n"
		"W/S:  Forward/Backward\n"
		"A/D:  Strafe left/Strafe right\n"
		"Q/E:  Up/Down\n"
		" /\\:  Use up arrow key to pitch up\n"
		"</>:  Use Left/Right arrow keys to pan Left/Right\n"
		" \\/:  Use down arrow key to pitch down\n"
		"[/]:  Use the open and close brackets to roll Left/Right\n"
		"ESC:  Return to normal rendering mode\n"
		" ^#:  Where # is a number 0-9 inclusive on the keypad.  See the shadow map for light #\n"
	);
}
