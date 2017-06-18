//---------------------------------------------------------------------------
//
// Copyright (c) 2016 Taehyun Rhee, Joshua Scott, Ben Allen
//
// This software is provided 'as-is' for assignment of COMP308 in ECS,
// Victoria University of Wellington, without any express or implied warranty. 
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// The contents of this file may not be copied or duplicated in any form
// without the prior permission of its owner.
//
//----------------------------------------------------------------------------

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <stdexcept>

#include "cgra_geometry.hpp"
#include "cgra_math.hpp"
#include "simple_image.hpp"
#include "simple_shader.hpp"
#include "opengl.hpp"
#include "geometry.hpp"
#include "water.hpp"

using namespace std;
using namespace cgra;

// Window
//
GLFWwindow* g_window;


// Projection values
// 
float g_fovy = 20.0;
float g_znear = 0.01;
float g_zfar = 1000.0;


// Mouse controlled Camera values
//
bool g_leftMouseDown = false;
vec2 g_mousePosition;
float g_pitch = 0;
float g_yaw = 0;
float g_zoom = 1.0;

// Values and fields to showcase the use of shaders
// Remove when modifying main.cpp for Assignment 3
//
bool g_useShader = false;

GLuint g_shader = 0;


// Rendered objects
Geometry *g_table;
GLuint g_table_tex;

Geometry *g_sphere;
GLuint g_sphere_tex;

Geometry *g_torus;
GLuint g_torus_tex;

Geometry *g_cube;
GLuint g_cube_tex;

Geometry *g_teapot;
GLuint g_teapot_tex;

Geometry *g_bunny;
GLuint g_bunny_tex;

Water *g_water;

// Mouse Button callback
// Called for mouse movement event on since the last glfwPollEvents
//
void cursorPosCallback(GLFWwindow* win, double xpos, double ypos) {
	// cout << "Mouse Movement Callback :: xpos=" << xpos << "ypos=" << ypos << endl;
	if (g_leftMouseDown) {
		g_yaw -= g_mousePosition.x - xpos;
		g_pitch -= g_mousePosition.y - ypos;
	}
	g_mousePosition = vec2(xpos, ypos);
}


// Mouse Button callback
// Called for mouse button event on since the last glfwPollEvents
//
void mouseButtonCallback(GLFWwindow *win, int button, int action, int mods) {
	// cout << "Mouse Button Callback :: button=" << button << "action=" << action << "mods=" << mods << endl;
	if (button == GLFW_MOUSE_BUTTON_LEFT)
		g_leftMouseDown = (action == GLFW_PRESS);
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		if (g_useShader) {
			g_useShader = false;
			cout << "Using the default OpenGL pipeline" << endl;
		}
		else {
			g_useShader = true;
			cout << "Using a shader" << endl;
		}
	}
}


// Scroll callback
// Called for scroll event on since the last glfwPollEvents
//
void scrollCallback(GLFWwindow *win, double xoffset, double yoffset) {
	// cout << "Scroll Callback :: xoffset=" << xoffset << "yoffset=" << yoffset << endl;
	g_zoom -= yoffset * g_zoom * 0.2;
}


// Keyboard callback
// Called for every key event on since the last glfwPollEvents
//
void keyCallback(GLFWwindow *win, int key, int scancode, int action, int mods) {
	// cout << "Key Callback :: key=" << key << "scancode=" << scancode
	// 	<< "action=" << action << "mods=" << mods << endl;
	// YOUR CODE GOES HERE
	// ...
}


// Character callback
// Called for every character input event on since the last glfwPollEvents
//
void charCallback(GLFWwindow *win, unsigned int c) {
	// cout << "Char Callback :: c=" << char(c) << endl;
	// Not needed for this assignment, but useful to have later on
}


// Sets up where and what the light is
// Called once on start up
// 
void initLight() {
	float direction[] = { 0.0f, 0.0f, 1.0f, 0.0f };
	float diffintensity[] = { 0.7f, 0.7f, 0.7f, 1.0f };
	float ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };

	glLightfv(GL_LIGHT0, GL_POSITION, direction);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffintensity);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);


	glEnable(GL_LIGHT0);
}


