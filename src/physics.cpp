Vec2 project_along(Shape* s, Vec2 v) {
	Vec2 bounds = {FLT_MAX, -FLT_MAX};
	for (auto p : s->points) {
		float d = dot(p, v);
		bounds._[0] = fmin(bounds._[0], d);
		bounds._[1] = fmax(bounds._[1], d);
	}

	return bounds;
}

void gen_normals_from_points(Array<Vec2> points, Array<Vec2>& normals) {
	int size = points.size();
	Vec2 edge;
	normals.resize(size);
	int curr = 0;

	for (int i = 0; i < size; i++) {
		int j = (i + 1) % size;
		edge = points[i] - points[j];

		if (edge.x == 0 && edge.y == 0) continue;

		bool unique = true;
		for (int i = 0; i < curr; i++) {
			if (dot(normals[i], edge) == 0.0f) {
				unique = false;
				break;
			}
		}

		if (!unique) continue;
		
		Vec2 n = {-edge.y, edge.x}; // Rotation in constructor.
		normals[curr++] = normalize(n);
	}

	normals.resize(curr);
} 

Shape::Shape(PhysicsEngine& engine, Array<Vec2> points) {
	Vec2 sum;
	for (auto p : points) {
		sum = sum + p;
	}

	Vec2 center = sum / points.size();
	this->points = Array<Vec2>(points.size());

	for (int i = 0; i < points.size(); i++) {
		this->points[i] = points[i] - center;
	}
	this->offset = center;

	gen_normals_from_points(this->points, this->normals);

	this->bounds = project_along(this, engine.broad_phase_normal);
}

Shape::Shape(PhysicsEngine& engine, Array<Vec2> points, Array<Vec2> normals): 
points(points), normals(normals) {
	this->bounds = project_along(this, engine.broad_phase_normal);
};

inline Shape scale_shape(PhysicsEngine& engine, const Shape& shape, float scale) {
	Array<Vec2> points(shape.points.size());
	for (int i = 0; i < points.size(); i++) {
		points[i] = shape.points[i] * scale;
	}

	return Shape(engine, points);
}

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
		Bound bound(id);
		engine.bounds.push_back(bound);
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
	
	for (int i = 0; i < engine.bounds.size(); i++) {
		if (engine.bounds[i].id.pos == id.pos) {
			engine.bounds.erase(engine.bounds.begin() + i);
			break;
		}
	}

	return true;
}

void clear(PhysicsEngine& engine) {
	for (Body& b : engine.bodies) {
		if (!b.alive) continue;

		b.alive = false;
		b.id.uid = -b.id.uid;
		b.id.uid = -engine.next_free;
		engine.next_free = b.id.pos;
	}

	engine.bounds.clear();
}


bool can_collide(const Body& a, const Body& b) {
	if ((a.mask & b.mask) == 0) return false;
	if (a.mass == 0 && b.mass == 0) return false;
	return true;
}

void fuse_without_duplicates(const Array<Vec2>& a, const Array<Vec2>& b, 
		Array<Vec2>& out) {
	out = a;
	out.reserve(a.size() + b.size());
	
	for (auto data_b : b) {
		bool unique = true;
		for (auto data_a : a) {
			if (data_a == data_b) {
				unique = false;
				break;
			}
		}

		if (unique) {
			out.push_back(data_b);
		}
	}

}

bool collision_check(Collision* c) {
	Body a = (*c->a);
	Body b = (*c->b);

	c->overlap = FLT_MAX;

	Vec2 delta_pos = 
		(a.position + a.shape->offset) - 
		(b.position + b.shape->offset);


	Array<Vec2> normals;
	fuse_without_duplicates(a.shape->normals, b.shape->normals, normals);

	for (Vec2 n : normals) {
		float projected_distance = dot(delta_pos, n);

		float a_projection;
		float b_projection;

		bool pointing_towards_a = projected_distance < 0;
		if (pointing_towards_a) {
			a_projection = project_along(a.shape, n)._[1];
			b_projection = project_along(b.shape, n)._[0];
		} else {
			a_projection = project_along(a.shape, n)._[0];
			b_projection = project_along(b.shape, n)._[1];
		}

		float d   = fabs(projected_distance);
		float sum = fabs(a_projection) + fabs(b_projection);

		if (d < sum) {
			float overlap = sum - d;
			if (overlap < c->overlap) {
				c->overlap = overlap;
				if (pointing_towards_a)
					c->normal = n;
				else
					c->normal = -n;
			}
		} else {
			return false;
		}
	}

	return true;
}

void update_bounds(PhysicsEngine& engine, Array<Bound>& list) {
	for (auto& b : list) {
		Body& body = *find_body(engine, b.id);

		float projection = 
			dot(body.position, engine.broad_phase_normal);
		b.v = Vec2(projection, projection) + body.shape->bounds - body.shape->offset;
	}

	for (int i = 1; i < list.size(); i++) {
		for (int j = i; list[j - 1].v._[0] > list[j].v._[0] && 0 < j; j--) {
			Bound b = list[j];
			list[j] = list[j - 1];
			list[j - 1] = b;
		}
	}
}

