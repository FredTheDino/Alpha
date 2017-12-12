// UNTESTED!

Vec2 project_along(Shape* s, Vec2 v) {
	Vec2 bounds = {FLT_MIN, FLT_MAX};
	for (auto p : s->points) {
		float d = dot(p, v);
		bounds[0] = min(bounds[0], d);
		bounds[1] = max(bounds[1], d);
	}

	return bounds;
}

void gen_normals_from_points(Array<Vec2> points, Array<Vec2>* normals) {
	int size = points.size();
	normals->reserve(size);
	Vec2 edge;

	for (int i = 0; i < size; i++) {
		int j = (i + 1) % size;
		edge = points[i] - points[j];

		bool unique = true;
		for (auto n : *normals) {
			if (dot(n, edge) == 0.0f) {
				unique = false;
				break;
			}
		}

		if (!unique) continue;
		
		Vec2 n = {-edge.y, edge.x}; // Rotation in constructor.
		normals->push_back(normalize(n));
	}
} 

Shape::Shape(PhysicsEngine& engine, Array<Vec2> points) {
	Vec2 sum;
	for (auto p : points) {
		sum = sum + p;
	}

	Vec2 center = sum / points.size();
	this->points.resize(points.size());

	for (int i = 0; i < points.size(); i++) {
		this->points[i] = points[i] - center;
	}
	this->offset = center;

	gen_normals_from_points(this->points, &this->normals);

	this->bounds = project_along(this, engine.broad_phase_normal);
}

Shape::Shape(PhysicsEngine& engine, Array<Vec2> points, Array<Vec2> normals): 
points(points), normals(normals) {
	this->bounds = project_along(this, engine.broad_phase_normal);
};

BodyID add_body(PhysicsEngine& engine, Body b) {
	BodyID id = {};

	b.alive = true;
	if (engine.next_free == -1) {
		// Add a new one
		id.pos = engine.bodies.size();
		engine.bodies.push_back(b);
	} else {
		// Use an old one
		id.pos = engine.next_free;
		// Update the next free.
		engine.next_free = -engine.bodies[id.pos].id.uid;
	}
	id.uid = engine.uid_gen++;
	b.id = id;
	engine.bodies[id.pos] = b;

	{
		Bound min_bound = {id, true,  b.shape->bounds[0]};
		Bound max_bound = {id, false, b.shape->bounds[1]};

		engine.bounds.push_back(min_bound);
		engine.bounds.push_back(max_bound);
	}

	return id;
}

BodyID add_body(PhysicsEngine& engine, Body b, Shape* s) {
	b.shape = s;
	return add_body(engine, b);
}

Body* find_body(PhysicsEngine& engine, BodyID id) {
	if (id.pos < 0) 
		return nullptr;
	if (engine.bodies.size() < id.pos) 
		return nullptr;

	const Body& b = engine.bodies[id.pos];
	if (b.id.uid != id.uid)
		return nullptr;

	if (b.alive)
		return &engine.bodies[id.pos];

	return nullptr;
}

bool remove_body(PhysicsEngine& engine, BodyID id) {
	Body* b = find_body(engine, id);
	if (b == nullptr)
		return false;

	b->alive  = false;
	b->id.uid = -id.uid;

	b->id.uid = -engine.next_free;
	engine.next_free = b->id.pos;
	
	int min_pos, max_pos;

	for (int i = 0; i < engine.bounds.size(); i++) {
		if (engine.bounds[i].id.pos == id.pos) {
			min_pos = i;

			for (int j = i; j < engine.bounds.size(); j++) {
				if (engine.bounds[j].id.pos == id.pos) {
					max_pos = j;
					break;
				}
			}

			break;
		}
	}

	engine.bounds.erase(min_pos);
	engine.bounds.erase(max_pos);

	return true;
}

bool can_collide(const Body& a, const Body& b) {
	if ((a.mask & b.mask) == 0) return false;
	if (a.mass == 0 && b.mass == 0) return false;
	return true;
}

void sort_bounds(Array<Bounds>& list) {
	for (int i = 1; i < list.size(); i++) {
		for (int j = i; list[j - 1].value > list[j].value && 0 < j; j--) {
			Bound b = list[j];
			list[j] = list[j - 1];
			list[j - 1] = b;
		}
	}
}

void update_physics_engine(PhysicsEngine& engine, float delta) {
	sort_bounds(engine.bounds);

	int num_potential_collisions = 0;
	int num_collisions = 0;
	for (int i = 0; i < engine.bounds.size(); i++) {
		if (!engine.bounds[i].is_min_bound) continue;

		BodyID id_a = engine.bounds[i].id;
		for (int j = i; j < engine.bounds.size(); j++) {
			if (!engine.bounds[j].is_min_bound) continue;
			BodyID id_b = engine.bounds[j].id;

			Body a = find_body(engine, id_a);
			Body b = find_body(engine, id_b);

			if (!can_collide(a, b)) continue;
			
			// Do actual collision test here...
		}
	}
}

