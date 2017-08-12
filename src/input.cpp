// 
// This file handles the input from controllers, keyboards and mice.
// It is responsible for reading the input keymap which the hotloader
// reloads when it is updated.
//

#define Q(x) #x
#define QUOTE(x) Q(x)
#define TO_KEY_ENUM(N) if (key == Q(N)) return GLFW_KEY_## N ;
#define TO_JOY_ENUM(N) if (key == Q(N)) return N;

const float DEADZONE = 0.1f;

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
	//int modifyers = 0; // Used for keyboards for Alt, Ctrl, Shift etc...
	INPUT_STATE state = INPUT_STATE::UP;
	float value = 0.0f;
};

enum CONTROLLER_TYPE {
	NONE,
	UNKNOWN,
	DS3,
	DS4,
	XBOX,
	XBONE,
};

enum CONTROLLER_BUTTONS {
	LEFT_UP,
	LEFT_DOWN,
	LEFT_LEFT,
	LEFT_RIGHT,

	RIGHT_UP,
	RIGHT_DOWN,
	RIGHT_LEFT,
	RIGHT_RIGHT,

	L2,
	R2,

	DPAD_UP,
	DPAD_DOWN,
	DPAD_RIGHT,
	DPAD_LEFT,

	SELECT,
	START,
	HOME,

	CROSS,
	SQUARE,
	CIRCLE,
	TRIANGLE,

	L1,
	R1,

	L3,
	R3,
};

struct Controller {
	CONTROLLER_TYPE type = CONTROLLER_TYPE::NONE;

	struct {
		float left_up    = 0;
		float left_down  = 0;
		float left_left  = 0;
		float left_right = 0;

		float right_up    = 0;
		float right_down  = 0;
		float right_left  = 0;
		float right_right = 0;

		float l2 = 0;
		float r2 = 0;

		float dpad_up    = 0;
		float dpad_down  = 0;
		float dpad_right = 0;
		float dpad_left  = 0;

		float select = 0;
		float start  = 0;
		float home   = 0;

		float cross    = 0;
		float square   = 0;
		float circle   = 0;
		float triangle = 0;

		float l1 = 0;
		float r1 = 0;

		float l3 = 0;
		float r3 = 0;
	};

	float operator[] (size_t i) {
		return ((float*)(void*)this)[i+1];
	}
};

struct InputState {
	float value = 0.0f;
	INPUT_STATE state = INPUT_STATE::UP;
};

struct InputMap {
	Controller controllers[GLFW_JOYSTICK_LAST];
	Array<InputAction> actions;
	HashMap<String, InputState> inputs;
} input_map;

void clean_input(InputMap& map) {
	map.actions.clear();
	map.inputs.clear();
}

/*
void controller_connect_callback(int joy, int event) {
	printf("Triggered!\n");
	if (event == GLFW_CONNECTED) {
		printf("Connected a controller '%s' (%d)\n", glfwGetJoystickName(joy), joy);
	} else if (event == GLFW_DISCONNECTED) {
		printf("Disconnected a controller '%s' (%d)\n", glfwGetJoystickName(joy), joy);
	}
}
*/

CONTROLLER_TYPE get_controller_type_from_name(String name);
void handle_ds3  (int id, Controller& c);
void handle_ds4  (int id, Controller& c);
void handle_xbox (int id, Controller& c);
void handle_xbone(int id, Controller& c);

