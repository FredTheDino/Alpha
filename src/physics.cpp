// UNTESTED!

Vec2 project_along(Shape* s, Vec2 v) {
	Vec2 bounds = {FLT_MAX, -FLT_MAX};
	for (auto p : s->points) {
		float d = dot(p, v);
		bounds._[0] = fmin(bounds._[0], d);
		bounds._[1] = fmax(bounds._[1], d);
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
		Bound min_bound(id, true);
		Bound max_bound(id, false);

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

	engine.bounds.erase(engine.bounds.begin() + min_pos);
	engine.bounds.erase(engine.bounds.begin() + max_pos);

	return true;
}

bool can_collide(const Body& a, const Body& b) {
	if ((a.mask & b.mask) == 0) return false;
	if (a.mass == 0 && b.mass == 0) return false;
	return true;
}

void update_bounds(PhysicsEngine& engine, Array<Bound>& list) {
	for (auto& b : list) {
		Body& body = *find_body(engine, b.id);

		b.value = dot(body.transform.position, engine.broad_phase_normal);
		b.value += body.shape->bounds._[!b.is_min_bound];
	}

	for (int i = 1; i < list.size(); i++) {
		for (int j = i; list[j - 1].value > list[j].value && 0 < j; j--) {
			Bound b = list[j];
			list[j] = list[j - 1];
			list[j - 1] = b;
		}
	}
}

void update_physics_engine(PhysicsEngine& engine, float delta) {
	update_bounds(engine, engine.bounds);

	int num_potential_collisions = 0;
	int num_collisions = 0;
	printf("listing bounds:\n");
	for (int i = 0; i < engine.bounds.size(); i++) {
		BodyID id_a = engine.bounds[i].id;
		printf("%d) @%d, v: %f, left:%d\n", i, id_a.pos, 
				engine.bounds[i].value, engine.bounds[i].is_min_bound);
		if (!engine.bounds[i].is_min_bound) continue;

		for (int j = i; j < engine.bounds.size(); j++) {
			BodyID id_b = engine.bounds[j].id;
			if (id_b.pos == id_a.pos) break;

			if (!engine.bounds[j].is_min_bound) continue;

			num_potential_collisions++;

			Body* a = find_body(engine, id_a);
			Body* b = find_body(engine, id_b);

			if (!can_collide(*a, *b)) continue;
			
			// Do actual collision test here...
			num_collisions++;
		}
	}

	printf("num_pot: %d\n    num: %d\nnum_bodies_squared: %d\n", 
			num_potential_collisions, num_collisions, 
			engine.bodies.size() * engine.bodies.size());
}