// An example of how to load a texure from a hardcoded location
//
void initTexture(const char *path, GLuint *binding) {
	Image tex(path);

	glActiveTexture(GL_TEXTURE0); // Use slot 0, need to use GL_TEXTURE1 ... etc if using more than one texture PER OBJECT
	glGenTextures(1, binding); // Generate texture ID
	glBindTexture(GL_TEXTURE_2D, *binding); // Bind it as a 2D texture

	// Setup sampling strategies
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Finnaly, actually fill the data into our texture
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, tex.w, tex.h, tex.glFormat(), GL_UNSIGNED_BYTE, tex.dataPointer());
}


void initModels() {
    initTexture("./res/textures/wood.jpg", &g_table_tex);
    g_table = new Geometry("./res/assets/table.obj");
    //g_table->m_position = vec3(-5, 0, -5);

    g_sphere = new Geometry("./res/assets/sphere.obj");
    g_sphere->m_position = vec3(5, 2, 5);

    g_torus = new Geometry("./res/assets/torus.obj");
    g_torus->m_position = vec3(-5, 1, 5);

    initTexture("./res/textures/brick.jpg", &g_cube_tex);
    g_cube = new Geometry("./res/assets/box.obj");
    g_cube->m_position = vec3(-5, 2.5, -5);

    g_teapot = new Geometry("./res/assets/teapot.obj");
    g_teapot->m_position = vec3(5, 0.25, -5);

    g_bunny = new Geometry("./res/assets/bunny.obj");
    g_bunny->m_position = vec3(0, 0.35, 0);
}


// An example of how to load a shader from a hardcoded location
//
void initShader() {
	// To create a shader program we use a helper function
	// We pass it an array of the types of shaders we want to compile
	// and the corrosponding locations for the files of each stage
	g_shader = makeShaderProgramFromFile({GL_VERTEX_SHADER, GL_FRAGMENT_SHADER }, { "./res/shaders/shaderDemo.vert", "./res/shaders/shaderDemo.frag" });
}


void setupLight() {
    // No transform for the light
    // makes it move realitive to camera
    glMatrixMode(GL_PROJECTION);
    //glLoadIdentity();

    // DIRECTIONAL LIGHT

    vec4 position(0, 2, -1, 0);
    vec4 diffuse(0.3, 0.3, 0.3, 0.0);
    vec4 ambient(0.3, 0.3, 0.3, 1.0);
    vec4 specular(0, 0, 0, 1);

    glLightfv(GL_LIGHT0, GL_POSITION, position.dataPointer());
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  diffuse.dataPointer());
    glLightfv(GL_LIGHT0, GL_AMBIENT,  ambient.dataPointer());
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular.dataPointer());

    glEnable(GL_LIGHT0);

    // POINT LIGHT



    position = vec4(7, 6, 4, 1);
    diffuse = vec4(0.5, 0.4, 0.6, 1);
    ambient = vec4(0.2, 0.1, 0.1, 1);
    specular = vec4(0.5, 0.4, 0.6, 1);

    glLightfv(GL_LIGHT1, GL_POSITION, position.dataPointer());
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  diffuse.dataPointer());
    glLightfv(GL_LIGHT1, GL_SPECULAR, specular.dataPointer());
    glLightfv(GL_LIGHT1, GL_AMBIENT, ambient.dataPointer());
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.7);
    // ambient = 0;

    glEnable(GL_LIGHT1);

    // SPOTLIGHT

    position = vec4(-8, 9, 2, 1);
    diffuse = vec4(0.8, 0.8, 0.2, 1);
    ambient = vec4(0.3, 0.3, 0.1, 1);
    specular = vec4(1, 1, 0, 0.5);

    //vec4 direction(-0.8f, -0.7f, 0.5, 1);
    vec4 direction(8, -10, -2, 1);

    glLightfv(GL_LIGHT2, GL_POSITION, position.dataPointer());
    glLightfv(GL_LIGHT2, GL_DIFFUSE,  diffuse.dataPointer());
    glLightfv(GL_LIGHT2, GL_AMBIENT, ambient.dataPointer());
    glLightfv(GL_LIGHT2, GL_SPECULAR, specular.dataPointer());

    glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, direction.dataPointer());
    //glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 0.5);
    glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 20);
    glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 0.5);

    glEnable(GL_LIGHT2);

}


