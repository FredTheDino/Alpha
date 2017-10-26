enum Component_Type {
	NO_TYPE,
};

struct Entity;

struct Component {
	Component();
	~Component();

	Component_Type type = NO_TYPE;
	Entity* parent = nullptr;

	// When added to a component.
	void (*add)(Component* self);
	// Called when the component is removed.
	void (*remove)(Component* self);

	// Called every frame.
	void (*update)(Component* self, float delta);
	// Called after update every frame.
	void (*late_update)(Component* self, float delta);
	// Called every fixed time-step.
	void (*fixed_update)(Component* self, float delta);
	// Called once per frame.
	void (*draw)(Component* self);
};

struct Entity_List {
	Array<Entity*> entities;
} entity_list;


struct Entity {
	Entity(Entity_List* e_list = &entity_list) {
		e_list->entities.push_back(this);
		list = e_list;
	}

	~Entity() {
		for (int i = 0; i < list->entities.size(); i++) {
			if (list->entities[i] == this) {
				list->entities.erase(list->entities.begin() + i);
				break;
			}
		}
	}

	Entity_List* list;
	Array<Component*> components;

	bool add_component(Component* c) {
		assert(c->parent == nullptr);
		c->parent = this;
		components.push_back(c);
		c->add(c);
		return true;
	}

	bool remove_components_of_type(Component_Type type) {
		Array<int> to_be_deleted;
		to_be_deleted.reserve(10);
		for (int i = 0; i < components.size(); i++) {
			if (type == components[i]->type) {
				to_be_deleted.push_back(type);
			}
		}

		if (to_be_deleted.size() == 0)
			return false;

		for (int i = to_be_deleted.size(); 0 < i; i--) {
			components[i]->remove(components[i]);
			components.erase(components.begin() + i);
		}
		return true;
	}
};

void update_list(Entity_List& list, float delta) {
	// can a macro make this shorter?
	for (int i = 0; i < list.entities.size(); i++) {
		Entity& e = *list.entities[i];
		for (int j = 0; j < e.components.size(); j++) {
			Component& c = *e.components[j];
			if (c.update) {
				c.update(&c, delta);
			}
		}
	}

	for (int i = 0; i < list.entities.size(); i++) {
		Entity& e = *list.entities[i];
		for (int j = 0; j < e.components.size(); j++) {
			Component& c = *e.components[j];
			if (c.late_update) {
				c.late_update(&c, delta);
			}
		}
	}
}

void draw_list(Entity_List& list) {
	// can a macro make this shorter?
	for (int i = 0; i < list.entities.size(); i++) {
		Entity& e = *list.entities[i];
		for (int j = 0; j < e.components.size(); j++) {
			Component& c = *e.components[j];
			if (c.draw) {
				c.draw(&c);
			}
		}
	}
}
