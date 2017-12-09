#version 300 es

// This is needed for ES.
precision highp float;
precision highp int;

uniform sampler2D screen;
uniform float time;

uniform vec2 sample_size;
uniform int msaa;

#define VERT 1
#ifdef VERT
//
// Vertex Shader
//
layout(location=0) in vec2 position;
layout(location=1) in vec2 uv;

out vec2 fragUV;

void main() {
	gl_Position = vec4(position.x * 2.0, position.y * 2.0, 0, 1);
	fragUV = vec2(uv.x, 1.0 - uv.y);
}

#else
//
// Fragment Shader
//

in vec2 fragUV;

out vec4 color;

int pixels_x = 300;
int pixels_y = 300;

void main() {
	//vec2 uv = vec2(floor(fragUV.x * pixels_x) / pixels_x, floor(fragUV.y * pixels_y) / pixels_y);
	vec2 uv = fragUV * float(msaa);
	vec4 texel;

	texel = texture(screen, uv);
	if (msaa > 1) {
		for (int x = 1 - msaa; x < msaa - 1; x++) {
			for (int y = 1 - msaa; y < msaa - 1; y++) {
				float sample_x = uv.x + sample_size.x * float(x);
				float sample_y = uv.y + sample_size.y * float(y);
				float weight = 0.5f / (float(x * x + y * y) + 1.0f);
				texel = mix(texel, texture(screen, vec2(sample_x, sample_y)), weight);
			}
		}
	} 

	float gamma = 1.5;
	color = vec4(
		(texel.r - 0.5) * gamma + 0.5, 
		(texel.g - 0.5) * gamma + 0.5, 
		(texel.b - 0.5) * gamma + 0.5, 1.0);
	color.w = 1.0;
}
#endif
