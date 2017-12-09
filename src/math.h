struct Vec2;
struct Vec3;
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
