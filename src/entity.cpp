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
		list.next_free = -list.entities[id.pos].id.uid;
	}

	id.uid = list.uid_gen++;

	e.id.pos = id.pos;
	e.id.uid = id.uid;

	entity_list.entities[id.pos] = Entity(e);

	return id;
}

EntityID add_entity(EntityList& list, String name, Entity e) {
	EntityID id;
	id.pos = -1;
	id.uid = -1;

	e.name = name;
	id = add_entity(list, e);
	list.name_to_id[name] = id;

	printf("Added entity %s@%d:%d\n", name.c_str(), id.pos, id.uid);
	return id;
}

Entity* find_entity(EntityList& list, EntityID id) {
	if (id.pos < 0) 
		return nullptr;
	if (list.entities.size() < id.pos) 
		return nullptr;

	const Entity& e = list.entities[id.pos];
	if (e.id.uid != id.uid)
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
	e->id.uid   = -id.uid;
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
		e.id.uid = -list.next_free;
		list.next_free = e.id.pos;
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
			e.id.uid = -list.next_free;
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
