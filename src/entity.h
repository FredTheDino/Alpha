#define MAX_NUM_ENTITIES 64
struct EntityID {
	short pos;
	// If a unique id is negative, the entity is dead.
	unsigned short uid;
};

enum ComponentType {
	TRANSFORM,
	BODY,

	NUM_COMPONENTS
};

struct EntityList {
	// Fixed number of entities,
	// might change.
	int  uid [MAX_NUM_ENTITIES] = {};
	bool comp[MAX_NUM_ENTITIES][NUM_COMPONENTS] = {}; // Flag is true if that component is used.
	unsigned short curr_uid = 0;
	int next_free = 0;
	int max_entity_pos = 0;

	struct Transform {
		Vec2 position;
		Vec2 scale;
		float rotation;
	};

	struct Body {
		Vec2 velocity;
	};

	Transform transform_c[MAX_NUM_ENTITIES] = {};
	Body body_c[MAX_NUM_ENTITIES] = {};

	// Transforms.
	struct TransformSystem {
	} transform_system;

} entity_list;

