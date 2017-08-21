// Entity-Component-System, the easy way.
// Maybe move this to header file?

enum System_Type {
	TEST, // Really, really bad name.
	NUM_SYSTEM_TYPES,
};

// Woo forward declariton
struct System;

// The list of systems.
typedef Array<System*> All_Systems;
All_Systems all_systems;

struct System {

	System(All_Systems& all_sys, System_Type type, void (*update)(float), void (*draw)()) {
		assert(0 <= type);
		assert(type < NUM_SYSTEM_TYPES);

		size_t size = all_sys.size();
		if (size < NUM_SYSTEM_TYPES) {
			all_sys.resize(NUM_SYSTEM_TYPES);
		}
	
		this->type = type;
		this->update = update;
		this->draw = draw;
	
		all_sys[type] = this;
	}

	System_Type type;

	void (*update)(float delta);
	void (*draw)();

	// Don't know if this should be here....
	// Array<void*> components;
};

void update_system(All_Systems all_sys, float delta) {
	auto size = all_sys.size();
	for (int i = 0; i < size; i++) {
		auto sys = all_sys[i];
		sys->update(delta);
	}
}

void draw_system(All_Systems all_sys) {
	auto size = all_sys.size();
	for (int i = 0; i < size; i++) {
		auto sys = all_sys[i];
		sys->draw();
	}
}

// Entity stuff...

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
	Entity* other = &el.list[id.position];
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
	id.position = pos;
	do {
		id.unique_id += 1;
	} while (id.unique_id == 0);

	return id;
}

bool remove_entity(Entity_List& el, Entity_ID id) {
	Entity* e = get(el, id);
	if (!e) return false;
	if (e->id.position == -1) return false;
	// So we know there's nothing here.
	e->id.position = -1;
	
	// All done.
	el.free_pos.push(id.position);
	return true;
}

namespace file {
	void test_update(float delta);
	void test_draw();
}

struct Test_Component;

struct Test {
	Test() : s(all_systems, TEST, file::test_update, file::test_draw) {}
	// Note, this has to be first, this is so they share pointer.
	System s;
	Array<Test_Component> components;
} test;
// A way to do polymorphism without using C++ inheritance.
System* test_system = (System*)(void*)&test;

struct Test_Component {
	float data;
};

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

