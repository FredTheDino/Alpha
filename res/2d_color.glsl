#version 150


uniform sampler2D sprite;
uniform float layer;

uniform float x;
uniform float y;

uniform float color_scale;

#define VERT 1
#ifdef VERT
//
// Vertex Shader
//
in vec2 position;
in vec2 uv;

out vec2 fragUV;

void main() {
	gl_Position = vec4(position.x + x, position.y + y, layer, 1);
	fragUV = uv;
}

#else
//
// Fragment Shader
//

in vec2 fragUV;

out vec4 color;

void main() {
	vec4 texel = texture(sprite, fragUV);

	if (texel.w < 0.1) {
		discard;
	}
	color = texel * color_scale;
	color.w = texel.w;
}

#endif
