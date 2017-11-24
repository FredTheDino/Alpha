#version 300 es

// This is needed for ES.
precision highp float;
precision highp int;

uniform float aspect;
uniform vec2  cam_pos;
uniform float cam_rot;
uniform float cam_zoom;

uniform sampler2D sprite;
uniform int sub_sprite;
uniform ivec2 sub_sprite_dim;
uniform float layer;

uniform vec2  position;
uniform vec2  scale;
uniform float rotation;

uniform vec3 color_hint;

#define VERT 1
#ifdef VERT
//
// Vertex Shader
//
in vec2 vert_position;
in vec2 vert_uv;

out vec2 fragUV;

vec2 rotate(vec2 point, float angle) {
	float srot = sin(angle);
	float crot = cos(angle);
	return vec2(
		point.x * crot - point.y * srot, 
		point.x * srot + point.y * crot);

}

void main() {
	vec2 world_position = rotate(vert_position * scale, rotation) + position;
	vec2 projected = vec2(world_position + cam_pos);

	projected = rotate(projected, cam_rot);

	projected.x /= aspect;

	gl_Position = vec4(projected / cam_zoom, layer, 1);

	// Assumes a unit square in the coordinates.
	if (sub_sprite_dim.x <= 1 && sub_sprite_dim.y <= 1) {
		fragUV = vert_uv;
	} else {
		int x_uv = sub_sprite % sub_sprite_dim.x;
		int y_uv = (sub_sprite - x_uv) / sub_sprite_dim.y;

		fragUV.x = (vert_uv.x + float(x_uv)) / float(sub_sprite_dim.x);
		fragUV.y = (vert_uv.y + float(y_uv)) / float(sub_sprite_dim.y);
	}
}

#else
//
// Fragment Shader
//

in vec2 fragUV;

out vec4 color;

void main() {
	vec4 texel = texture2D(sprite, fragUV);

	if (texel.w < 0.1) {
		discard;
	}

	color = texel * vec4(color_hint, 1);
	color.w = texel.w;
}

#endif
