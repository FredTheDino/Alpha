
// Simular to entity.[h/cpp]

// Maybe add shapes to a list and remove all of them at the end, just to make them
// garbage collected.

struct PhysicsEngine;
struct Collision;

struct Shape {
	Shape(PhysicsEngine& engine, Array<Vec2> points);
	Shape(PhysicsEngine& engine, Array<Vec2> points, Array<Vec2> normals); 
	Array<Vec2> points;
	Array<Vec2> normals;

	Vec2 bounds = {}; // For the broadphase.
	Vec2 offset = {}; // This should be set on the body.
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

	// Transform transform; 
	// Rotation or scale isn't implemented in the 
	// collision test yet...

	Vec2 velocity;
	Vec2 last_position;
	Vec2 position;
	float mass = 1;

	Array<Collision> collisions;
};

struct Bound {
	Bound(BodyID id) : id(id) {};

	BodyID id;
	Vec2 v;
};

struct Collision {
	BodyID id_a;
	BodyID id_b;
	Body* a;
	Body* b;

	Vec2 normal;
	Vec2 selected_normal;

	float overlap;
	float elasticity = 0.5f;
};

struct PhysicsEngine {
	Array<Body> bodies;
	int next_free = -1;
	unsigned short uid_gen = 0;

	Vec2 broad_phase_normal = Vec2(1, 0); // Doesn't need to be normalized, as long as all shape normals are normalized, which they have to be.
	Array<Bound> bounds;
	int itterations = 2;

	Vec2 gravity = Vec2(0, -1);
} engine;
