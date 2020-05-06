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


enum AnaglyphMode
{
	OFF=0,
	RED_CYAN=1,
	GREEN_PURPLE=2
};

//globals consts
const float rotateMagnitude = 30;
const float moveMagnitude = 10;
const float spotOff = 5;
const float IOD = 0.02f;
const char* const modeToStr[]={"Off","Red-Cyan","Green-Purple"};

//forward declarations
void init(GLFWwindow* window);
void display(GLFWwindow* window, double currentTime);
void setupScene(void);
glm::mat4 compPerspective(float fov, float aspect, float near, float far, float IOD, float lr);
void handleKeys(GLFWwindow* window, double time);
void handleUserLight();
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow* window, double xScrOff, double yScrOff);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void DebugCB(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,
             const GLchar* message, const GLvoid* userParam);
void printHelp(void);

//global variables
bool axesEnabled=false;
bool paused=false;

GLint viewMap = -1;
GLint viewFace = 0;
double lastTime=0;
AnaglyphMode anaMode=AnaglyphMode::OFF;
glm::vec3 initialCameraLOC(-90.21,21.44,-19.81);
float initialPitch=-25.0f;
float initialPan=-50.0f;
glm::vec3 initialLightLOC(-64.67,13.27,-40.90);
glm::vec3 initialLightDIR(-0.04,-0.38,-0.92);
ShaderPair renderingPrograms;
ShaderPair renderingProgramsTess;
Camera c;
World wld;
Skybox sbox;
//movable light
Object* iLight;
bool grabbedLight=false;
Light userlight;

