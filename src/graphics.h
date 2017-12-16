#pragma once
struct Vertex {
	Vec2 position;
	Vec2 uv;

	Vertex(float x, float y, float u, float v) {
		position.x = x;
		position.y = y;
		uv.x = u;
		uv.y = v;
	}
};

struct Mesh {
	unsigned vbo;
	unsigned vao;
	unsigned draw_count;
};

struct Texture {
	Texture() {}
	Texture(String path, int sx, int sy);
	int w, h;
	unsigned texture_id = -1;
	unsigned sprites_x;
	unsigned sprites_y;
};

struct Shader {
	Shader() {}
	Shader(String path, String name);
	GLuint program = -1; 
	String name;
};  

struct Camera {
	Vec2 position = {0, 0};
	float rotation = 0;
	float zoom = 1;
} main_camera;

struct Face {
	// The character.
	char id = 0;
	// The start of the quad.
	float u, v = 0;
	// The size of the quad.
	float w, h = 0;
	// The offsets
	float offset_x, offset_y = 0;
	// How long until the next char.
	float advance = 0;
};

struct Font {
	HashMap<char, Face> faces;
	float min_height = 100.0f;
	float max_height = -100.0f;

	Texture texture;
};

