// Entity systems the easy way.
#include "entity.h"

bool inline isAlive(const EntityList& list, const EntityID id) {
	return list.uid[id.pos] == id.uid;
}

EntityID create_entity(EntityList& list) {
	EntityID id = {};
	if (list.next_free >= 0) {
		// No holes, so append.
		id.pos = list.next_free++;
	} else {
		id.pos = -list.next_free; // Make posetive.
		list.next_free = -list.uid[id.pos];
	}

	if (list.max_entity_pos < id.pos) {
		list.max_entity_pos = id.pos; // This is the one that is furthest along.
	}

	id.uid = ++list.curr_uid;
	list.uid[id.pos] = id.uid;
	printf("pos: %d, uid: %d\n", id.pos, id.uid);
	return id;
}


void remove_entity(EntityList& list, EntityID id) {
	if (!isAlive(list, id)) return; // Can't kill it twice!
	printf("deleted pos: %d, uid: %d\n", id.pos, id.uid);

	// Remove it.
	list.uid[id.pos] = list.next_free;
	list.next_free = -id.pos;
	
	// Check if it is the end one that is deleted
	if (id.pos == list.max_entity_pos) {
		// If it is, see if we can find the next max one.
		while (0 <= list.max_entity_pos && 0 < id.uid) {
			list.max_entity_pos--;
		}
	}
}

inline void add_component(EntityList& list, EntityID id, ComponentType type) {
	list.comp[id.pos][type] = true;
}

void* get_component(EntityList& list, EntityID id, ComponentType type) {
	switch (type) {
		case TRANSFORM:
			return &list.transform_c[id.pos];
		case BODY:
			return &list.body_c[id.pos];
		case NUM_COMPONENTS:
		default:
			return nullptr;
	}
}

inline EntityList::Transform* get_transform(EntityList& list, EntityID id) {
	return &list.transform_c[id.pos];
}

inline EntityList::Body* get_body(EntityList& list, EntityID id) {
	return &list.body_c[id.pos];
}

void update_systems(EntityList& list, float delta) {
	for (int i = 0; i < list.max_entity_pos; i++) {
		if (list.comp[i][TRANSFORM] && list.comp[i][BODY]) {
			// Fall system
			list.body_c[i].velocity = list.body_c[i].velocity + Vec2(0, delta);
			list.transform_c[i].position = list.transform_c[i].position + (list.body_c[i].velocity * delta);
			printf("(#%d) @%f, %f\n", i, list.transform_c[i].position.x, list.transform_c[i].position.y);
		}
	}
};

// // Assume the global list if none is specified.
// inline void add_component(EntityID id, ComponentType type) {
// 	entity_list.comp[id.pos][type] = true;
// }
// 
// EntityID inline create_entity() {
// 	return create_entity(entity_list);
// }
// 
// void inline remove_entity(EntityID id) {
// 	remove_entity(entity_list, id);
// }
// 
// inline EntityList::Transform& get_transform(EntityID id) {
// 	return entity_list.transform_c[id.pos];
// }

// Systems need to get updated when an entity is removed.
// Or just checked when they're updated. I could have a 
// list in the entity list with entities that have been
// removed this frame, and check that against the systems.
// 
// But systems need to be implemented in their unique way.
// And components could exist only in systems. Making them
// only into a struct and making the system to interface.
//
// This would require 0 OOP, and it gives me wet dreams 
// system architecture wise.