void poll_controller_data(InputMap& map) {
	// if (map.controllers.size() == 0) return;
	for (int i = 0; i < GLFW_JOYSTICK_LAST; i++) {
		CONTROLLER_TYPE& type = map.controllers[i].type;
		int present = glfwJoystickPresent(i);
		if (!present) {
			if (type != CONTROLLER_TYPE::NONE) {
				// Disconnect
				type = CONTROLLER_TYPE::NONE;
				printf("Controller %d disconnected.\n", i);
			}
			continue;
		}

		if (present && type == CONTROLLER_TYPE::NONE) {
			// Connect
			printf("Connected controller %d as a ", i);
			String name = glfwGetJoystickName(i);
			type = get_controller_type_from_name(name);
		}

		Controller& c = map.controllers[i];
		switch (type) {
			case CONTROLLER_TYPE::DS3:
				handle_ds3(i, c);
				break;
			case CONTROLLER_TYPE::DS4:
				handle_ds4(i, c);
				break;
			case CONTROLLER_TYPE::XBOX:
				handle_xbox(i, c);
				break;
			case CONTROLLER_TYPE::XBONE:
				handle_xbone(i, c);
				break;
			case CONTROLLER_TYPE::UNKNOWN:
				break;
			case CONTROLLER_TYPE::NONE:
			default:
				printf("Unhandled controller type: %d\n", type);
		}
	}
}
	
