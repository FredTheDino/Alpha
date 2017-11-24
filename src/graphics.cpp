
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

#define GET_LOC(name) glGetUniformLocation(s.program, name);
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
		Vec4 color_hint = {1, 1, 1, 1}, float layer = 0) {
	GLint sprite_loc =         GET_LOC("sprite");
	GLint sub_sprite_dim_loc = GET_LOC("sub_sprite_dim");
	GLint sub_sprite_loc =     GET_LOC("sub_sprite");
	GLint layer_loc =          GET_LOC("layer");

	GLint pos_loc =   GET_LOC("position");
	GLint scale_loc = GET_LOC("scale");
	GLint rot_loc =   GET_LOC("rotation");

	GLint color_hint_loc = GET_LOC("color_hint");

	bind_texture(texture, 0);
	glUniform1i(sprite_loc, 0);
	glUniform2i(sub_sprite_dim_loc, texture.sprites_x, texture.sprites_y);
	glUniform1i(sub_sprite_loc, sub_sprite);

	glUniform1f(layer_loc, layer);

	glUniform2f(pos_loc, position.x, position.y);
	glUniform2f(scale_loc, scale.x, scale.y);
	glUniform1f(rot_loc, rotation);
	glUniform3f(color_hint_loc, color_hint.x, color_hint.y, color_hint.z);

	draw_mesh(quad_mesh);
}

#undef GET_LOC

