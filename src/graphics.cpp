
// 
// Start of mesh stuff.
//

Mesh new_mesh(Array<Vertex> const& verticies) {
	Mesh m;

	glGenVertexArrays(1, &m.vao);
	glGenBuffers(1, &m.vbo);

	m.draw_count = verticies.size();

	glBindVertexArray(m.vao);
	{
		// Stream over the data
		glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
		glBufferData(GL_ARRAY_BUFFER, verticies.size() * sizeof(Vertex),
				&verticies[0], GL_STATIC_DRAW);

		// Position
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
				sizeof(Vertex), (GLvoid*) offsetof(Vertex, position));

		// Texture coordinate
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
				sizeof(Vertex), (GLvoid*) offsetof(Vertex, uv));
	}
	glBindVertexArray(0);
	return m;
}

void draw_mesh(Mesh m) {
	glBindVertexArray(m.vao);
	// Need to change this if I change to indexed drawing.
	glDrawArrays(GL_TRIANGLES, 0, m.draw_count);

	glBindVertexArray(0);
}

void delete_mesh(Mesh m) {
	glDeleteVertexArrays(1, &m.vao);
	glDeleteBuffers(1, &m.vbo);
}

//
// Shaders
//

bool glsl_error(GLuint target, GLenum flag, const char* message, bool is_program = false) {
	GLint success = 1;
	if (is_program) {
		glGetProgramiv(target, flag, &success);
	} else {
		glGetShaderiv(target, flag, &success);
	}

	if (success == GL_FALSE) {
		// @Robustness this will need to change if I feel it is needed.
		GLchar error[512];

		GLsizei length = 0;
		if (is_program) {
			glGetProgramInfoLog(target, sizeof(error), &length, error);
		} else {
			glGetShaderInfoLog(target, sizeof(error), &length, error);
		}

		printf("(%s): %s\n", message, error);
		return false;
	}
	return true;
}

GLuint compile_shader(String const& source, GLenum shader_type) {
	GLuint shader = glCreateShader(shader_type);
	if (!shader) {
		return -1;
	}

	const GLchar* src = (GLchar*) source.c_str();
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);

	if (!glsl_error(shader, GL_COMPILE_STATUS, "Shader Compilation")) {
		glDeleteShader(shader); // Don't leak the shader.
		return -1;
	}
	return shader;
}

GLuint link_program(GLuint Vertex_shader, GLuint fragment_shader) {
	GLuint program = glCreateProgram();

	glAttachShader(program, Vertex_shader);
	glAttachShader(program, fragment_shader);

	glLinkProgram(program);

	if(!glsl_error(program, GL_LINK_STATUS, "Program Linking", true)) {
		//We don't need the program anymore.
		glDeleteProgram(program);
		return -1;
	}

	glDetachShader(program, Vertex_shader);
	glDetachShader(program, fragment_shader);

	return program;
}

Shader new_shader(String path, String name) {
	String source;
	Shader s;
	s.name = name;

	FILE* file = fopen(path.c_str(), "r");

	fseek(file, 0, SEEK_END);
	// Add one for the null termination.
	size_t size = ftell(file) + 1;
	
	source.resize(size);

	rewind(file);

	fread(&source[0], 1, size, file);

	fclose(file);
	
	auto vertex_shader = compile_shader(source, GL_VERTEX_SHADER);

	// Change the def to turn it into the fragment shader.
	for (size_t i = 0; i < size; i++) {
		if (source[i+0] != 'V') continue;
		if (source[i+1] != 'E') continue;
		if (source[i+2] != 'R') continue;
		if (source[i+3] != 'T') continue;

		source[i] = 'Q';
		break;
	}
	
	auto fragment_shader = compile_shader(source, GL_FRAGMENT_SHADER);

	if (fragment_shader != -1 && vertex_shader != -1) {
		s.program = link_program(vertex_shader, fragment_shader);
	}
	
	if (vertex_shader != -1) {
		glDeleteShader(vertex_shader);
	}

	if (fragment_shader != -1) {
		glDeleteShader(vertex_shader);
	}

	return s;
}

void use_shader(const Shader& s) {
	glUseProgram(s.program);
}

void delete_shader(Shader s) {
	glDeleteProgram(s.program);
}

GLuint get_uniform_loc(const Shader s, String name) {
	return glGetUniformLocation(s.program, name.c_str());
}


//
// Start of texture stuff.
//

