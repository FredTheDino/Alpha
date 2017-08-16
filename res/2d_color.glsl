#version 300 es

precision highp vec2;
precision highp float;

uniform float aspect;
uniform vec2 cam_pos;
uniform float cam_rot;
uniform float cam_zoom;

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
	vec2 world_position = position + vec2(x, y);
	vec2 projected = vec2(world_position + cam_pos);

	float srot = sin(cam_rot);
	float crot = cos(cam_rot);

	projected = vec2(
		projected.x * crot - projected.y * srot, 
		projected.x * srot + projected.y * crot);

	projected.x /= aspect;

	gl_Position = vec4(projected, layer, cam_zoom);
	//gl_Position = vec4(position, layer, 1);
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
