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

	float run_acceleration = 500;
	float max_speed = 500;
	float jump_vel = 1;

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

		if (is_down("right")) {
			p.last_moved_direction = 1;
			p.facing_direction = 1;
			p.speed += delta * p.run_acceleration * (value("right"));
		} else if (is_down("left")) {
			p.last_moved_direction = -1;
			p.facing_direction = -1;
			p.speed += delta * p.run_acceleration * (value("left"));
		} else {
			p.speed -= p.run_acceleration * 1.5f * delta;
			p.facing_direction = 0;
		}

		p.speed = clamp(p.speed, 0, p.max_speed);
		printf("Speed : %f\n", p.speed);
		b.velocity.x = p.speed * p.last_moved_direction * delta;
		
		bool grounded = false;
		for (auto c : b.collisions) {
			if (dot(c.selected_normal, Vec2(0, 1)) > 0.75f) {
				grounded = true;
				break;
			}
		}

		if (grounded && pressed("jump")) {
			b.velocity.y = p.jump_vel;
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

		Vec2 delta_pos = body->velocity * 1.1f + ((Player*) player->data)->position - 
			main_camera.position;
		main_camera.position = main_camera.position + delta_pos * delta;
	};

	return e;
}
