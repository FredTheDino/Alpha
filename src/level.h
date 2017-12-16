struct Level {
	Level(EntityList* e, PhysicsEngine* p) : list(e), engine(p) {};
	~Level() {
		for (auto it : shapes) {
			delete it.second;
		}

		for (auto it : textures) {
			delete_texture(it.second);
		}
	}

	EntityList* list;
	PhysicsEngine* engine;
	String path;

	HashMap<String, Shape*> shapes;
	HashMap<String, Texture> textures;
};
