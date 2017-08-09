//
// This keeps tracks of assets so they can be updated
// in the game if they are changed on disk.
//

enum ASSET_TYPES {
	SHADER,
	IMAGE,
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
	short delay = 1;
} hot_loader;

void destroy_hotloader(HotLoader& loader) {
}

void register_asset(HotLoader& loader, HotLoadableAsset asset) {
	loader.assets.push_back(asset);
}

void update_loader(HotLoader& loader) {
	struct stat attrib;
	for (auto& asset : loader.assets) {
		//HotLoadableAsset& asset = loader.assets[i];
		auto success = stat(asset.path.c_str(), &attrib);
		if (asset.last_edit_time == attrib.st_ctime) continue;
		if (asset.timer != loader.delay) {
			asset.timer++;
			continue;
		}
		
		// Reset the timer.
		asset.last_edit_time = attrib.st_ctime;
		asset.timer = 0;

		// Reload the actual assets.
		if (asset.type == ASSET_TYPES::SHADER) {
			Shader* shader = ((Shader*)asset.asset);
			Shader temp_shader = new_shader(asset.path, shader->name);
			if (temp_shader.program == -1) {
				printf("[Hotloader.cpp] Failed to reload shader (%s)\n", shader->name.c_str());
			} else {
				delete_shader(*shader);
				shader->program = temp_shader.program;
				printf("[HotLoader.cpp] Shader reloaded (%s)\n", shader->name.c_str());
			}
		} else if (asset.type == ASSET_TYPES::IMAGE) {
			Texture* texture = (Texture*)asset.asset;
			bool success = update_texture(*texture, asset.path);
			if (success) {
				printf("[Hotloader.cpp] Image reloaded (%s)\n", asset.path.c_str());
			} else {
				asset.last_edit_time -= 10;
			}
		} else {
			printf("[HotLoader.cpp] Trying to update unsupported asset type.\n");
		}
	}
}

void register_hotloadable_asset(HotLoader& loader, Shader* _asset, String path, String name) {
	struct stat attrib;
	// not sure about the return code...
	auto error = stat(path.c_str(), &attrib);
	if (error) {
		printf("[Hotloader.cpp] Failed to add shader asset, '%s'!\n", path.c_str());
		return;
	}

	HotLoadableAsset asset;

	asset.asset = (void*) _asset;
	asset.path = path;
	asset.type  = ASSET_TYPES::SHADER;
	asset.last_edit_time = attrib.st_ctime;

	*_asset = new_shader(path, name);

	register_asset(loader, asset);
}

void register_hotloadable_asset(HotLoader& loader, 
		Texture* _asset, String path, 
		bool linear_filtering = true, 
		int sprites_x = 0, int sprites_y = 0, 
		bool use_mipmaps = false) {

	String file_path = find_texture_file(path);
	struct stat attrib;
	// not sure about the return code...
	auto error = stat(file_path.c_str(), &attrib);
	if (error) {
		printf("[Hotloader.cpp] Failed to add image asset, '%s'!\n", file_path.c_str());
		return;
	}

	HotLoadableAsset asset;

	asset.asset = (void*) _asset;
	asset.path = file_path;
	asset.type  = ASSET_TYPES::IMAGE;
	asset.last_edit_time = attrib.st_ctime;

	*_asset = new_texture(path, linear_filtering, sprites_x, sprites_y, use_mipmaps);

	register_asset(loader, asset);
}
