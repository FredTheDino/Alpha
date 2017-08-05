#include <unistd.h>

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
		exit(1);
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
	}
	
	// GL default settings.
	printf("[OpenGL] Version: %s\n", glGetString(GL_VERSION));
	printf("[OpenGL] Vendor: %s\n", glGetString(GL_VENDOR));

	// We're doing 2D, so this should almost always be off.
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glClearColor(0.75, 0.3, 0.21, 1.0);

	while (!_g.should_quit) {
		glfwPollEvents();
		if (glfwGetKey(_g.window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			_g.should_quit = true;
		}
		glfwSwapBuffers(_g.window);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	}
}
