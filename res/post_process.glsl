#version 150

uniform sampler2D screen;
uniform float time;

#define VERT 1
#ifdef VERT
//
// Vertex Shader
//
in vec2 position;
in vec2 uv;
out vec2 fragUV;

void main() {
	gl_Position = vec4(position * 2, 0, 1);
	fragUV = vec2(uv.x, 1 - uv.y);
}

#else
//
// Fragment Shader
//

in vec2 fragUV;

out vec4 color;

void main() {
	
	vec2 uv = fragUV + 0.005 * vec2(sin(time + 20 * fragUV.x), cos(time + 50 * fragUV.y));

	vec4 texel;
	texel = texture(screen, uv);
	color = texel;
}

#endif
