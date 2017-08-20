// Entity-Component-System, the easy way.
// Maybe move this to header file?

enum System_Type {
	NONE,
	NUM_SYSTEM_TYPES,
};

// Woo forward declariton
struct System;

// The list of systems.
typedef Array<System*> All_Systems;
All_Systems all_systems;

struct System {
	System(String name, System_Type type) {
		assert(0 < type);
		assert(type < NUM_SYSTEM_TYPES)

		size_t size = systems.size();
		if (size < NUM_SYSTEM_TYPES) {
			system.reserve(NUM_SYSTEM_TYPES);
		}
	
		this->name = name;
		this->type = type;
	
		systems[type] = this;
	}

	String name;
	System_Type type;

	Array<void*> components;

	void (update*)(float delta);
	void (draw*)();
};

void update(All_Systems all_sys, float delta) {
	auto size = all_sys.size();
	for (int i = 0; i < size; i++) {
		auto& sys = all_sys[i];
		sys.update(delta);
	}
}

void draw(All_Systems all_sys) {
	auto size = all_sys.size();
	for (int i = 0; i < size; i++) {
		auto& sys = all_sys[i];
		sys.draw(delta);
	}
}

// Entity stuff...

struct Component {
	System_Type type;
};

struct Component_Pointer {
	int component_id;
	System_Type type;

	bool operator== (const Component_Pointer& other) const {
		return component_id == other.component_id 
			&& type == other.type;
	}
};

struct Entity_ID {
	int position;
	int unique_id;

	bool operator== (const Entity_ID& other) const {
		return position == other.position && unique_id == other.unique_id;
	}
};

struct Entity {
	Entity_ID id;
	Array<Component_Pointer> components;

	bool operator== (const Entity& other) const {
		return id == id;
	}
};

struct Entity_List {
	Stack<int> free_pos;
	Array<Entity> list;
}

Entity* get(Entity_List& el, Entity_ID id) {
	auto other = el.list[id.position];
	if (other.id == id) {
		return &other;
	}
	return nullptr;
}

Entity_ID new_entity(Entity_List& el) {
	int pos = -1;
	if (el.free_pos.size() == 0) {
		// We need to make more space.
		pos = el.list.size();
		el.list.push_back(Entity());
	} else {
		// Just slot it in.
		pos = el.free_pos.top();
		el.free_pos.pop();
	}

	el.list[pos] = Entity();
	Entity_ID& id;
	id = el.list[pos].id;
	id.position = pos;
	do {
		id.unique_id += 1;
	} while (id.unique_id == 0);
	return id;
}

bool remove_entity(Entity_List& el, Entity* e)

void add_component(Entity_ID id, Component_Pointer ptr) {
	e.push_back(ptr);
}

bool remove_component(Entity_ID id, Component_Pointer ptr) {
	bool erased = false;
	for (auto it : e->begin()) {
		if (it == ptr) {
			e->erase(it);
			erased = true;
		}
	}
	return erased;
}

bool remove_component(Entity_ID id, System_Type type) {
	bool erased = false;
	for (auto it : e->begin()) {
		if (it.type == type) {
			e->erase(it);
			erased = true;
		}
	}
	return erased;
}