//
// These are all the supported image formats. (We will find these outselves)
// So don't include the file suffix in the engine.
//
#define array_len(A) (sizeof( A )/sizeof( A [0]))
String supported_texture_formats[] = {".jpg", ".png", ".tga", ".psd", ".gif"};


String find_texture_file(String path) {
	String file_path;
	for (int i = 0; i < array_len(supported_texture_formats); i++) {
		// 
		// @Speed, we dont need to re copy the entire file name,
		// we could just copy over the last chars since each
		// file suffix is the same length
		//
		file_path = path + supported_texture_formats[i];
		if (access(file_path.c_str(), F_OK) == 0) {
			return file_path;
		}
	}

	printf("[Graphics.cpp] Failed to load image '%s', the file could not be found. Supported file formats:", path.c_str());
	for (int i = 0; i < array_len(supported_texture_formats); i++) {
		printf(" %s", supported_texture_formats[i].c_str());
	}
	printf("\n");

	return "";
}

Texture new_texture(
		String path, bool linear_filtering = true, 
		int sprites_x = 0, int sprites_y = 0, 
		bool use_mipmaps = false) {

	String file_path = find_texture_file(path);
	int width, height, num_channels = 0;

	/*
#ifdef LINUX
	FILE* file = fopen(file_path.c_str(), "r");
	assert(file);
	
	unsigned char* data = stbi_load_from_file(file, &width, &height, &num_channels, 4);

	fclose(file);
#elif WINDOWS
*/
	unsigned char* data = stbi_load(file_path.c_str(), &width, &height, &num_channels, 4);
//#endif

	if ((width == 0 && height == 0) || data == nullptr) {
		printf("Failed to load image. '%s'\n", file_path.c_str());
		Texture t;
		t.texture_id = -1;
		return t;
	}

	GLuint id;
	glGenTextures(1, &id);

	glBindTexture(GL_TEXTURE_2D, id);

	GLenum filter = linear_filtering ? GL_LINEAR : GL_NEAREST;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	if (use_mipmaps) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	GLenum format = GL_RGBA;
	if (num_channels == 1) {
		format = GL_RED;
	} else if (num_channels == 2) {
		format = GL_RED | GL_GREEN;
	} else if (num_channels == 3) {
		format = GL_RGB;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, data);

	Texture t;
	t.texture_id = id;
	t.w = width;
	t.h = height;
	t.sprites_x = sprites_x;
	t.sprites_y = sprites_y;

	return t;
}

bool update_texture(Texture& t, String path) {
	// Can't do this cause the hotloader wants
	// to know exactyly which file it is, so
	// here the path isn't checked...

	int width, height, num_channels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &num_channels, 4);

	if ((width == 0 && height == 0) || data == nullptr) {
		return false;
	}
	
	glBindTexture(GL_TEXTURE_2D, t.texture_id);

	GLenum format = GL_RGBA;
	if (num_channels == 1) {
		format = GL_RED;
	} else if (num_channels == 2) {
		format = GL_RED | GL_GREEN;
	} else if (num_channels == 3) {
		format = GL_RGB;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, data);

	t.w = width;
	t.h = height;

	return true;
}

void delete_texture(Texture& t) {
	glDeleteTextures(1, &t.texture_id);
}

void bind_texture(Texture const & t, int texture_slot) {
	glActiveTexture(GL_TEXTURE0 + texture_slot);
	glBindTexture(GL_TEXTURE_2D, t.texture_id);
}

//
// @Robustness: Maybe we shouldn't send this in via uniforms,
// maybe we should send it in once per frame to a UBO, just
// to make sure we don't send more then we need.
//
// This is only if we need it, and it becomes a hazzle to manage
// multiple shaders.
//

#define GET_LOC(name) glGetUniformLocation(s.program, name)
void send_camera_to_shader(Shader s, Camera c = main_camera) {
	GLint aspect   = GET_LOC("aspect");
	GLint cam_pos  = GET_LOC("cam_pos");
	GLint cam_rot  = GET_LOC("cam_rot");
	GLint cam_zoom = GET_LOC("cam_zoom");

	glUniform1f(aspect,   global.window_aspect_ratio);
	glUniform1f(cam_rot,  c.rotation);
	glUniform2f(cam_pos,  c.position.x, c.position.y);
	glUniform1f(cam_zoom, c.zoom);
}

