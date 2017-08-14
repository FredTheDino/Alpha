// 
// This file handles the input from controllers, keyboards and mice.
// It is responsible for reading the input keymap which the hotloader
// reloads when it is updated.
//
#include "input.h"

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

// 
// Input maps have the following format:
// # This is a comment.
//
// Keyboard 
// name_of_action K a
// name^      type^ ^Key/Key identifier.
// 
// Controller
// name_of_action J BUTTON_IDENTIFIER
// name^      type^ ^Key

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
	bool used_keyboard = false;
	bool used_controller = false;

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
			int key_state = glfwGetKey(global.window, action.input);
			if (key_state == GLFW_PRESS) {
				used_keyboard = true;
				map.inputs[action.input_name].value = 1.0f;
			}
		} else {
			float value = map.controllers[0][action.input];
			float& curr_value = map.inputs[action.input_name].value;
			if (value > curr_value) {
				used_controller = true;
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

	if (used_keyboard) {
		map.using_keyboard = true;
	} else if (used_controller) {
		map.using_keyboard = false;
	}
}

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

inline bool is_end_of_string(const char c) {
	return !(c == '\n' || c == '\0');
}

int find_next_nonspace(const char* p) {
	size_t i = -1;
	char c;
	do {
		i++;
		c = p[i];
	} while (c == ' ' && is_end_of_string(c));
	return i;
}

int find_next_space(const char* p) {
	size_t i = -1;
	char c;
	do {
		i++;
		c = p[i];
	} while (c != ' ' && is_end_of_string(c));
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
		find_next_nonspace(line);
		size_t i = find_next_space(line);
		char c = line[i];

		// Copy over the string.
		InputAction action;
		line[i] = '\0';
		action.input_name = line;

		if (action.input_name == "") {
			continue;
		}

		char* key_p;
		key_p = line + i + 1;
		key_p += find_next_nonspace(key_p);
		char type = toupper(key_p[0]);
		
		// Find the next "word"
		key_p += find_next_space(key_p);
		key_p += find_next_nonspace(key_p);
		c = key_p[0];

		i = find_next_space(key_p);
		key_p[i] = '\0';
		String key = key_p;

		if (type == 'K') {
			// It's a keyboard!
			action.is_keyboard = true;

			action.input = string_to_glfw_key(key);
		} else if (type == 'J') {
			action.is_keyboard = false;

			action.input = string_to_joy(key);
		} else {
			printf("[Input.cpp] Unrecognized input type '%c', 'J' and 'K' are the only supporded types. (%s:%d)\n", key_p[0], path.c_str(), line_number);
			continue;
		}

		add_if_not_added(map, action.input_name);

		map.actions.push_back(action);
	}

	if (line) 
		free(line);

	fclose(file);
	return true;
}

inline Vec2 mouse_to_rhc (InputMap& map) {
	return Vec2(map.mouse_pos.x / global.window_width, map.mouse_pos.y / global.window_height);
}

