struct Vec2;
struct Vec3;

float mod(float a, float b) {
	float d = a / b;
	return (d - floor(d)) * b;
}

int factorial(int i) {
	if (i == 0)
		return 1;
	return i * factorial(i - 1);
}

/*
struct Vec4;
struct Mat4;
*/

struct Vec4 {
	union {
		struct {
			float x;
			float y;
			float z;
			float w;
		};
		float _[4];
	};

	Vec4(float _x = 0, float _y = 0, float _z = 0, float _w = 0) {
		x = _x;
		y = _y;
		z = _z;
		w = _w;
	}
};

struct Vec3 {
	union {
		struct {
			float x;
			float y;
			float z;
		};
		float _[3];
	};

	Vec3(float _x = 0, float _y = 0, float _z = 0) {
		x = _x;
		y = _y;
		z = _z;
	}

	Vec3 operator+ (const Vec3& other) {
		return Vec3(x + other.x, y + other.y, z + other.z);
	}

	Vec3 operator- (const Vec3& other) {
		return Vec3(x - other.x, y - other.y, z - other.z);
	}

	Vec3 operator* (const float scale) {
		return Vec3(x * scale, y * scale, z * scale);
	}

	Vec3 operator/ (const float scale) {
		return Vec3(x / scale, y / scale, z / scale);
	}

};

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

	Vec2 operator+ (const Vec2& other) const {
		return Vec2(x + other.x, y + other.y);
	}

	Vec2 operator- (const Vec2& other) const {
		return Vec2(x - other.x, y - other.y);
	}

	Vec2 operator* (const float scale) const {
		return Vec2(x * scale, y * scale);
	}

	Vec2 operator/ (const float scale) const {
		return Vec2(x / scale, y / scale);
	}

	bool operator== (const Vec2& other) const {
		return x == other.x && y == other.y;
	}

	Vec2 operator- () const {
		return Vec2(-x, -y);
	}
};

Vec2 rotate(Vec2 v, float angle) {
	float s = sin(angle);
	float c = cos(angle);
	return Vec2(
			v.x * c - v.y * s, 
			v.x * s + v.y * c);
}

float length_sq(Vec2 v) {
	return v.x * v.x + v.y * v.y;
}

float length(Vec2 v) {
	return sqrt(length_sq(v));
}

Vec2 normalize(Vec2 v) {
	float scale = 1.0 / length(v);
	return v * scale;
}

float dot (const Vec2& a, const Vec2& b) {
	return a.x * b.x + b.y * a.y;
}

struct Transform {
	Vec2 position = {};
	Vec2 scale = {1, 1};
	float rotation = 0.0f;
};

float min(float v, float _min) {
	return v < _min ? v : _min;
}

float max(float v, float _max) {
	return _max < v ? v : _max;
}

float clamp(float v, float _min, float _max) {
	return max(min(v, _max), _min);
}

float sign(float s) {
	if (s == 0)
		return 0;
	if (s < 0)
		return -1.0f;
	return 1.0f;
}