#define COLLISION_MARGIN 1.0001f

void move_ds(Collision& c) {
	Body* d;

	Vec2 normal; // Should point from the static
	if (c.a->mass == 0) {
		d = c.b;
		normal = c.normal;
	} else {
		d = c.a;
		normal = -c.normal;
	}

	// Move the dynamic body
	d->position = d->position + normal * c.overlap * COLLISION_MARGIN;
}

void solve_ds(Collision& c) {
	Body* d;
	Body* s;

	Vec2 normal; // Should point from the static
	if (c.a->mass == 0) {
		d = c.b;
		s = c.a;
		normal = c.normal;
	} else {
		d = c.a;
		s = c.b;
		normal = -c.normal;
	}

	// Cancel all the velocity going towards the static
	float dot_vel = dot(d->velocity, normal);
	if (0 < dot_vel) return;
	d->velocity = d->velocity - normal * dot_vel * (1 + c.elasticity);
}

void move_dd(Collision& c) {
	Body* a = c.a;
	Body* b = c.b;
	
	// Make sure they don't penetrate, based on mass ratio
	float inverted_total_mass = 1.0f / (a->mass + b->mass);
	float a_distnace = inverted_total_mass * a->mass * c.overlap;
	float b_distnace = inverted_total_mass * b->mass * c.overlap;
	a->position = a->position - c.normal * a_distnace * COLLISION_MARGIN;
	b->position = b->position + c.normal * b_distnace * COLLISION_MARGIN;

}

void solve_dd(Collision& c) {
	Body* a = c.a;
	Body* b = c.b;

	// Make sure they don't penetrate, based on mass ratio
	float inverted_total_mass = 1.0f / (a->mass + b->mass);
	
	// We're done if they're not gonna collide.
	if (dot(a->velocity, b->velocity) > 0) return; // Shouldn't this be the other way around?

	Vec2 relative_velocity = a->velocity - b->velocity;
	float relative_1d_momentum = dot(relative_velocity, c.normal) * 
		inverted_total_mass * (1 + c.elasticity);

	a->velocity = a->velocity - c.normal * relative_1d_momentum * b->mass;
	b->velocity = b->velocity + c.normal * relative_1d_momentum * a->mass;
}

#undef COLLISION_MARGIN

void update_physics_engine(PhysicsEngine& engine, float delta) {
	for (Body& b : engine.bodies) {
		b.collisions.clear();
		if (b.mass == 0) continue;
		b.velocity = b.velocity + engine.gravity * delta;
		b.last_position = b.position;
		b.position = b.position + b.velocity * delta;
	}	

	update_bounds(engine, engine.bounds);

	int num_potential_collisions = 0;
	int num_collisions = 0;

#ifdef DEBUG_PHYSICS
	printf("---------------\n");
#endif
	for (int step = 0; step < engine.itterations; step++) {
		for (int i = 0; i < engine.bounds.size() - 1; i++) {
			Bound bound_a = engine.bounds[i];

#ifdef DEBUG_PHYSICS
			printf("%d) @%d, min: %f, max:%f\n", i, bound_a.id.pos, 
			bound_a.v._[0], bound_a.v._[1]);
#endif

			for (int j = i + 1; j < engine.bounds.size(); j++) {

				Bound bound_b = engine.bounds[j];
				if (bound_a.v._[1] <= bound_b.v._[0]) break;

				num_potential_collisions++;

				Body* a = find_body(engine, bound_a.id);
				Body* b = find_body(engine, bound_b.id);

				// printf("-bp- %d hit %d\n", i, j);
				if (!can_collide(*a, *b)) continue;

				// Do actual collision test here...
				//
				// The broadphase might not be as thuroughly
				// tested as it should be.
				//

#ifdef DEBUG_PHYSICS
				printf("-bp- %d hit %d\n", i, j);
#endif

				Collision c;
				c.a = a;
				c.b = b;
				c.id_a = a->id;
				c.id_b = b->id;
				if (!collision_check(&c)) continue;

#ifdef DEBUG_PHYSICS
				printf("---- %d hit %d\n", i, j);
				printf("Normal: %f, %f. D: %f\n", c.normal.x, c.normal.y, c.overlap);
				printf("a: %f, %f\n", a->position.x, a->position.y);
				printf("b: %f, %f\n", b->position.x, b->position.y);
#endif

				bool is_ds = a->mass == 0 || b->mass == 0;

				if (is_ds) {
					move_ds(c);
					solve_ds(c);
				} else {
					move_dd(c);
					solve_dd(c);
				}

				c.selected_normal = c.normal;
				b->collisions.push_back(c);
				c.selected_normal = -c.normal;
				a->collisions.push_back(c);

				num_collisions++;
			}
		}
	}

	// printf("num_pot: %d\n    num: %d\nnum_checks: %d\n", 
	//		num_potential_collisions, num_collisions, 
	//		factorial(engine.bodies.size()) * 2);
}

