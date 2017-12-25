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

	float run_acc = 7000;
	float run_slow_down = 10000;
	float air_acc = 4000;
	float air_slow_down = 2000;

	float max_speed = 1750;
	float jump_vel = 4;
	float gravity = 9;
	float hover = 0.3f;

	int facing_direction = 0;
	int last_moved_direction = 1;

	float speed;
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
			if (dot(c.selected_normal, Vec2(0, 1)) > 0.1f) {
				grounded = true;
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

		if (grounded) {
			if (is_down("right")) {
				p.last_moved_direction = 1;
				p.facing_direction = 1;
				p.speed += delta * p.run_acc * (value("right"));
			} else if (is_down("left")) {
				p.last_moved_direction = -1;
				p.facing_direction = -1;
				p.speed -= delta * p.run_acc * (value("left"));
			} else {
				p.speed -= sign(p.speed) * p.run_slow_down * delta;
				p.facing_direction = 0;
			}
		} else {
			if (is_down("right")) {
				p.last_moved_direction = 1;
				p.facing_direction = 1;
				p.speed += delta * p.air_acc * (value("right"));
			} else if (is_down("left")) {
				p.last_moved_direction = -1;
				p.facing_direction = -1;
				p.speed -= delta * p.air_acc * (value("left"));
			} else {
				p.speed -= sign(p.speed) * p.air_slow_down * 1.5f * delta;
				p.facing_direction = 0;
			}
			
		}

		p.speed = clamp(p.speed, -p.max_speed, p.max_speed);
		b.velocity.x = forward_vec.x *  p.speed * delta;
		b.velocity.y -= p.gravity * delta;

		bool jumped = false;
		if (grounded && pressed("jump")) {
			b.velocity.y = p.jump_vel;
			jumped = true;
		}

		if (!grounded && is_down("jump") && b.velocity.y > 0) {
			b.velocity.y += p.gravity * p.hover * delta;
			jumped = true;
		}

		if (grounded && !jumped && !is_dynamic) {
			b.velocity = b.velocity - upp_vec * 1000 * delta;
		}



		// Needed for rendering.
		p.position = b.position;
	};

	e.draw   = [](Entity* e, float t) {
		Player* p = (Player*) e->data;

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

Entity new_body_entity(Vec2 position, float mass, Shape* shape) {
	Body b;
	b.position = position;
	b.mass = mass;
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
