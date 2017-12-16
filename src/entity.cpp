
//
// Entity System
//

EntityID add_entity(EntityList& list, Entity e) {
	EntityID id = {};
	// Set some meta info
	e.alive   = true;
	e.cleared = false;

	if (list.next_free == -1) {
		// Add a new one
		id.pos = list.entities.size();
		list.entities.push_back(e);
	} else {
		// Use an old one
		id.pos = list.next_free;
		// Update the next free.
		list.next_free = -list.entities[id.pos].uid;
	}

	id.uid = list.uid_gen++;

	e.pos = id.pos;
	e.uid = id.uid;

	entity_list.entities[id.pos] = Entity(e);

	return id;
}

EntityID add_entity(EntityList& list, String name, Entity e, bool replace = true) {
	EntityID id;
	id.pos = -1;
	id.uid = -1;

	if (!replace) {
		EntityID id = list.name_to_id[name];
		const Entity e = list.entities[id.pos];
		if (!e.alive || !e.cleared) 
			return id;
	}

	e.name = name;
	id = add_entity(list, e);
	list.name_to_id[name] = id;
	return id;
}

Entity* find_entity(EntityList& list, EntityID id) {
	if (id.pos < 0) 
		return nullptr;
	if (list.entities.size() < id.pos) 
		return nullptr;

	const Entity& e = list.entities[id.pos];
	if (e.uid != id.uid)
		return nullptr;

	if (e.alive)
		return &list.entities[id.pos];

	return nullptr;
}

Entity* find_entity(EntityList& list, String name) {
	try {
		return find_entity(list, list.name_to_id[name]);
	} catch (std::out_of_range) {
		return nullptr;
	}
}

bool remove_entity(EntityList& list, EntityID id) {
	Entity* e = find_entity(list, id);
	if (e == nullptr)
		return false;

	e->alive = false;
	e->uid   = -id.uid;
	list.dead.push_back(id.pos);

	if (e->name != "")
		list.name_to_id.erase(e->name);

	return true;
}

bool remove_entity(EntityList& list, String name) {
	return remove_entity(list, list.name_to_id[name]);
}

void clear(EntityList& list) {
	for (Entity& e : list.entities) {
		e.alive = false;
		if (e.cleared) continue;
		if (e.clear)
			e.clear(&e);
		
		e.cleared = true;
		e.uid = -list.next_free;
		list.next_free = e.pos;
	}
}

void update_entities(EntityList& list, float delta) {
	for (int i = 0; i < list.entities.size(); i++) {
		Entity& e = list.entities[i];
		if (!e.alive) continue;
		if (!e.update) continue;
		e.update(&e, delta);
	}
}

void draw_entities(EntityList& list, float t) {
	for (int i = 0; i < list.entities.size(); i++) {
		Entity& e = list.entities[i];
		if (!e.alive) continue;
		if (!e.draw) continue;
		e.draw(&e, t);
	}
}

void gc_entities(EntityList& list) {
	if (list.dead.size() != 0) {
		for (int i = 0; i < list.dead.size(); i++) {
			int pos = list.dead[i];
			Entity& e = list.entities[pos];

			if (e.clear)
				e.clear(&e);

			e.cleared = true;
			e.uid = -list.next_free;
			list.next_free = pos;
		}
		list.dead.clear();
	}
} 

EntityList::~EntityList() {
	for (int i = 0; i < entities.size(); i++) {
		Entity& e = entities[i];
		if (!e.cleared && e.clear)
			e.clear(&e);
	}
}

//
// Entity Creation
//

struct Sprite {
	Shader* shader;
	Texture* texture;
	float layer = 0;
	int sub_sprite = 0;
};

struct SpriteEntity {
	Sprite s;
	Transform t;
};


Entity new_sprite_entity(Vec2 position, Vec2 scale, float rotation, 
		Shader* shader, Texture* texture, int sub_sprite = 0, float layer = 0.0f) {
	SpriteEntity* se = new SpriteEntity();
	se->s.shader = shader;
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

		draw_sprite(*se.s.shader, *se.s.texture, 
				se.s.sub_sprite, se.t.position, 
				se.t.scale, se.t.rotation, 
				{1, 1, 1, 1}, se.s.layer);
	};

	e.clear  = [](Entity* e) {
		delete (SpriteEntity*) e->data;
	};

	return e;
}

struct Player {
	Sprite s;
	Vec2 position;
	BodyID body;

	float speed = 7;
	float jump_vel = 1;
};

Entity new_player(Vec2 position, Shape* shape, Shader* shader, Texture* texture) {
	Player* p = new Player;
	
	p->s.shader = shader;
	p->s.texture = texture;
	p->position = position;
	Body body;
	body.position = position;
	p->body = add_body(engine, body, shape);

	Entity e;
	e.data = p;
	e.type = PLAYER_ENTITY;

	e.update = [](Entity* e, float delta) {
		Player* p = (Player*) e->data;
		Body* b = find_body(engine, p->body);

		b->velocity.x += delta * p->speed * (value("right") - value("left"));
		b->velocity.y += delta * p->speed * (value("up") - value("down"));
		
		bool grounded = false;
		for (auto c : b->collisions) {
			if (dot(c.selected_normal, Vec2(0, 1)) > 0.75f) {
				grounded = true;
				break;
			}
		}

		if (grounded && pressed("jump")) {
			b->velocity.y = p->jump_vel;
		}

		p->position = b->position;
	};

	e.draw   = [](Entity* e, float t) {
		Player* p = (Player*) e->data;

		draw_sprite(*p->s.shader, *p->s.texture, 
				p->s.sub_sprite, p->position, 
				{0, 0}, 0, 
				{1, 1, 1, 1}, p->s.layer);
	};

	e.clear  = [](Entity* e) {
		delete (Player*) e->data;
	};

	return e;
}

Entity new_body_entity(Vec2 position, float mass, Shape* shape) {
	Body b;
	b.position = position;
	b.mass = mass;
	BodyID* id = new BodyID(add_body(engine, b, shape));

	Entity e;
	e.data = id;
	e.type = BODY_ENTITY;

	e.update = [](Entity* e, float delta) {
	};

	e.draw   = [](Entity* e, float t) {
	};

	e.clear  = [](Entity* e) {
		delete (BodyID*) e->data;
	};
	return e;
}



