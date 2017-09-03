// Entity-Component-System, the easy way.
// Maybe move this to header file?

enum System_Type {
	TEST, // Really, really bad name.
	NUM_SYSTEM_TYPES,
};

// Woo forward declariton
/*
struct System;

struct System {

	System(System_Type type, void (*update)(float), void (*draw)()) {
		assert(0 <= type);
		assert(type < NUM_SYSTEM_TYPES);

		size_t size = all_sys.size();
		if (size < NUM_SYSTEM_TYPES) {
			all_sys.resize(NUM_SYSTEM_TYPES);
		}
	
		this->type = type;
		this->update = update;
		this->draw = draw;
	}

	System_Type type;

	void (*update)(float delta);
	void (*draw)();

	// Don't know if this should be here....
	// Array<void*> components;
};
*/

// Entity stuff...

struct Component_Pointer {
	int id;
	System_Type type;

	bool operator== (const Component_Pointer& other) const {
		return id == other.id 
			&& type == other.type;
	}
};

struct Entity_ID {
	int id;
	int u_id;

	bool operator== (const Entity_ID& other) const {
		return id == other.id && u_id == other.u_id;
	}
};

struct Entity {
	Entity(int flags=0) {
		this->flags = flags;
	}

	Entity_ID id;
	int flags;
	Array<Component_Pointer> components;

	bool operator== (const Entity& other) const {
		return id == id;
	}
};

struct Entity_List {
	Stack<int> free_pos;
	Array<Entity> list;
};

Entity* get(Entity_List& el, Entity_ID id) {
	Entity* other = &el.list[id.id];
	if (other->id == id) {
		return other;
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
	Entity_ID& id = el.list[pos].id;
	id.id = pos;
	do {
		id.u_id += 1;
	} while (id.u_id == 0);

	return id;
}

bool remove_entity(Entity_List& el, Entity_ID id) {
	Entity* e = get(el, id);
	if (!e) return false;
	if (e->id.id == -1) return false;
	// So we know there's nothing here.
	e->id.id = -1;
	
	// All done.
	el.free_pos.push(id.id);
	return true;
}

namespace file {
	void test_update(float delta);
	void test_draw();
}

void* systems[System_Type::NUM_SYSTEM_TYPES];

struct Component {
	int id;
	System_Type type;
	int next_free = -1;
};

struct Test_Component {
	Component c;
	float data;
};

void update(float delta) {
	printf("Updating!\n");
}

void add(void* _c) {
	Test_Component* c    = (Test_Component*) _c;
	Component*      comp = (Component*) _c; 
	if (next_free = -1) {
		test.components.push_back(*c);
	} else {
		int id = 0;
	}
}

struct Test_System {
	Test_System() {
		systems[System_Type::TEST] = this;
	}

	~Test_System() {
		systems[System_Type::TEST] = nullptr;
	}

	// Note, this has to be first, this is so they share pointer.
	Array<Test_Component> components;
	int next_free = -1;

	void (*update)(float) = update;

	void (*add)(void*);
} test;

Component_Pointer make_test_component(float data) {
	Component_Pointer ptr;
	ptr.type = System_Type::TEST;
	ptr.component_id = test.components.size();

	Test_Component c;
	c.data = data;
	test.components.push_back(c);

	return ptr;
}

namespace file {
	void test_update(float delta) {
		printf("Update!\n");
		for (auto c : test.components) {
			printf("c.data: %f\n", c.data);
		}
	}

	void test_draw() {
	}
}

void add_component(Entity_List& el, Entity_ID id, Component_Pointer ptr) {
	auto e = get(el, id);
	e->components.push_back(ptr);
}

bool remove_components_of_type(Entity_List& el, Entity_ID id, System_Type type) {
	bool erased = false;
	Entity& e = *get(el, id);
	for (int i = 0; i < e.components.size(); i++) {
		if (e.components[i].type == type) {
			e.components.erase(e.components.begin() + i);
			erased = true;
		}
	}
	return erased;

}

bool remove_component(Entity_List& el, Entity_ID id, Component_Pointer ptr) {
	bool erased = false;
	Entity& e = *get(el, id);
	for (int i = 0; i < e.components.size(); i++) {
		if (e.components[i] == ptr) {
			e.components.erase(e.components.begin() + i);
			erased = true;
		}
	}
	return erased;
}

