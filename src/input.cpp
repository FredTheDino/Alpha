// 
// This file handles the input from controllers, keyboards and mice.
// It is responsible for reading the input keymap which the hotloader
// reloads when it is updated.
//

enum INPUT_STATE {
	UP,
	DOWN,
	RELEASED,
	PRESSED
};

struct InputAction {
	String input_name;
	bool is_keyboard = true;
	int input = 0;
	int modifyers = 0; // Used for keyboards for Alt, Ctrl, Shift etc...
	INPUT_STATE state = INPUT_STATE::UP;
	float value;
};

struct InputState {
	float value;
	INPUT_STATE state = INPUT_STATE::UP;
};

struct InputMap {
	Array<InputAction> actions;
	HashMap<String, InputState> inputs;
} input_map;

// 
// Input maps have the following format:
// # This is a comment.
// Keyboard 
// name_of_action K a [A|C|S]
// name^      type^ ^Key  ^Modifyers
// 
// name_of_action J BUTTON_IDENTIFIER
// name^      type^ ^Key
// 
// .map

void clean_input(InputMap& map) {
	map.actions.clear();
	map.inputs.clear();
}

void dump_input_to_file(InputMap& map, String path) {

}

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
		char* line_p = line + i + 1;

		if (line_p[0] == 'K') {
			// It's a keyboard!
			bool break_out = false;
			i = -1;
			do {
				i++;
				c = line_p[i];
				if (c != '\n' && c != '\0') {
					printf("[Input.cpp] Incomplete line. (%s:%d)\n", path.c_str(), line_number);				
					break_out = true;
					break;
				}
			} while (c != ' ');

			if (break_out) {
				continue;
			}

			printf("TODO INPUTLOADING!!!!!!!!: Found input with name: %s, and key: %c\n", name.c_str(), c);
		} else if (line_p[0] == 'J') {
			printf("[Input.cpp] Not handleing joystick input. (%s:%d)\n", path.c_str(), line_number);
		} else {
			printf("[Input.cpp] Unrecognized input type. (%s:%d)\n", path.c_str(), line_number);
		}
	}

	fclose(file);
}