// Sets up where the camera is in the scene
// 
void setupCamera(int width, int height) {
	// Set up the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(g_fovy, width / float(height), g_znear, g_zfar);

	// Set up the view part of the model view matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0, 0, -50 * g_zoom);
	glRotatef(g_pitch, 1, 0, 0);
	glRotatef(g_yaw, 0, 1, 0);
}


void renderGeometry();

// Draw function
//
void render(int width, int height) {

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// Enable flags for normal rendering
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
    glEnable(GL_TEXTURE_2D);

    setupLight();

	setupCamera(width, height);

    // Enable Drawing texures
    glEnable(GL_TEXTURE_2D);
    // Use Texture as the color
    if (!g_useShader) glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    // Set the location for binding the texture
    glActiveTexture(GL_TEXTURE0);
    // Bind the texture
    glBindTexture(GL_TEXTURE_2D, g_cube_tex);

    glActiveTexture(GL_TEXTURE0);
    // Bind the texture
    glBindTexture(GL_TEXTURE_2D, g_cube_tex);

    if (g_useShader) {
        // Use the shader we made
        glUseProgram(g_shader);

        // Set our sampler (texture0) to use GL_TEXTURE0 as the source
        glUniform1i(glGetUniformLocation(g_shader, "texture0"), 0);
    }

    renderGeometry();
    g_water->render();

    // Disable flags for cleanup (optional)
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_NORMALIZE);
}

void renderGeometry() {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shininess;

    // matte
    ambient = vec4(0.1, 0.1, 0.1, 1);
    diffuse = vec4(0.5, 0.5, 0.5, 1);
    specular = vec4(0.05, 0.05, 0.05, 1);
    shininess = 5;

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient.dataPointer());
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse.dataPointer());
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular.dataPointer());
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess*128);

    g_cube->renderGeometry();

    // wood
    glBindTexture(GL_TEXTURE_2D, g_table_tex);

    // varnish
    ambient = vec4(0.5, 0.5, 0.5, 5);
    diffuse = vec4(0.3, 0.3, 0.3, 1);
    specular = vec4(0.5, 0.5, 0.5, 1);
    shininess = 40;

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient.dataPointer());
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse.dataPointer());
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular.dataPointer());
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess*128);

    g_table->renderGeometry();

    glDisable(GL_TEXTURE_2D);

    // white china
    ambient = vec4(0.2, 0.2, 0.2, 1);
    diffuse = vec4(0.5, 0.5, 0.5, 1);
    specular = vec4(0.3, 0.3, 0.3, 1);
    shininess = 30;

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient.dataPointer());
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse.dataPointer());
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular.dataPointer());
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess*128);

    // bunny
    g_bunny->renderGeometry();

    // bronze
    ambient = vec4(0.2, 0.1, 0.05, 1);
    diffuse = vec4(0.7, 0.4, 0.2, 1);
    specular = vec4(0.4, 0.3, 0.2, 1);
    shininess = 30;

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient.dataPointer());
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse.dataPointer());
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular.dataPointer());
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess*128);

    // sphere
    g_sphere->renderGeometry();

    // bluish metal
    ambient = vec4(0.05, 0.05, 0.05, 1);
    diffuse = vec4(0.2, 0.25, 0.3, 1);
    specular = vec4(0.1, 0.3, 0.4, 1);
    shininess = 15;

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient.dataPointer());
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse.dataPointer());
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular.dataPointer());
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

    // teapot
    g_teapot->renderGeometry();

    // red plastic
    ambient = vec4(0, 0, 0, 1);
    diffuse = vec4(0.5, 0, 0, 1);
    specular = vec4(0.7, 0.5, 0.5, 1);
    shininess = 40;

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient.dataPointer());
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse.dataPointer());
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular.dataPointer());
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess*128);

    // torus
    g_torus->renderGeometry();

    if (g_useShader) {
        // Unbind our shader
        glUseProgram(0);
    }
}


// Forward decleration for cleanliness (Ignore)
void APIENTRY debugCallbackARB(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar*, GLvoid*);


void initWater();

