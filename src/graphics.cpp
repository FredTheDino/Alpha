
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


