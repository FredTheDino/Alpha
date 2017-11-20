// If a unique id is negative, the entity is dead.
struct EntityID {
	short pos;
	unsigned short uid;
};

#define MAX_NUM_ENTITIES 512
struct EntityList {
	// Fixed number of entities,
	// might change.
	int uid[MAX_NUM_ENTITIES];
	unsigned short curr_uid = 0;
	int next_free = 0;
};

