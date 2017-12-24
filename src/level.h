struct Level {
	Level(EntityList* e, PhysicsEngine* p) : list(e), engine(p) {};

	EntityList* list;
	PhysicsEngine* engine;
	String path;

	HashMap<String, Shape*> shapes;
	HashMap<String, Texture> textures;
};
