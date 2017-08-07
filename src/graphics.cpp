
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
	GLuint program; 
	String name;
};  

Mesh new_mesh(Array<Vertex> const& verticies) {
	Mesh m;

	glGenVertexArrays(1, &m.vao);
	glGenBuffers(1, &m.vbo);

	m.draw_count = size(verticies);

	glBindVertexArray(m.vao);
	{
		// Stream over the data
		glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
		glBufferData(GL_ARRAY_BUFFER, size(verticies) * sizeof(Vertex),
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

	const GLchar* src = (GLchar*) source._data;
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

	FILE* file = fopen(path._data, "r");

	fseek(file, 0, SEEK_END);
	// Add one for the null termination.
	size_t size = ftell(file) + 1;
	
	reserve(source, size);
	source._size = size;

	rewind(file);

	fread(source._data, 1, size, file);

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

