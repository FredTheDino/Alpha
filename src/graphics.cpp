
// Maybe in math?
struct Vec2;
/*
struct Vec3;
struct Vec4;
struct Mat4;
*/

struct Vec2 {
	union {
		struct {
			float x;
			float y;

		};
		float _[2];
	};

	Vec2(float _x = 0, float _y = 0) {
		x = _x;
		y = _y;
	}

	Vec2 operator+ (const Vec2& other) {
		return Vec2(x + other.x, y + other.y);
	}

	Vec2 operator- (const Vec2& other) {
		return Vec2(x - other.x, y - other.y);
	}

	Vec2 operator* (const float scale) {
		return Vec2(x * scale, y * scale);
	}

	Vec2 operator/ (const float scale) {
		return Vec2(x / scale, y / scale);
	}

};

float dot (const Vec2& a, const Vec2& b) {
	return a.x * b.y + b.x * a.y;
}

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
	int w, h;
	unsigned texture_id;
	unsigned sprites_x = 1;
	unsigned sprites_y = 1;
};

struct Shader {
	GLuint program = -1; 
	String name;
};  

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
// Start of shader stuff,
//

bool check_glsl_error(GLuint target, GLenum flag, const char* message, bool is_program = false) {
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

	if (!check_glsl_error(shader, GL_COMPILE_STATUS, "Shader Compilation")) {
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

	if(!check_glsl_error(program, GL_LINK_STATUS, "Program Linking", true)) {
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

	return "";
}

Texture new_texture(
		String path, bool linear_filtering = true, 
		int sprites_x = 0, int sprites_y = 0, 
		bool use_mipmaps = false) {

	String file_path = find_texture_file(path);
	int width, height, num_channels = 0;

#ifdef LINUX
	FILE* file = fopen(file_path.c_str(), "r");
	assert(file);
	
	unsigned char* data = stbi_load_from_file(file, &width, &height, &num_channels, 4);

	fclose(file);
#elif WINDOWS
	unsigned char* data = stbi_load(file_path.c_str(), &width, &height, &num_channels, 4);
#endif

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
	FILE* file = fopen(path.c_str(), "r");
	if (file == 0) {
		// Don't sweat it if we can't find the file.
		return false;
	}

	int width, height, num_channels;
	unsigned char* data = stbi_load_from_file(file, &width, &height, &num_channels, 4);

	if ((width == 0 && height == 0) || data == nullptr) {
		return false;
	}
	
	// Done with the file.
	fclose(file);

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

void bind_texture(Texture& t, int texture_slot) {
	glActiveTexture(GL_TEXTURE0 + texture_slot);
	glBindTexture(GL_TEXTURE_2D, t.texture_id);
}
