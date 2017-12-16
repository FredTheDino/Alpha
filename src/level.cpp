Array<Vec2> load_level_points(String line) {
	auto split_line = split(line, ',', ';', ' ');

	int size = split_line.size() / 2;
	Array<Vec2> points(size);

	for (int i = 0; i < points.size(); i++) {
		if (split_line.size() < i * 2 + 1) break;
		points[i] = Vec2(stof(split_line[i * 2]), stof(split_line[i * 2 + 1]));
	}

	return points;
}

Texture load_level_texture(String line) {
	auto split_line = split(line, ',', ';', ' ');

	switch(split_line.size()) {
		default:
			return Texture();
		case 1:
			return new_texture(split_line[0]);
		case 2:
			return new_texture(split_line[0], stoi(split_line[1]));
		case 3:
			return new_texture(split_line[0], stoi(split_line[1]),
					stoi(split_line[2]));
		case 4:
			return new_texture(split_line[0], stoi(split_line[1]),
					stoi(split_line[2]), stoi(split_line[3]));
	}
}

Entity load_level_entity(Level& l, String type, String line) {
	auto sl = split(line, ',', ';', ' ');
	
	if (type == "PLAYER") {
		if (sl.size() < 4) 
			return Entity();
		return new_player(
				Vec2(stof(sl[0]), stof(sl[1])),
				l.shapes[sl[2]],
				&color_shader,
				&l.textures[sl[3]]
				);

	} else if (type == "SPRITE") {
		if (sl.size() < 7) 
			return Entity();
		return new_sprite_entity(
				Vec2(stof(sl[0]), stof(sl[1])),
				Vec2(stof(sl[2]), stof(sl[3])),
				stof(sl[4]),
				&color_shader,
				&l.textures[sl[5]],
				stoi(sl[6])
				);
	} else if (type == "BODY") {
		if (sl.size() < 4) 
			return Entity();
		return new_body_entity(
				Vec2(stof(sl[0]), stof(sl[1])),
				stof(sl[2]),
				l.shapes[sl[3]]
				);

	}
	
	return Entity();
}

void clear_level(Level& l) {
	clear(*l.engine);
	clear(*l.list);

	for (auto it : l.shapes) {
		delete it.second;
	}

	for (auto it : l.textures) {
		delete_texture(it.second);
	}
}


bool load_level(Level& l) {
	FILE* file = fopen((l.path + ".lvl").c_str(), "r");
	if (!file) {
		printf("Failed to load level \"%s.lvl\", no file found.\n", l.path.c_str());
		return false;
	}

	Array<String> split_line;
	size_t line_size;
	char* line;
	while (!feof(file)) {
		// Reading lines!
		line_size = 0;
		line = nullptr;

		getline(&line, &line_size, file);
		if (line[0] == '\n') continue;

		split_line.clear();
		split(String(line), split_line, ':');

		switch (split_line[0][0]) {
			case 's': {
					auto points = load_level_points(split_line[2]);
					Shape* s = new Shape(engine, points);
					l.shapes[split_line[1]] = s;
					break;
				}
			case 't': {
					Texture t = load_level_texture(split_line[2]);
					l.textures[split_line[1]] = t;
					break;
				}
			case 'e': {
					auto fields = split(split_line[1]);
					Entity e = load_level_entity(l, fields[0], split_line[2]);

					if (fields.size() == 1) {
						add_entity(*l.list, e);
					} else {
						add_entity(*l.list, fields[1], e);
					}
					break;
				}
			case '#':
			default:
					  break;
		}
	}
	return true;
}

void reload_level(Level& l) {
	clear_level(l);
	load_level(l);
}

Level load_level(EntityList& list, 
		PhysicsEngine& engine, String path) {

	Level l(&list, &engine);
	l.path = path;

	load_level(l);

	return l;

}
