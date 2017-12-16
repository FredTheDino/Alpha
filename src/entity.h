enum EntityType {
	NO_TYPE,
	SPRITE_ENTITY,
	PLAYER_ENTITY,
	BODY_ENTITY,
};

struct EntityID {
	int pos;
	int uid;
};

// Simular to physics.[h/cpp]

struct Entity {
	Entity() {}
	Entity(EntityType type, void* data, void (*update)(Entity*, float), 
			void (*draw)(Entity*, float), void (*clear)(Entity*)) {
		this->type = type;
		this->data = data;
		this->update = update;
		this->draw = draw;
		this->clear = clear;
	}

	// Meta
	int pos;
	int uid = -1;
	bool cleared = true;
	bool alive   = false;
	String name  = "";

	EntityType type = EntityType::NO_TYPE;
		
	void (*update)(Entity*, float) = 0;
	void (*draw)(Entity*, float) = 0;
	void (*clear)(Entity*) = 0;
	void* data = 0;
};

struct EntityList {
	~EntityList();

	HashMap<String, EntityID> name_to_id;
	Array<Entity> entities;
	int next_free = -1;
	unsigned short uid_gen = 0;
	
	Array<int> dead;
} entity_list;
