// The external libraries we will be needing.
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

// STL stuff.
#include <typeinfo>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>

// My very own containers
#include "containers.cpp"
#include "graphics.cpp"

// My stuff.
#include "globals.h"

Mesh quad_mesh;

void window_close_callback(GLFWwindow* window) {
	_g.should_quit = true;
}

void window_resize_callback(GLFWwindow* window, int new_width, int new_height) {
	_g.window_width = new_width;
	_g.window_height = new_height;
	_g.window_aspect_ratio = (float) new_width / new_height;

	glViewport(0, 0, new_width, new_height);
}

void game_main() {
	// Initalize window.
	if (!glfwInit()) {
		printf("[Init] Failed to initalize GLFW - Window and IO lib.");
		assert(0);
	}

	_g.window = glfwCreateWindow(_g.window_width, _g.window_height, _g.window_title, NULL, NULL);
	_g.window_aspect_ratio = _g.window_width / _g.window_height;

	glfwSetWindowCloseCallback(_g.window, window_close_callback);
	glfwSetWindowSizeCallback(_g.window, window_resize_callback);

	glfwMakeContextCurrent(_g.window);	

	// @FIXME, we dont allow this to be set ATM, that would probably be smart.
	glfwSwapInterval(1);

	if(glewInit()) {
		printf("[Init] Failed to intalize GLEW - Extension Wrangler\n");
		assert(0);
	}
	
	// GL default settings.
	printf("[OpenGL] Version: %s\n", glGetString(GL_VERSION));
	printf("[OpenGL] Vendor: %s\n", glGetString(GL_VENDOR));

	// We're doing 2D, so this should almost always be off.
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glClearColor(0.75, 0.3, 0.21, 1.0);

	Array<Vertex> verticies;
	reserve(verticies, 6);

	push_back(verticies, Vertex(-0.5, -0.5, 0, 0));
	push_back(verticies, Vertex( 0.5, -0.5, 1, 0));
	push_back(verticies, Vertex( 0.5,  0.5, 1, 1));

	push_back(verticies, Vertex( 0.5,  0.5, 1, 1));
	push_back(verticies, Vertex(-0.5,  0.5, 0, 1));
	push_back(verticies, Vertex(-0.5, -0.5, 0, 0));
	quad_mesh = new_mesh(verticies);

	float t = 0.0f;
	while (!_g.should_quit) {
		t += 0.01f;
		glfwPollEvents();
		if (glfwGetKey(_g.window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			_g.should_quit = true;
		}

		draw_mesh(quad_mesh);

		glBegin(GL_TRIANGLES);
		{
			glColor3f(0.0f, 1.0f, 1.0f);
			glVertex2f(0, sin(t));
			glVertex2f(1, 1);
			glVertex2f(1, -1);
		}
		glEnd();

		glfwSwapBuffers(_g.window);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	}
}