//Main program
// 
int main(int argc, char **argv) {

	// Initialize the GLFW library
	if (!glfwInit()) {
		cerr << "Error: Could not initialize GLFW" << endl;
		abort(); // Unrecoverable error
	}

	// Get the version for GLFW for later
	int glfwMajor, glfwMinor, glfwRevision;
	glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRevision);

	// Create a windowed mode window and its OpenGL context
	g_window = glfwCreateWindow(640, 480, "Hello World", nullptr, nullptr);
	if (!g_window) {
		cerr << "Error: Could not create GLFW window" << endl;
		abort(); // Unrecoverable error
	}

	// Make the g_window's context is current.
	// If we have multiple windows we will need to switch contexts
	glfwMakeContextCurrent(g_window);



	// Initialize GLEW
	// must be done after making a GL context current (glfwMakeContextCurrent in this case)
	glewExperimental = GL_TRUE; // required for full GLEW functionality for OpenGL 3.0+
	GLenum err = glewInit();
	if (GLEW_OK != err) { // Problem: glewInit failed, something is seriously wrong.
		cerr << "Error: " << glewGetErrorString(err) << endl;
		abort(); // Unrecoverable error
	}



	// Print out our OpenGL verisions
	cout << "Using OpenGL " << glGetString(GL_VERSION) << endl;
	cout << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;
	cout << "Using GLFW " << glfwMajor << "." << glfwMinor << "." << glfwRevision << endl;



	// Attach input callbacks to g_window
	glfwSetCursorPosCallback(g_window, cursorPosCallback);
	glfwSetMouseButtonCallback(g_window, mouseButtonCallback);
	glfwSetScrollCallback(g_window, scrollCallback);
	glfwSetKeyCallback(g_window, keyCallback);
	glfwSetCharCallback(g_window, charCallback);



	// Enable GL_ARB_debug_output if available. Not nessesary, just helpful
	if (glfwExtensionSupported("GL_ARB_debug_output")) {
		// This allows the error location to be determined from a stacktrace
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		// Set the up callback
		glDebugMessageCallbackARB(debugCallbackARB, nullptr);
		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
		cout << "GL_ARB_debug_output callback installed" << endl;
	}
	else {
		cout << "GL_ARB_debug_output not available. No worries." << endl;
	}


	// Initialize Geometry/Material/Lights
	// YOUR CODE GOES HERE
	// ...
	initLight();
    initModels();
    initWater();
	initShader();


	// Loop until the user closes the window
	while (!glfwWindowShouldClose(g_window)) {

		// Make sure we draw to the WHOLE window
		int width, height;
		glfwGetFramebufferSize(g_window, &width, &height);

		// Main Render
		render(width, height);

		// Swap front and back buffers
		glfwSwapBuffers(g_window);

		// Poll for and process events
		glfwPollEvents();
	}

	glfwTerminate();
}

void initWater() {
    g_water = new Water(vec2(-50, -50), vec2(50, 50), 2.5);
}






//-------------------------------------------------------------
// Fancy debug stuff
//-------------------------------------------------------------

// function to translate source to string
string getStringForSource(GLenum source) {

	switch (source) {
	case GL_DEBUG_SOURCE_API:
		return("API");
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		return("Window System");
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		return("Shader Compiler");
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		return("Third Party");
	case GL_DEBUG_SOURCE_APPLICATION:
		return("Application");
	case GL_DEBUG_SOURCE_OTHER:
		return("Other");
	default:
		return("n/a");
	}
}

// function to translate severity to string
string getStringForSeverity(GLenum severity) {

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:
		return("HIGH!");
	case GL_DEBUG_SEVERITY_MEDIUM:
		return("Medium");
	case GL_DEBUG_SEVERITY_LOW:
		return("Low");
	default:
		return("n/a");
	}
}

// function to translate type to string
string getStringForType(GLenum type) {
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		return("Error");
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		return("Deprecated Behaviour");
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		return("Undefined Behaviour");
	case GL_DEBUG_TYPE_PORTABILITY:
		return("Portability Issue");
	case GL_DEBUG_TYPE_PERFORMANCE:
		return("Performance Issue");
	case GL_DEBUG_TYPE_OTHER:
		return("Other");
	default:
		return("n/a");
	}
}

// actually define the function
void APIENTRY debugCallbackARB(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei, const GLchar* message, GLvoid*) {
	if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) return;

	cerr << endl; // extra space

	cerr << "Type: " <<
		getStringForType(type) << "; Source: " <<
		getStringForSource(source) << "; ID: " << id << "; Severity: " <<
		getStringForSeverity(severity) << endl;

	cerr << message << endl;

	if (type == GL_DEBUG_TYPE_ERROR_ARB) throw runtime_error("");
}