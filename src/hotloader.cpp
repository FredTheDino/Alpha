//
// This keeps tracks of assets so they can be updated
// in the game if they are changed on disk.
//

enum ASSET_TYPES {
	SHADER,
	IMAGE,
	INPUT_MAP,
	AUDIO,
};

struct HotLoadableAsset {
	void* asset;
	ASSET_TYPES type;

	String path;
	short timer = 0;
	long last_edit_time;
};

struct HotLoader {
	Array<HotLoadableAsset> assets;
	short delay = 10;
} hot_loader;

void register_asset(HotLoader& loader, HotLoadableAsset asset) {
	loader.assets.push_back(asset);
}

// Dependant on the platform layer.
unsigned long get_edit_time(const char* path, bool silent=false);

void update_loader(HotLoader& loader) {
	struct stat attrib;
	for (auto& asset : loader.assets) {
		//HotLoadableAsset& asset = loader.assets[i];
		unsigned long last_edit_time = get_edit_time(asset.path.c_str(), true);
		if (asset.last_edit_time == last_edit_time) continue;
		if (asset.timer != loader.delay) {
			asset.timer++;
			continue;
		}
		
		// Reset the timer.
		asset.last_edit_time = last_edit_time;
		asset.timer = 0;

		// Reload the actual assets.
		if (asset.type == ASSET_TYPES::SHADER) {
			Shader* shader = (Shader*) asset.asset;
			Shader temp_shader = new_shader(asset.path, shader->name);
			if (temp_shader.program == -1) {
				printf("[Hotloader.cpp] Failed to reload shader (%s)\n", shader->name.c_str());
			} else {
				delete_shader(*shader);
				shader->program = temp_shader.program;
				printf("[HotLoader.cpp] Shader reloaded (%s)\n", shader->name.c_str());
			}
		} else if (asset.type == ASSET_TYPES::IMAGE) {
			Texture* texture = (Texture*) asset.asset;
			bool success = update_texture(*texture, asset.path);
			if (success) {
				printf("[Hotloader.cpp] Image reloaded (%s)\n", asset.path.c_str());
			} else {
				asset.last_edit_time -= 10;
			}
		} else if (asset.type == ASSET_TYPES::INPUT_MAP) {
			InputMap* map = (InputMap*) asset.asset;
			InputMap new_map;
			if (parse_input_file(new_map, asset.path)) {
				map->actions.clear();
				map->inputs.clear();
				*map = new_map;
				printf("[Hotloader.cpp] Input reloaded (%s)\n", asset.path.c_str());
			} else {
				printf("[Hotloader.cpp] Failed to reload input file\n");
			}
		} else if (asset.type == ASSET_TYPES::AUDIO) {
			auto sound = (Sound*) asset.asset;
			bool success = update_sound(*sound, asset.path);
			if (success) {
				printf("[Hotloader.cpp] Sound reloaded (%s)\n", asset.path.c_str());
			} else {
				asset.last_edit_time -= 10;
			}
		} else {
			printf("[HotLoader.cpp] Trying to update unsupported asset type.\n");
		}
	}
}

void register_hotloadable_asset(HotLoader& loader, Shader* _asset, String path, String name) {
	HotLoadableAsset asset;

	asset.asset = (void*) _asset;
	asset.path = path;
	asset.type  = ASSET_TYPES::SHADER;
	asset.last_edit_time = get_edit_time(path.c_str());

	*_asset = new_shader(path, name);

	register_asset(loader, asset);
}

Shader::Shader(String path, String name) {
	register_hotloadable_asset(hot_loader, this, path, name);
}


void register_hotloadable_asset(HotLoader& loader, 
		Texture* _asset, String path, 
		bool linear_filtering = true, 
		int sprites_x = 1, int sprites_y = 1, 
		bool use_mipmaps = false) {
	String file_path = find_texture_file(path);

	HotLoadableAsset asset;
	asset.asset = (void*) _asset;
	asset.path = file_path;
	asset.type  = ASSET_TYPES::IMAGE;
	asset.last_edit_time = get_edit_time(file_path.c_str());

	*_asset = new_texture(path, linear_filtering, sprites_x, sprites_y, use_mipmaps);

	register_asset(loader, asset);
}

Texture::Texture(String path, int sx, int sy) {
	register_hotloadable_asset(hot_loader, this, path, true, sx, sy, false);
}

void register_hotloadable_asset(HotLoader& loader, InputMap* map, String path) {
	HotLoadableAsset asset;
	asset.asset = (void*) map;
	asset.path = path;
	asset.type  = ASSET_TYPES::INPUT_MAP;
	asset.last_edit_time = get_edit_time(path.c_str());

	parse_input_file(*map, path);

	register_asset(loader, asset);
}

void register_hotloadable_asset(HotLoader& loader, Sound* s, String path) {
	HotLoadableAsset asset;
	asset.asset = (void*) s;
	asset.path = path;
	asset.type  = ASSET_TYPES::AUDIO;
	asset.last_edit_time = get_edit_time(path.c_str());

	*s = new_sound(path);

	register_asset(loader, asset);
}

Sound::Sound(String path) {
	register_hotloadable_asset(hot_loader, this, path);
}
