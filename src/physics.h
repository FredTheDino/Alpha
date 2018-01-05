
// Simular to entity.[h/cpp]

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

	Shape* shape    = nullptr;
	int8_t mask     = 0b11111111;
	int8_t hit_tags = 0b00000000;
	int8_t tag      = 0b00000000;
	bool alive      = false;
	bool is_trigger = false;
	// Rotation or scale isn't implemented in the 
	// collision test yet...

	Vec2 velocity;
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

	int8_t tag;
	bool is_trigger;

	float overlap;
	float impulse;
	float margin;
};

struct PhysicsEngine {
	Array<Body> bodies;
	int next_free = -1;
	unsigned short uid_gen = 0;

	int itterations = 2; // Run it multiple times for stability.
	float margin = 1.00001f;

	Vec2 broad_phase_normal = Vec2(1, 0); // Does not need to be unit length.
	Array<Bound> bounds;


	Vec2 gravity = Vec2(0, 0); // I think I want to do this manually...
} engine;
