#define MAX_NUM_ENTITIES 128
struct EntityID {
	short pos;
	// If a unique id is negative, the entity is dead.
	unsigned short uid;
};

enum ComponentType {
	TRANSFORM,
	BODY,
	SPRITE,

	NUM_COMPONENT_TYPES
};

enum SystemType {
	FALL_SYSTEM,
	SIMPLE_SPRITE_SYSTEM,

	NUM_SYSTEM_TYPES
};

struct EntityList {
	// Fixed number of entities,
	// might change.
	int  uid    [MAX_NUM_ENTITIES] = {};
	bool comp   [MAX_NUM_ENTITIES][NUM_COMPONENT_TYPES] = {};
	bool system [MAX_NUM_ENTITIES][NUM_SYSTEM_TYPES] = {};
	unsigned short curr_uid = 0;
	int next_free = 0;
	int max_entity_pos = 0;

	struct Transform {
		Vec2 position;
		Vec2 scale;
		float rotation;
	};
	Transform transform_c[MAX_NUM_ENTITIES] = {};

	struct Body {
		Vec2 velocity;
	};
	Body body_c[MAX_NUM_ENTITIES] = {};

	struct Sprite {
		Texture t;
	};
	Sprite sprite_c[MAX_NUM_ENTITIES] = {};

} entity_list;

