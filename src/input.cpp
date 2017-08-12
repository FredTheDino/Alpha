// 
// This file handles the input from controllers, keyboards and mice.
// It is responsible for reading the input keymap which the hotloader
// reloads when it is updated.
//

#define Q(x) #x
#define QUOTE(x) Q(x)
#define TO_ENUM(N) if (key == Q(N)) return GLFW_KEY_## N ;

enum INPUT_STATE {
	UP = 0,
	DOWN = 1,
	RELEASED = 2,
	PRESSED = 3,
};

struct InputAction {
	String input_name;
	bool is_keyboard = true;
	int input = 0;
	int modifyers = 0; // Used for keyboards for Alt, Ctrl, Shift etc...
	INPUT_STATE state = INPUT_STATE::UP;
	float value = 0.0f;
};

struct InputState {
	float value = 0.0f;
	INPUT_STATE state = INPUT_STATE::UP;
};

struct InputMap {
	Array<InputAction> actions;
	HashMap<String, InputState> inputs;
} input_map;

void clean_input(InputMap& map) {
	map.actions.clear();
	map.inputs.clear();
}
	
void update_input(InputMap& map = input_map) {
	for (auto& it : map.inputs) {
		auto& state = it.second;
		// Reset the value each frame.
		state.value = 0.0f;

		if (state.state == INPUT_STATE::RELEASED) {
			state.state = INPUT_STATE::UP;
		} else if (state.state == INPUT_STATE::PRESSED) {
			state.state = INPUT_STATE::DOWN;
		}
	}

	for (const auto& action : map.actions) {
		if (action.is_keyboard) {
			int key_state = glfwGetKey(_g.window, action.input);
			if (key_state == GLFW_PRESS) {
				map.inputs[action.input_name].value = 1.0f;
			}
		} else {
			printf("[Update Input] Joystick input not supported.\n");
		}
	}

	for (auto& it : map.inputs) {
		auto& state = it.second;
		if (state.value < 0.5f) {
			if (state.state == INPUT_STATE::DOWN) {
				state.state = INPUT_STATE::RELEASED;
			} 
		} else {
			if (state.state == INPUT_STATE::UP) {
				state.state = INPUT_STATE::PRESSED;
			}
		}
	}
}

void dump_input_to_file(InputMap& map, String path) {

}

#define TO_GLFW_KEY(N) ( if (key == N ) return GLFW_KEY_## N ; )

int string_to_glfw_key(String& key) {
	for (auto & c: key) c = toupper(c);

	if (key.size() == 1) {
		// It's a key A-Z or 0-9
		char c = key[0];
		if ('0' <= c && c <= '9') {
			return c - '0' + 48;
		}
		if (('A' <= c && c <= 'Z') || 
			c == '[' || c == ']' || c == ',' || 
			c == '.' || c == '-' ||
			c == '=' || c == ';') {
			return c;
		}
	}
	
	TO_ENUM(ESCAPE);
	TO_ENUM(ENTER);
	TO_ENUM(SPACE);
	
	printf("Unknown key: %s\n", key.c_str());
	return -1;
}


// 
// Input maps have the following format:
// # This is a comment.
//
// Keyboard 
// name_of_action K a [A|C|S]
// name^      type^ ^Key  ^Modifyers
// 
// name_of_action J BUTTON_IDENTIFIER
// name^      type^ ^Key
// 
// .map
bool parse_input(InputMap& map, String path) {
	if (access(path.c_str(), F_OK) != 0) {
		printf("[Input.cpp] Couldn't find input file '%s'\n", path.c_str());
		return false;
	}

	FILE* file = fopen(path.c_str(), "r");
	if (file == 0) {
		printf("[Input.cpp] Failed to open file '%s'\n", path.c_str());
		return false;
	}

	int line_number = 0;
	char * line = NULL;
	size_t len = 0;

	while (getline(&line, &len, file) != -1) {
		line_number++;
		// If it's a comment, don't continue
		if (line[0] == '#') continue;
		
		// Find the first space.
		size_t i = -1;
		char c;
		do {
			c = line[++i];
		} while (c != ' ' && c != '\n' && c != '\0');
		// Copy over the string.
		line[i] = '\0';
		String name = line;
		line[i] = c;
		// Cut of the first part of the line.
		char* line_p = line + i;
		i = 0;
		while (line_p[i] == ' ') {
			i++;
		}
		line_p += i;

		if (toupper(line_p[0]) == 'K') {
			// It's a keyboard!
			char* key_p = line_p;
			i = 0;
			do {
				i++;
				c = key_p[i];
			} while (c == ' ');

			if (c == '\n' && c == '\0') {
				continue;
			}

			key_p += i;

			i = 0;
			do {
				i++;
				c = key_p[i];
			} while (c != ' ' && c != '\n' && c != '\0');

			key_p[i] = '\0';
			String key = key_p;
			key_p[i] = c;

			int key_id = string_to_glfw_key(key);


			int mod = 0;
			// Parse the rest of the line.
			char* mod_p = key_p + i;
			i = 0;
			do {
				if (toupper(c) == 'A') {
					// Alt modifyer
					mod |= GLFW_MOD_ALT;
					printf("Alt ");
				}
				
				if (toupper(c) == 'C') {
					// Ctrl modifyer
					mod |= GLFW_MOD_CONTROL;
					printf("Ctrl ");
				}

				if (toupper(c) == 'S') {
					// Shift modifyer.
					mod |= GLFW_MOD_SHIFT;
					printf("Shift ");
				}

				c = mod_p[i];
				i++;
			} while (c != '\0');
			
			// Check if we allready have an entry for it.
			// Add if we don't.
			auto it = map.inputs.find(name);
			if (it == map.inputs.end()) {
				InputState state;
				map.inputs[name] = state;
			}

			// Add to the end of the input array...
			InputAction action;
			action.is_keyboard = true;
			action.input_name = name;

			action.input = string_to_glfw_key(key);
			action.modifyers = mod;

			map.actions.push_back(action);
		} else if (toupper(line_p[0]) == 'J') {
			printf("[Input.cpp] Not handleing joystick input. (%s:%d)\n", path.c_str(), line_number);
		} else {
			printf("[Input.cpp] Unrecognized input type '%c', 'J' and 'K' are the only supporded.  (%s:%d)\n", line_p[0], path.c_str(), line_number);
		}
	}

	fclose(file);
	return true;
}
