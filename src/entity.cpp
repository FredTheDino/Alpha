// Entity systems the easy way.
#include "entity.h"

EntityID create_entity(EntityList& list) {
	EntityID id = {};
	if (list.next_free >= 0) {
		// No holes, so append.
		// (Make sure these work like they should.)
		id.pos = list.next_free++;
	} else {
		id.pos = -list.next_free; // Make it posetive
		list.next_free = -list.uid[id.pos];
	}
	id.uid = ++list.curr_uid;
	list.uid[id.pos] = id.uid;
	return id;
}

void remove_entity(EntityList& list, EntityID id) {
	if (!isAlive(list, id)) return; // Can't kill it twice.
	list.uid[id.pos] = list.next_free;
	list.next_free = -id.pos;
}

bool inline isAlive(EntityList& list, EntityID id) const {
	return list.uid[id.pos] == id.uid;
}

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