void draw_sprite(Shader s, const Texture& texture, int sub_sprite, 
		Vec2 position, Vec2 scale = {1, 1}, float rotation = 0, 
		Vec4 color_hint = {1, 1, 1, 1}, float layer = 0.0f) {

	bind_texture(texture, 0);
	glUniform1i(GET_LOC("sprite"), 0);
	glUniform2i(GET_LOC("sub_sprite_dim"), texture.sprites_x, texture.sprites_y);
	glUniform1i(GET_LOC("sub_sprite"), sub_sprite);

	glUniform1f(GET_LOC("layer"), layer);
	glUniform4f(GET_LOC("color_hint"), color_hint.x, color_hint.y, color_hint.z, color_hint.w);

	glUniform2f(GET_LOC("position"), position.x, position.y);
	glUniform2f(GET_LOC("scale"), scale.x, scale.y);
	glUniform1f(GET_LOC("rotation"), rotation);

	draw_mesh(quad_mesh);
}

//
// FONT STUFF!
//

void split(String text, Array<String>& s, char c1 = ' ', char c2 = '\0', char c3 = '\0') {
	int word_start = -1;
	int word_end = -1;

	for (int i = 0; i <= text.size(); i++) {
		if (text[i] == c1 || text[i] == c2 || text[i] == c3
				|| text[i] == '\n' || i == text.size()) {
			word_end = i;
			if (word_start == -1) continue;

			s.push_back(text.substr(word_start, word_end - word_start));
			word_start = -1;
		} else {
			if (word_start == -1)
				word_start = i;
		}
	}
}

inline Array<String> split(String text, char c1 = ' ', char c2 = '\0', char c3 = '\0') {
	Array<String> s;
	split(text, s, c1, c2, c3);
	return s;
}

Font load_font_from_files(String path) {
	Font font;
	font.texture = new_texture(path);
	if (font.texture.texture_id == -1) {
		printf("Failed to load font image \"%s\", no file found.\n", path.c_str());
		return font;
	}
	
	FILE* file = fopen((path + ".fnt").c_str(), "r");
	if (!file) {
		printf("Failed to load font meta data \"%s\".fnt, no file found.\n", path.c_str());
		return font;
	}

	Vec2 pixel_to_uv(
			1.0f / font.texture.w, 
			1.0f / font.texture.h);
	
	Array<String> split_line;
	size_t line_size;
	char* line;
	while (!feof(file)) {
		// Reading lines!
		line_size = 0;
		line = nullptr;
		getline(&line, &line_size, file);
		split(String(line), split_line);

		if (split_line[0] == "char") {
			Face face;
			// Parse all the data and store it in a struct
			face.id = stoi(split(split_line[1], '=')[1]);
			face.u  = stoi(split(split_line[2], '=')[1]) * pixel_to_uv.x;
			face.v  = stoi(split(split_line[3], '=')[1]) * pixel_to_uv.y;
			face.w  = stoi(split(split_line[4], '=')[1]) * pixel_to_uv.x;
			face.h  = stoi(split(split_line[5], '=')[1]) * pixel_to_uv.y;

			face.offset_x  = stoi(split(split_line[6], '=')[1]) * pixel_to_uv.y;
			face.offset_y  = stoi(split(split_line[7], '=')[1]) * pixel_to_uv.y;
			face.advance   = stoi(split(split_line[8], '=')[1]) * pixel_to_uv.y;

			// Save the struct
			font.faces[face.id] = face;
			// Find the limits
			if (face.v != 0) {
				font.min_height = fmin(font.min_height, face.v) * pixel_to_uv.x;
				font.max_height = fmax(font.max_height, face.v + face.h) * pixel_to_uv.x;
			}
		}
		free(line);
		split_line.clear();
	}

	return font;
}

#define SPRITE_MODE 0
#define FONT_MODE 1
#define FILL_MODE 2

void draw_text(const Shader s, const Font& f, const Mesh m, 
		Vec2 position = {0, 0}, Vec2 scale = {1, 1}, 
		float rotation = 0.0f, Vec4 color_hint = {1, 1, 1, 1}, 
		float layer = 0.0f) {
	GLint draw_mode  = GET_LOC("draw_mode");
	glUniform1i(draw_mode, FONT_MODE); // We're drawing text.

	bind_texture(f.texture, 0);
	glUniform1i(GET_LOC("sprite"), 0);

	glUniform1f(GET_LOC("layer"), layer);
	glUniform2f(GET_LOC("position"), position.x, position.y);
	glUniform1f(GET_LOC("rotation"), rotation);
	glUniform2f(GET_LOC("scale"), scale.x, scale.y);
	glUniform4f(GET_LOC("color_hint"), color_hint.x, color_hint.y, color_hint.z, color_hint.w);

	draw_mesh(m);

	glUniform1i(draw_mode, SPRITE_MODE); // Reset.
}

