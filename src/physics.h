
// Simular to entity.[h/cpp]

// Maybe add shapes to a list and remove all of them at the end, just to make them
// garbage collected.

struct Shape {
	Shape(PhysicsEngine& engine, Array<Vec2> points);
	Shape(PhysicsEngine& engine, Array<Vec2> points, Array<Vec2> normals); 
	Array<Vec2> points;
	Array<Vec2> normals;

	Vec2 bounds = {}; // For the broadphase.
	Vec2 offset = {};
};

struct BodyID {
	int pos;
	int uid;
};

struct Body {
	BodyID id;

	Shape* shape = nullptr;
	int8_t mask  = 0b11111111;
	bool alive   = false;

	Transform transform;
	float mass = 1;
};

struct Bound {
	BodyID id;
	bool is_min_bound = false;
	float value;
};

struct PhysicsEngine {
	Array<Body> bodies;
	int next_free = -1;
	unsigned short uid_gen = 0;

	Vec2 broad_phase_normal = Vec2(1, 0); // Doesn't need to be normalized, as long as all shape normals are normalized, which they have to be.
	Array<Bound> bounds;
} engine;