int main()
{
	if(!glfwInit()) exit(1);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(800, 800, "CSC155: HW4", NULL, NULL);
	glfwMakeContextCurrent(window);
	if(glewInit() != GLEW_OK) exit(1);
	glfwSwapInterval(1);

//	glEnable(GL_DEBUG_OUTPUT);
	//gl debug
	if(GLEW_ARB_debug_output)
		glDebugMessageCallbackARB(&DebugCB, NULL);
	else
		printf("GLEW could not load ARB_debug_output\n");

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
	renderingProgramsTess =
		ShaderPair{Util::createShaderProgram("modVertShader.glsl", "modFragShader.glsl",
		                                     "modTcsShader.glsl",  "modTesShader.glsl"),
		           Util::createShaderProgram("shadVertShader.glsl", "shadFragShader.glsl"),
		           true};
	renderingPrograms =
		ShaderPair{Util::createShaderProgram("modVertShaderNT.glsl", "modFragShader.glsl"),
		           Util::createShaderProgram("shadVertShader.glsl", "shadFragShader.glsl")};
	if(!renderingPrograms) printf("Could not Load shader\n");
	glfwSetKeyCallback(window, key_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	wld.init();

	//setup camera
	c.setPos(initialCameraLOC);
	c.pan(glm::radians(initialPan));
	c.pitch(glm::radians(initialPitch));

	setupScene();

	Util::printGLInfo();
	printHelp();
//	Util::checkOpenGLError();
}

void display(GLFWwindow* window, double currentTime)
{
	double timeDiff=currentTime-lastTime;
	//handle keyboard input
	handleKeys(window, timeDiff);
	handleUserLight();
	//update the objects if not paused
	if(!paused)
		wld.update(timeDiff);
	lastTime = currentTime;

	//build perspective and view matrices
	glm::mat4 pMat, vMat, invvMat;
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float aspect = (float)width/(float)height;
	vMat = c.getTransform();

	//clear buffers
	glClear(GL_DEPTH_BUFFER_BIT);
	glClearColor(0,0,0,1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	//relight computes the actual positions of all the lights(not just relative positions)
	wld.relight();
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(5.0f,5.0f);
	wld.buildShadowBuffers(viewMap, viewFace);
	glDisable(GL_POLYGON_OFFSET_FILL);


	//restore program to state for rendering after running shadow programs
	if(viewMap==-1)glBindFramebuffer(GL_FRAMEBUFFER,0);
	glDrawBuffer(viewMap==-1?GL_FRONT:GL_NONE);
	if(viewMap==-1)glViewport(0,0,width,height);


	//render one or both eyes i==0 -> left eye, i==1 -> right eye
	for(int i=0;i<1+(anaMode!=AnaglyphMode::OFF); ++i)
	{
		//re-clear the depth buffer
		glClear(GL_DEPTH_BUFFER_BIT);
		//set eye specific uniforms/settings
		if(anaMode==AnaglyphMode::OFF||viewMap!=-1)
		{
			pMat = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 1000.0f);
			vMat = c.getTransform();
			invvMat = glm::transpose(glm::inverse(vMat));
			glColorMask(true, true, true, true);
		}
		else
		{
			float lr = i*2-1;
			pMat = compPerspective(glm::radians(60.0f), aspect, 0.5, 150,lr,IOD);
			vMat = c.getTransform(lr*IOD/2);
			invvMat = glm::transpose(glm::inverse(vMat));
			if(anaMode==AnaglyphMode::RED_CYAN)
			{
				if(i==0) //left eye
					glColorMask(true,  false, false, false);
				else     //right eye
					glColorMask(false, true,  true,  false);
			}
			else if(anaMode==AnaglyphMode::GREEN_PURPLE)
			{
				if(i==0) //left eye
					glColorMask(true,  false, true,  false);
				else     //right eye
					glColorMask(false, true,  false, false);
			}
		}
		//draw the skybox
		sbox.draw(pMat, vMat, c.getPos());

		//send the uniforms to the GPU
		glUseProgram(renderingPrograms.renderProgram);
		GLuint invvHandleR = glGetUniformLocation(renderingPrograms.renderProgram, "invv_matrix");
		glUniformMatrix4fv(invvHandleR, 1, GL_FALSE, glm::value_ptr(invvMat));
		GLuint projHandleR = glGetUniformLocation(renderingPrograms.renderProgram, "proj_matrix");
		glUniformMatrix4fv(projHandleR, 1, GL_FALSE, glm::value_ptr(pMat));
		wld.glTransferLights(vMat, renderingPrograms.renderProgram, "lights");
		//send the uniforms to the GPU
		glUseProgram(renderingProgramsTess.renderProgram);
		GLuint invvHandleRT=glGetUniformLocation(renderingProgramsTess.renderProgram, "invv_matrix");
		glUniformMatrix4fv(invvHandleRT, 1, GL_FALSE, glm::value_ptr(invvMat));
		GLuint projHandleRT=glGetUniformLocation(renderingProgramsTess.renderProgram, "proj_matrix");
		glUniformMatrix4fv(projHandleRT, 1, GL_FALSE, glm::value_ptr(pMat));
		wld.glTransferLights(vMat, renderingProgramsTess.renderProgram, "lights");

		//invoke draw
		wld.draw(vMat);
	}

	if(axesEnabled)
	{
		//draw axes
		LineDrawer::draw(pMat, vMat, glm::vec3(0,0,0), glm::vec3(9,0,0), glm::vec3(1,0,0));
		LineDrawer::draw(pMat, vMat, glm::vec3(0,0,0), glm::vec3(0,9,0), glm::vec3(0,1,0));
		LineDrawer::draw(pMat, vMat, glm::vec3(0,0,0), glm::vec3(0,0,9), glm::vec3(0,0,1));
		wld.drawLightVecs(pMat, vMat);
	}
//	Util::checkOpenGLError();
}

void setupScene(void)
{
	//build skybox
	sbox=Skybox("img/skybox");

	//model data
//	GLuint sunTexture      = Util::loadTexture("img/sun_euv.png");
//	GLuint marsTexture     = Util::loadTexture("img/mars.jpg");
//	GLuint europaTexture   = Util::loadTexture("img/europa.jpg");
	GLuint ringWldTexture  = Util::loadTexture("img/rw_test.png");
	GLuint ringWldDepthMap = Util::loadTexture("img/rw_depth.png");
//	GLuint earthTexture    = Util::loadTexture("img/earth.jpg");
	GLuint shuttleTexture  = Util::loadTexture("img/spstob_1.jpg");

	//habitat internal region is height=habwidth, width=2*pi*(1-wallThick)
	//texture aspect ratio = 62.517777
	Model sunm  = Generator::generateSphere();
	Model rh   = Generator::generateRingHab(0.1,0.005,0.015,500);
	Model shu  = ModelImporter::parseOBJ("models/shuttle.obj");
	Model s2   = Generator::generateSphere();
	Model spot  = Generator::generateSphere();

	Material canvas = Material::getCanvas();
	Material silver = Material::getSilver();
	Material   gold = Material::getGold();

	userlight ={.ambient=glm::vec4{0.05,0.05,0.05,1}, .diffuse=glm::vec4{0.7,0.7,0.7,1},
	            .specular=glm::vec4{0.8,0.8,0.8,1},.direction=initialLightDIR,
	            .cutoff=glm::radians(35.0f),.exponent=30,
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
	iLight =
		Object::makeAbsolute(&wld, sunm, renderingPrograms, 0, gold, 1,
		                     initialLightLOC,
		                     glm::vec3{0.1f,1,0}, 15, 0.6f);
	Object* ringHab =
		Object::makeAbsolute(&wld, rh, renderingProgramsTess, ringWldTexture, canvas, 100,
		                     glm::vec3{0,0,0},
		                     glm::vec3{0,1,0}, 60, 0);
	Object* shuttle =
		Object::makeAbsolute(&wld, shu, renderingPrograms, shuttleTexture, canvas, 2,
		                     glm::vec3{-65.99,5.63,-60.51},
		                     glm::vec3{-1,0,0   }, 10, 0.25);

	//add maps
	ringHab->attachDepthMap(ringWldDepthMap);

	iLight->attachLight(userlight);
	slxp->attachLight(sllxp);
	slzm->attachLight(sllzm);
	slxm->attachLight(sllxm);
	slzp->attachLight(sllzp);

	wld.directional={.ambient=glm::vec4{0,0,0,1}, .diffuse=glm::vec4{0.7,0.7,0.7,1},
	                 .specular=glm::vec4{1,1,1,1},.direction=glm::vec3{-0.96,-0.23,-0.14},
	                 .enabled=true,.type=LightType::DIRECTIONAL};
	wld.ambient={.ambient=glm::vec4{0.2,0.2,0.2,1},.enabled=true,.type=LightType::AMBIENT};

	ringHab->addChild(*shuttle);
}

glm::mat4
	compPerspective(float fov, float aspect, float near, float far, float IOD, float lr)
{
	float top, bottom, left, right, shift;
	top = glm::tan(fov/2.0f) * near;
	bottom = -top;
	shift = (IOD/2.0f) * (near/far);
	left = -aspect*top - shift*lr;
	right = aspect*top - shift*lr;
	return glm::frustum(left, right, bottom, top, near, far);
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

void handleUserLight()
{
	if(grabbedLight)
	{
		userlight.direction=-c.getDir();
		iLight->attachLight(userlight);
		iLight->overrideAbsPos(c.getPos()+glm::normalize(userlight.direction));
	}
}

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
		c.setPos(initialCameraLOC);
		c.pan(glm::radians(-50.2f));
		c.pitch(glm::radians(-25.0f));
		printf("Pressed key   R: Camera reset\n");
	}
	else if(key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		paused = !paused;
		printf("Pressed key   P: Time %s\n",(paused?"Paused":"Unpaused"));
	}
	else if(key == GLFW_KEY_T && action == GLFW_PRESS)
	{
		userlight.enabled= !userlight.enabled;
		iLight->attachLight(userlight);
		printf("Pressed key   T: Light toggled %s\n",(userlight.enabled?"On":"Off"));
	}
	else if(key == GLFW_KEY_G && action == GLFW_PRESS)
	{
		anaMode=(AnaglyphMode)(anaMode+1);
		if(anaMode>2)
			anaMode=(AnaglyphMode)(anaMode-3);
		printf("Pressed key   G: Toggles Anaglyph mode to %s\n",modeToStr[anaMode]);
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
		int newMap = (key-GLFW_KEY_KP_0);
		printf("Pressed key  ^%d: Viewing shadow map for light %d\n", newMap, newMap);
		viewMap=(GLint)newMap;
		viewFace=0;
	}
	else if(key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_5 &&
	        (mods & GLFW_MOD_ALT) && action == GLFW_PRESS)
	{
		int newFace = (key-GLFW_KEY_KP_0);
		printf("Pressed key M-%d: Viewing shadow map for light %d, face %d\n",
		       newFace, viewMap, newFace);
		viewFace=(GLint)newFace;
	}
}

void scroll_callback(GLFWwindow* window, double xScrOff, double yScrOff)
{
	//do nothing for now
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		grabbedLight=!grabbedLight;
		if(grabbedLight)
			printf("Grabbed Light\n");
		else
		{
			glm::vec3 dir = glm::normalize(c.getDir());
			glm::vec3 pos = c.getPos()+ dir;
			printf("Dropped Light at pos:(%.2f,%.2f,%.2f), dir:(%.2f,%.2f,%.2f)\n",
			       pos.x,pos.y,pos.z,dir.x,dir.y,dir.z);
		}
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void DebugCB(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,
             const GLchar* message, const GLvoid* userParam)
{
	printf("%s\n",message);
}

void printHelp(void)
{
	printf(
		"Usage:\n"
		"Note: M- means ALT\n"
		"Press any of the following keys while the window is selected\n"
		"  ?:  View this help message in console(F1 also works)\n"
		" ^Q:  Quit program(^W also works)\n"
		"  R:  Reset camera to original position ((0,0,30), looking a the origin)\n"
		"  P:  Toggle Pause time(default: off)\n"
		"  T:  Toggle the user light on/off(default: on)\n"
		"  G:  Toggle anaglyph mode(off/red-cyan/purple-green)(default: off)\n"
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
		"M-#:  Where # is a number 0-5 inclusive on the keypad.  Selects face # for positionals\n"
	);
}
