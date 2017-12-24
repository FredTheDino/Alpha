// The external libraries we will be needing.
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <AL/al.h>
#include <AL/alc.h>

// STL stuff.
#include <typeinfo>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <float.h>
// Doesn't work on windows.
// #include <unistd.h>
#include <assert.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

// I am bound by std...
#include <string>
#include <vector>
#include <unordered_map>
#include <stack>

typedef std::string String;
#define Stack std::stack
#define Array std::vector
#define HashMap std::unordered_map

// STB image.
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// My stuff.
#include "globals.h"
#include "math.h"
#include "physics.h"
#include "graphics.h"
#include "entity.h"

Mesh quad_mesh;

// Shaders.
Shader color_shader;
Shader post_process_shader;

// My very own containers
//#include "containers.cpp"
#include "physics_helper.cpp"
#include "physics.cpp"
#include "graphics.cpp"
#include "input.cpp"
#include "audio.cpp"
#include "entity.cpp"
#include "entity_type.cpp"
#include "level.h"
#include "level.cpp"

// Stuff with dependencies.
#include "hotloader.cpp"

#define POST_PROCESSING 1

struct PPO {
	GLuint buffer;
	Texture texture;
	GLuint depth;
} ppo;

void window_close_callback(GLFWwindow* window) {
	global.should_quit = true;
}

void window_resize_callback(GLFWwindow* window, int new_width, int new_height) {
	set_window_info(new_width, new_height, global.msaa);

	glViewport(0, 0, new_width, new_height);

#if POST_PROCESSING
	glBindFramebuffer(GL_FRAMEBUFFER, ppo.buffer);

	const int w = global.sample_width;
	const int h = global.sample_height;
	glViewport(0, 0, w, h);
	glBindTexture(GL_TEXTURE_2D, ppo.texture.texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, ppo.depth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)ppo.buffer);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ppo.texture.texture_id, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ppo.depth, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
}

// Setup the offscreen frame buffer.

void init_ppo() {

	const int w = global.sample_width;
	const int h = global.sample_height;
	glGenTextures(1, &ppo.texture.texture_id);
	glBindTexture(GL_TEXTURE_2D, ppo.texture.texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

/*
	// OpenGL 3?
	glGenRenderbuffers(1, &ppo.depth);
	glBindRenderbuffer(GL_RENDERBUFFER, ppo.depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
*/

	glGenTextures(1, &ppo.depth);
	glBindTexture(GL_TEXTURE_2D, ppo.depth);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &ppo.buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)ppo.buffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ppo.texture.texture_id, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ppo.depth, 0);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		printf("Failed to create frame buffer\n");
}

inline void init_engine() {
	if (!glfwInit()) {
		printf("[Init] Failed to initalize GLFW - Window and IO lib.");
		assert(0);
	}

	// Set the resolution.
	set_window_info(800, 600);

	global.window = glfwCreateWindow(global.window_width, global.window_height, global.window_title, NULL, NULL);

	glfwSetWindowCloseCallback(global.window, window_close_callback);
	glfwSetWindowSizeCallback(global.window, window_resize_callback);
	glfwMakeContextCurrent(global.window);
	glfwSetInputMode(global.window, GLFW_STICKY_KEYS, 1);

	// @FIXME, we dont allow this to be set ATM, that would probably be smart.
	glfwSwapInterval(1);

	if(glewInit()) {
		printf("[Init] Failed to intalize GLEW - Extension Wrangler\n");
		assert(0);
	}

	/////////////
	// INIT GL //
	/////////////
	printf("[OpenGL] Version: %s\n", glGetString(GL_VERSION));
	printf("[OpenGL] Vendor: %s\n", glGetString(GL_VENDOR));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.3, 0.8, 0.4, 1.0);

#if POST_PROCESSING
	init_ppo();
#endif
	init_audio();
	window_resize_callback(global.window, global.window_width, global.window_height);

	register_hotloadable_asset(hot_loader, &input_map, "res/input.map");

	// Init the graphics
	Array<Vertex> verticies;
	verticies.reserve(6);

	verticies.push_back(Vertex(-0.5, -0.5, 0, 1));
	verticies.push_back(Vertex( 0.5, -0.5, 1, 1));
	verticies.push_back(Vertex( 0.5,  0.5, 1, 0));

	verticies.push_back(Vertex( 0.5,  0.5, 1, 0));
	verticies.push_back(Vertex(-0.5,  0.5, 0, 0));
	verticies.push_back(Vertex(-0.5, -0.5, 0, 1));

	quad_mesh = new_mesh(verticies);

	color_shader = Shader("res/2d_color.glsl", "2d_color");
	post_process_shader = Shader("res/post_process.glsl", "post");
}

void set_aa(int msaa) {
	if (msaa < 1) msaa = 1;
	global.msaa = msaa;
	window_resize_callback(global.window, global.window_width, global.window_height);
}

void game_main() {
	init_engine();

	// Disables text buffering...
	setbuf(stdout, NULL);

	Level level = load_level(entity_list, engine, "res/level");

	// Load stuff
	Sound ha("res/a.wav");

	// Load font test!
	auto droid_sans = load_font_from_files("res/fonts/droid_sans");
	auto text_mesh = new_text_mesh(droid_sans, "Hello World");

	main_camera.zoom = 4;
	add_entity(entity_list, new_camera_controller());

	float t = 0.0f;
	float buffer = 0;
	float delta = 1.0f / 420.0f;
	int frame = 0;
	glfwSetTime(t);

	// It's clearing the entity list right after loading... I think...
	// It doesn't do that on Linux... it's super wierd...

	while (!global.should_quit) {
		float new_t = glfwGetTime();
		buffer += new_t - t;
		t = new_t;

		while (delta < buffer) {
			buffer -= delta;
			frame++;
			glfwPollEvents();

			update_input();

			update_loader(hot_loader);

			update_physics_engine(engine, delta);

			update_audio();

			// Begining of update.
			update_entities(entity_list, delta);

			gc_entities(entity_list); // This doesn't need to be run everyframe.

			if (is_down("exit")) {
				global.should_quit = true;
			}
			if (pressed("reload")) {
				reload_level(level);
				add_entity(entity_list, new_camera_controller());
			}
		}

#if POST_PROCESSING
		glBindFramebuffer(GL_FRAMEBUFFER, ppo.buffer);
#endif
		{
			// Setup graphics
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
			use_shader(color_shader);
			send_camera_to_shader(color_shader);

			debug_draw_engine(color_shader, engine);
			draw_entities(entity_list, frame * delta);
		}

		// Post processing.
#if POST_PROCESSING
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			use_shader(post_process_shader);
			GLuint screen_location = glGetUniformLocation(post_process_shader.program, "screen");
			bind_texture(ppo.texture, 0);
			glUniform1i(screen_location, 0);

			glUniform1f(glGetUniformLocation(post_process_shader.program, "time"), frame * delta);
			glUniform2f(glGetUniformLocation(post_process_shader.program, "sample_size"),
					1.0f / global.sample_width, 1.0f / global.sample_height);
			glUniform1i(glGetUniformLocation(post_process_shader.program, "msaa"), global.msaa);

			draw_mesh(quad_mesh);
		}
#endif

		glfwSwapBuffers(global.window);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	}

	delete_shader(color_shader);
#if POST_PROCESSING
	delete_shader(post_process_shader);
#endif

	destroy_audio();

	clear_level(level);
}