void update_input(InputMap& map = input_map) {
	poll_controller_data(map);
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
			float value = map.controllers[0][action.input];
			float& curr_value = map.inputs[action.input_name].value;
			if (value > curr_value) {
				curr_value = value;
			}
		}
	}

	for (auto& it : map.inputs) {
		auto& state = it.second;
		if (state.value <= DEADZONE) {
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

inline bool exists(InputMap& map, const String& name) {
	auto it = map.inputs.find(name);
	return it != map.inputs.end();
}

inline bool exists(const String& name) {
	return exists(input_map, name);
}

bool add_if_not_added(InputMap& map, const String& name) {
	if (exists(map, name)) {
		return true;
	}
	InputState state;
	map.inputs[name] = state;
	return false;
}

bool unknown_name_error(InputMap& map, const String& name) {
	bool success = add_if_not_added(map, name);
	if (!success)
		printf("[Input.cpp] Unknown input name '%s'\n", name.c_str());
	return success;
}

bool is_down(InputMap& map, const String& name) {
	unknown_name_error(map, name);
	
	auto state = map.inputs[name].state;
	return state == INPUT_STATE::DOWN 
		|| state == INPUT_STATE::PRESSED;
}

inline bool is_down(const String& name) {
	return is_down(input_map, name);
}

bool is_up(InputMap& map, const String& name) {
	unknown_name_error(map, name);
	
	auto state = map.inputs[name].state;
	return state == INPUT_STATE::UP 
		|| state == INPUT_STATE::RELEASED;
}

inline bool is_up(const String& name) {
	return is_up(input_map, name);
}

bool pressed(InputMap& map, const String& name) {
	unknown_name_error(map, name);
	
	auto state = map.inputs[name].state;
	return state == INPUT_STATE::PRESSED;
}

inline bool pressed(const String& name) {
	return pressed(input_map, name);
}

bool released(InputMap& map, const String& name) {
	unknown_name_error(map, name);
	
	auto state = map.inputs[name].state;
	return state == INPUT_STATE::RELEASED;
}

inline bool released(const String& name) {
	return released(input_map, name);
}

bool changed(InputMap& map, const String& name) {
	unknown_name_error(map, name);
	
	auto state = map.inputs[name].state;
	return state == INPUT_STATE::RELEASED
		|| state == INPUT_STATE::PRESSED;

}

inline bool changed(const String& name) {
	return changed(input_map, name);
}

float value(InputMap& map, const String& name) {
	unknown_name_error(map, name);

	return map.inputs[name].value;
}

inline float value(const String& name) {
	return value(input_map, name);
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
	
	TO_KEY_ENUM(ESCAPE);
	TO_KEY_ENUM(ENTER);
	TO_KEY_ENUM(SPACE);
	
	printf("Unknown key: %s\n", key.c_str());
	return -1;
}

int string_to_joy(String& key) {
	TO_JOY_ENUM(LEFT_UP);
	TO_JOY_ENUM(LEFT_DOWN);
	TO_JOY_ENUM(LEFT_LEFT);
	TO_JOY_ENUM(LEFT_RIGHT);

	TO_JOY_ENUM(RIGHT_UP);
	TO_JOY_ENUM(RIGHT_DOWN);
	TO_JOY_ENUM(RIGHT_LEFT);
	TO_JOY_ENUM(RIGHT_RIGHT);

	TO_JOY_ENUM(DPAD_UP);
	TO_JOY_ENUM(DPAD_DOWN);
	TO_JOY_ENUM(DPAD_RIGHT);
	TO_JOY_ENUM(DPAD_LEFT);

	TO_JOY_ENUM(SELECT);
	TO_JOY_ENUM(START);
	TO_JOY_ENUM(HOME);

	TO_JOY_ENUM(CROSS);
	TO_JOY_ENUM(SQUARE);
	TO_JOY_ENUM(CIRCLE);
	TO_JOY_ENUM(TRIANGLE);

	TO_JOY_ENUM(L1);
	TO_JOY_ENUM(R1);
	TO_JOY_ENUM(L2);
	TO_JOY_ENUM(R2);
	TO_JOY_ENUM(L3);
	TO_JOY_ENUM(R3);

	printf("Unknown joy button: %s\n", key.c_str());
	return -1;
}


// 
// Input maps have the following format:
// # This is a comment.
//
// Keyboard 
// name_of_action K a
// name^      type^ ^Key  ^Modifyers
// 
// name_of_action J BUTTON_IDENTIFIER
// name^      type^ ^Key
// 
// .map

inline bool check_if_valid(const char c) {
	return !(c == '\n' || c == '\0');
}

int find_next_nonspace(const char* p) {
	size_t i = -1;
	char c;
	do {
		i++;
		c = p[i];
	} while (c == ' ' && check_if_valid(c));
	return i;
}

int find_next_space(const char* p) {
	size_t i = -1;
	char c;
	do {
		i++;
		c = p[i];
	} while (c != ' ' && check_if_valid(c));
	return i;
}


bool parse_input_file(InputMap& map, String path) {
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
	char* line = NULL;
	size_t len = 0;
	while (getline(&line, &len, file) != -1) {
		line_number++;
		// If it's a comment, don't continue
		if (line[0] == '#') continue;

		// Find the first space.
		size_t i = find_next_nonspace(line);
		char c = line[0];
		if (!check_if_valid(c)) {
			printf("Empty line: %d\n", line_number);
			continue;
		}

		i = find_next_space(line);
		c = line[i];
		if (!check_if_valid(c)) {
			printf("Only name: %d\n", line_number);
			continue;
		}

		// Copy over the string.
		line[i] = '\0';
		String name = line;

		// Cut of the first part of the line.
		char* key_p = line + i + 1;

		// Find the type of input.
		key_p += find_next_nonspace(key_p);
		char type = toupper(key_p[0]);

		// Find the next "word"
		key_p += find_next_space(key_p);
		key_p += find_next_nonspace(key_p);
		c = key_p[0];
		if (!check_if_valid(c)) {
			continue;
		}

		i = find_next_space(key_p);
		key_p[i] = '\0';
		String key = key_p;

		InputAction action;
		
		// Check if we allready have an entry for it.
		// Add if we don't.

		if (type == 'K') {
			// It's a keyboard!
			action.is_keyboard = true;

			action.input = string_to_glfw_key(key);
			action.input_name = name;
		} else if (type == 'J') {
			action.is_keyboard = false;

			action.input = string_to_joy(key);
		} else {
			printf("[Input.cpp] Unrecognized input type '%c', 'J' and 'K' are the only supporded types. (%s:%d)\n", key_p[0], path.c_str(), line_number);
			continue;
		}
		
		action.input_name = name;
		printf("Added action: %s, k=%d, key: %d\n", name.c_str(), action.is_keyboard, action.input);
		add_if_not_added(map, name);
		map.actions.push_back(action);
	}

	fclose(file);
	return true;
}
