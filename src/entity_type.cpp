struct Sprite {
	Shader* shader;
	Texture texture;
	float layer = 0;
	int sub_sprite = 0;
};

struct SpriteEntity {
	Sprite s;
	Transform t;
};

// Tweek this some more!

struct Player {
	Sprite s;
	Vec2 position;
	BodyID body;

	float run_acc = 20000;
	float run_slow_down = 15000;
	float air_acc = 1000;
	float air_slow_down = 500;

	float max_speed = 3000;
	int jump_count = 0;
	bool has_jumped = false;
	float first_jump_vel = 5.5;
	float second_jump_vel = 4.5;
	float gravity = 15;
	float hover = 5;

	int facing_direction = 0;

	float speed;
	float speed_delta;
	bool grounded;
};

//
// @SPRITE
//

Entity new_sprite_entity(Vec2 position, Vec2 scale, float rotation,
		Texture texture, int sub_sprite = 0, float layer = 0.0f) {
	SpriteEntity* se = new SpriteEntity();
	se->s.shader = &color_shader;
	se->s.texture = texture;
	se->s.sub_sprite = sub_sprite;
	se->s.layer = sub_sprite;

	se->t.position = position;
	se->t.scale = scale;
	se->t.rotation = rotation;

	Entity e;
	e.data = se;
	e.type = SPRITE_ENTITY;

	e.update = [](Entity* e, float delta) {};

	e.draw   = [](Entity* e, float t) {
		SpriteEntity& se = *((SpriteEntity*) e->data);

		draw_sprite(*se.s.shader, se.s.texture,
				se.s.sub_sprite, se.t.position,
				se.t.scale, se.t.rotation,
				{1, 1, 1, 1}, se.s.layer);
	};

	e.clear  = [](Entity* e) {
		delete (SpriteEntity*) e->data;
	};

	return e;
}

//
// @PLAYER
//


Entity new_player(Vec2 position, Shape* shape, Texture texture) {
	Player* p = new Player;

	p->s.shader = &color_shader;
	p->s.texture = texture;
	p->position = position;
	Body body;
	body.position = position;
	p->body = add_body(engine, body, shape);

	Entity e;
	e.data = p;
	e.type = PLAYER_ENTITY;

	e.update = [](Entity* e, float delta) {
		Player& p = *(Player*) e->data;
		Body& b = *find_body(engine, p.body);

		bool grounded = false;
		bool is_dynamic = false;
		Vec2 upp_vec;
		Vec2 forward_vec = Vec2(1, 0);
		for (auto c : b.collisions) {
			if (c.is_trigger) continue;
			if (dot(c.selected_normal, Vec2(0, 1)) > 0.1f) {
				grounded = true;
				p.jump_count = 0;
				upp_vec = c.selected_normal;
				forward_vec = Vec2(upp_vec.y);

				if (c.id_b.pos == b.id.pos) {
					is_dynamic = c.a->mass != 0;
				} else {
					is_dynamic = c.b->mass != 0;
				}

				break;
			}
		}
		

		float acc = p.run_acc;
		float slow_down = p.run_slow_down;
		if (!grounded) {
			acc = p.air_acc;
			slow_down = p.air_slow_down;
		}

		float dir = value("right") - value("left");
		if (dir) {
			p.speed += delta * acc * dir;
			p.facing_direction = sign(dir);
		} else {
			p.speed -= sign(p.speed) * slow_down * delta;
		}


		// Water calculations
		bool in_water = false;
		Vec2 water_force;
		for (auto c : b.collisions) {
			if (c.tag & 1) { // 1 is water 
				if (c.overlap > 0.5f) {
					in_water = true;
				} else {
					Vec2 up = c.selected_normal;
					Vec2 forward = Vec2(-up.y, up.x);

					float vertical_overlap = dot(b.velocity, up);
					float horizontal_overlap = dot(b.velocity, forward);

					float slope = vertical_overlap / horizontal_overlap;
					float vel = length_sq(b.velocity);

					if ((0 < slope || slope < -1.0f) && vel > 10) {
						continue;
					}

					float magic_number = 0.30;
					float d = horizontal_overlap * magic_number / delta;
					p.speed -= d * sign(p.speed);

					up = up * vertical_overlap * -2;
					water_force = up;
				}
			}
		}

		p.speed = clamp(p.speed, -p.max_speed, p.max_speed);
		b.velocity.x = forward_vec.x * p.speed * delta;
		b.velocity.y -= p.gravity * delta;

		p.speed_delta = p.speed * delta;
		b.velocity = b.velocity + water_force;

		//
		// @Cleanup, restructure this so this update call is cleaner.
		// And add in some floating logic, maybe a bouancy factor and 
		// storeing how emmersed the body is in water. 
		//
		// player_jump_logic(p, delta);

		bool jumped = false;
		if (p.jump_count < 2 && pressed("jump")) {
			jumped = true;
			p.jump_count++;
			if (p.jump_count == 1) {
				b.velocity.y = p.first_jump_vel;
			} else {
				b.velocity.y = p.second_jump_vel;
			}
			p.has_jumped = true;
			grounded = false;
		}

		if (!grounded && is_down("jump") && b.velocity.y > 0 && p.has_jumped) {
			b.velocity.y += p.hover * delta;
			jumped = true;
		} else {
			p.has_jumped = false;
		}

		if (grounded && !jumped && !is_dynamic) {
			b.velocity = b.velocity - upp_vec * 1000 * delta;
		}

		// Needed for rendering.
		p.position = b.position;
		p.grounded = grounded;
	};

	e.draw   = [](Entity* e, float t) {
		Player* p = (Player*) e->data;

		char speed[10];
		sprintf(speed, "%0.2f", p->speed_delta);
		char grounded[10];
		sprintf(grounded, "g:%i", p->grounded);
		auto m = new_text_mesh(droid_sans, speed);
		draw_text(*p->s.shader, droid_sans, m, main_camera.position + Vec2(0, 5.5), {3, 3});
		delete_mesh(m);

		m = new_text_mesh(droid_sans, grounded);
		draw_text(*p->s.shader, droid_sans, m, main_camera.position + Vec2(1, 5.5), {3, 3});
		delete_mesh(m);

		draw_sprite(*p->s.shader, p->s.texture,
				p->s.sub_sprite, p->position,
				{0.5, 0.5}, 0,
				{1, 1, 1, 1}, p->s.layer);
	};

	e.clear  = [](Entity* e) {
		delete (Player*) e->data;
	};

	return e;
}

//
// @BODY
//

Entity new_body_entity(Vec2 position, bool is_trigger, float mass, int8_t tag, Shape* shape) {
	Body b;
	b.position   = position;
	b.mass       = mass;
	b.is_trigger = is_trigger;
	b.tag        = tag;

	BodyID* id = new BodyID(add_body(engine, b, shape));

	Entity e;
	e.data = id;
	e.type = BODY_ENTITY;

	e.clear  = [](Entity* e) {
		delete (BodyID*) e->data;
	};
	return e;
}

Entity new_camera_controller() {
	Entity e;
	e.type = CAMERA_CONTROLLER_ENTITY;

	e.update = [](Entity* e, float delta) {
		static int last_uid = 0;
		Entity* player = find_entity(entity_list, "player");
		if (player == nullptr) {
			return;
		}

		Body* body = find_body(engine, ((Player*) player->data)->body);
		if (body == nullptr)
			return;

		Vec2 delta_pos = ((Player*) player->data)->position - main_camera.position;
		main_camera.position = main_camera.position + delta_pos * delta;
	};

	return e;
}