float length_of_text(const Font& f, const String text, 
		float size = 1.0f, float spacing = 1.0f) {
	float total = 0.0f;
	for (char c : text) {
		Face face = f.faces.at(c);
		total += (face.advance + face.offset_x) * size * spacing;
	}

	return total;
}

Mesh new_text_mesh(const Font& f, const String text, 
		float size = 1.0f, float spacing = 1.0f) {
	Array<Vertex> verticies;
	verticies.reserve(text.size() * 6);

	float cursor_pos = length_of_text(f, text, size, spacing) / 2;
	float base_line = f.max_height * size;
	
	for (char c : text) {
		Face face = f.faces.at(c);

		if (face.w != 0.0f) {
			float bx   = cursor_pos + face.offset_x * size;
			float bx_n = bx + face.w * size;

			float by   = base_line - face.offset_y * size;
			float by_n = by - face.h * size;

			float u    = face.u;
			float u_n  = u + face.w;

			float v    = face.v;
			float v_n  = v + face.h;

			verticies.push_back(Vertex(bx  , by_n, u  , v_n));
			verticies.push_back(Vertex(bx  , by  , u  , v));
			verticies.push_back(Vertex(bx_n, by_n, u_n, v_n));
			verticies.push_back(Vertex(bx  , by  , u  , v));
			verticies.push_back(Vertex(bx_n, by  , u_n, v));
			verticies.push_back(Vertex(bx_n, by_n, u_n, v_n));
		}

		cursor_pos += (face.advance + face.offset_x) * size * spacing;
	};

	return new_mesh(verticies);
}

void debug_draw_points(Shader s, Array<Vec2> points, Vec2 offset = {0, 0},
		Vec4 color_hint = {0.6, 0.3, 0.8, 1}) {

	glUniform1i(GET_LOC("draw_mode"), FILL_MODE);

	glUniform2f(GET_LOC("position"), offset.x, offset.y);
	glUniform1f(GET_LOC("rotation"), 0);
	glUniform2f(GET_LOC("scale"), 1, 1);
	glUniform4f(GET_LOC("color_hint"), color_hint.x, color_hint.y, color_hint.z, color_hint.w);
	glUniform1f(GET_LOC("layer"), 10.0f);

	glBegin(GL_POLYGON); 
	{
		for (auto p : points) {
			glVertex4f(p.x, p.y, p.x, p.y);
		}
	}
	glEnd();

	glUniform1i(GET_LOC("draw_mode"), SPRITE_MODE);
}

Vec4 hsv_to_rgb(float h, float s, float v) {
	float clamped_h = mod(h, 360.0f);
	float c = v * s;
	float x = c * (1 - fabs(mod(clamped_h / 60.0f, 2)) - 1);
	float m = v - c;

	Vec4 out(m, m, m, 1);
	if (clamped_h < 60) {
		out.x += c;
		out.y += x;
	} else if (clamped_h < 120) {
		out.x += x;
		out.y += c;
	} else if (clamped_h < 180) {
		out.y += c;
		out.w += x;
	} else if (clamped_h < 240) {
		out.y += x;
		out.z += c;
	} else if (clamped_h < 300) {
		out.x += x;
		out.z += c;
	} else if (clamped_h < 360) {
		out.x += c;
		out.w += x;
	}
	return out;
}

void debug_draw_body(Shader s, Body b) {
	Vec4 color = hsv_to_rgb(b.id.pos * 50, (b.id.uid % 50) / 100 + 0.2, b.mass == 0 ? 0.4f : 0.75f);
	debug_draw_points(s, b.shape->points, 
			b.shape->offset + b.position, color);
}

void debug_draw_body(Shader s, PhysicsEngine& engine, const BodyID id) {
	auto b = find_body(engine, id);
	if (!b) return;

	debug_draw_body(s, *b);
}

void debug_draw_engine(Shader s, PhysicsEngine& engine) {
	for (Body& b : engine.bodies) {
		debug_draw_body(s, b);
	}
}
