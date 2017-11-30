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
#include "graphics.h"

Mesh quad_mesh;

// My very own containers
//#include "containers.cpp"
#include "graphics.cpp"
#include "input.cpp"
#include "audio.cpp"
#include "entity.cpp"

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
	set_window_info(new_width, new_height);

	glViewport(0, 0, new_width, new_height);

#if POST_PROCESSING
	glBindFramebuffer(GL_FRAMEBUFFER, ppo.buffer);
	glViewport(0, 0, new_width, new_height);

	const int w = new_width;
	const int h = new_height;
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

	const int w = global.window_width;
	const int h = global.window_height;
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

void game_main() {
	// Initalize window.
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

#if POST_PROCESSING
	init_ppo();
#endif

	glClearColor(0.9, 0.8, 0.21, 1.0);

	/////////////
	// INIT AL //
	/////////////
	init_audio();

	Sound ha;
	register_hotloadable_asset(hot_loader, &ha, "res/a.wav");

	// Load the input map!
	register_hotloadable_asset(hot_loader, &input_map, "res/input.map");

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
	entity_list.color_shader = &color_shader;
	register_hotloadable_asset(hot_loader, &color_shader, "res/2d_color.glsl", "2d_color");
	Shader post_process_shader;
	register_hotloadable_asset(hot_loader, &post_process_shader, "res/post_process.glsl", "post");

	// Maybe move the hotloader to the constructor? It's maybe more convinient...
	Texture mario;
	register_hotloadable_asset(hot_loader, &mario, "res/mario");
	mario.sprites_y = 1;
	mario.sprites_x = 2;
	
	///////////////
	// ENTITIES! //
	///////////////
	// I'm actually happy with this!
	auto a = create_entity(entity_list);
	auto b = create_entity(entity_list);

	add_component(entity_list, a, TRANSFORM_COMPONENT);
	add_component(entity_list, a, SPRITE_COMPONENT);
	add_system(entity_list, a, SIMPLE_SPRITE_SYSTEM);
	get_sprite(entity_list, a)->sprite = mario;

	add_component(entity_list, b, TRANSFORM_COMPONENT);
	add_component(entity_list, b, SPRITE_COMPONENT);
	add_system(entity_list, b, SIMPLE_SPRITE_SYSTEM);
	get_sprite(entity_list, b)->sprite = mario;
	get_sprite(entity_list, b)->sub_sprite = 1;



	float t = 0.0f;
	float delta = 0.0f;
	glfwSetTime(t);
	while (!global.should_quit) {
		float new_t = glfwGetTime();
		delta = t - new_t;
		t = new_t;

		glfwPollEvents();

		update_loader(hot_loader);

		update_input();

		update_audio();

		if (is_down("exit")) {
			global.should_quit = true;
		}

		if (pressed("sound")) {
			play_sound(ha, SFX);
		}

#if POST_PROCESSING
		glBindFramebuffer(GL_FRAMEBUFFER, ppo.buffer);
#endif
		{
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

			use_shader(color_shader);

			main_camera.position.x += delta * value("right");
			main_camera.position.x -= delta * value("left");
			main_camera.position.y += delta * value("up");
			main_camera.position.y -= delta * value("down");

			main_camera.rotation += delta * value("turn");

			main_camera.zoom /= 1 + delta * value("zoom_in");
			main_camera.zoom *= 1 + delta * value("zoom_out");

			send_camera_to_shader(color_shader);

			float x = sin(t);
			float c = cos(t * 0.2);
			auto& a_t = entity_list.transform_c[a.pos];
			a_t.position.x = x;
			a_t.position.y = 0;
			a_t.rotation = x;
			entity_list.sprite_c[a.pos].layer = x;
			entity_list.transform_c[b.pos].scale.y = c;
			update_systems(entity_list, delta);
		}
		
		// Post processing.
#if POST_PROCESSING
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		use_shader(post_process_shader);
		GLuint screen_location = glGetUniformLocation(post_process_shader.program, "screen");
		bind_texture(ppo.texture, 0);
		glUniform1i(screen_location, 0);

		glUniform1f(glGetUniformLocation(post_process_shader.program, "time"), t);

		draw_mesh(quad_mesh);
#endif

		glfwSwapBuffers(global.window);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	}

	delete_texture(mario);
	delete_shader(color_shader);

	destroy_audio();
}
