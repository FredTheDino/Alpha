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
#include <sys/stat.h>

// I am bound by std...
#include <string>
#include <vector>

typedef std::string String;
#define Array std::vector

// STB image.
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// My very own containers
//#include "containers.cpp"
#include "graphics.cpp"

// My stuff.
#include "globals.h"

// Stuff with dependencies.
#include "hotloader.cpp"

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

	/////////////
	// INIT GL //
	/////////////
	
	// GL default settings.
	printf("[OpenGL] Version: %s\n", glGetString(GL_VERSION));
	printf("[OpenGL] Vendor: %s\n", glGetString(GL_VENDOR));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Setup the offscreen frame buffer.
	GLuint post_processing_buffer;
	glGenFramebuffers(1, &post_processing_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, post_processing_buffer);

	GLuint post_processing_texture;
	glGenTextures(1, &post_processing_texture);
	glBindTexture(GL_TEXTURE_2D, post_processing_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _g.window_width, _g.window_height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	GLuint post_processing_depth;
	glGenRenderbuffers(1, &post_processing_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, post_processing_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _g.window_width, _g.window_height);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, post_processing_texture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, post_processing_depth);

	glClearColor(0.75, 0.3, 0.21, 1.0);

	Array<Vertex> verticies;
	verticies.reserve(6);

	verticies.push_back(Vertex(-0.5, -0.5, 0, 1));
	verticies.push_back(Vertex( 0.5, -0.5, 1, 1));
	verticies.push_back(Vertex( 0.5,  0.5, 1, 0));

	verticies.push_back(Vertex( 0.5,  0.5, 1, 0));
	verticies.push_back(Vertex(-0.5,  0.5, 0, 0));
	verticies.push_back(Vertex(-0.5, -0.5, 0, 1));

	quad_mesh = new_mesh(verticies);
	Shader color_shader;
	register_hotloadable_asset(hot_loader, &color_shader, "res/2d_color.glsl", "2d_color");
	Shader post_process_shader;
	register_hotloadable_asset(hot_loader, &post_process_shader, "res/post_process.glsl", "post");

	Texture mario;
	register_hotloadable_asset(hot_loader, &mario, "res/mario");

	float t = 0.0f;
	while (!_g.should_quit) {
		t += 0.05f;
		glfwPollEvents();
		if (glfwGetKey(_g.window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			_g.should_quit = true;
		}

		update_loader(hot_loader);

		glBindFramebuffer(GL_FRAMEBUFFER, post_processing_buffer);
		glViewport(0, 0, _g.window_width, _g.window_height);
		
		glClearColor(0.0, 0.8, 0.21, 1.0);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		use_shader(color_shader);
		bind_texture(mario, 0);
		GLint sprite = glGetUniformLocation(color_shader.program, "sprite");
		glUniform1i(sprite, 0);

		GLuint loc = glGetUniformLocation(color_shader.program, "layer");

		GLint x_loc = glGetUniformLocation(color_shader.program, "x");
		GLint y_loc = glGetUniformLocation(color_shader.program, "y");
		GLint color_scale_loc = glGetUniformLocation(color_shader.program, "color_scale");

		glUniform1f(x_loc, 0.2);
		glUniform1f(y_loc, -0.2);
		glUniform1f(color_scale_loc, 0.1);

		glUniform1f(loc, sin(t));
		draw_mesh(quad_mesh);

		glUniform1f(x_loc, -0.2);
		glUniform1f(y_loc, 0.2);
		glUniform1f(color_scale_loc, 1.0f);

		glUniform1f(loc, 0);
		draw_mesh(quad_mesh);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, _g.window_width, _g.window_height);

		GLuint screen_location = glGetUniformLocation(post_process_shader.program, "screen");
		use_shader(post_process_shader);
		glUniform1i(screen_location, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, post_processing_texture);

		glUniform1f(glGetUniformLocation(post_process_shader.program, "time"), t);

		//
		// SPOILER! The framebuffers aren't working, should poorbably fix that...
		//

		draw_mesh(quad_mesh);

		glfwSwapBuffers(_g.window);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glClearColor(0.9, 0.8, 0.21, 1.0);
	}

	delete_texture(mario);
	delete_shader(color_shader);
}
