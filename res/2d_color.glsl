#version 330

layout(location=0) in vec2 position;
layout(location=1) in vec2 uv;

#define VERT
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
	color = vec4(1, 0, 0, 1);
}

#endif
