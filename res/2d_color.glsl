#version 150


uniform sampler2D sprite;

#define VERT 1
#ifdef VERT
//
// Vertex Shader
//
in vec2 position;
in vec2 uv;

out vec2 fragUV;

void main() {
	gl_Position = vec4(position.x, position.y, 0, 1);
	fragUV = uv;
}

#else
//
// Fragment Shader
//

in vec2 fragUV;

out vec4 color;

void main() {
	color = texture(sprite, fragUV);
}

#endif
