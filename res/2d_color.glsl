#version 330

in vec2 position;
in vec2 uv;

#define VERT 1
#ifdef VERT
//
// Vertex Shader
//

void main() {
	gl_Position = vec4(position.x, position.y, 0, 1);
}

#else
//
// Fragment Shader
//

out vec4 color;

void main() {
	color = vec4(0, 0, 0, 1);
}

#endif