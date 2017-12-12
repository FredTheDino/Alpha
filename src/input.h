#pragma once

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
	int controller = -1;
	int input = 0;
	INPUT_STATE state = INPUT_STATE::UP;
	float value = 0.0f;
};

enum CONTROLLER_TYPE {
	NO_CONTROLLER,
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
	CONTROLLER_TYPE type = CONTROLLER_TYPE::NO_CONTROLLER;

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
	bool using_keyboard = true;
	Controller controllers[GLFW_JOYSTICK_LAST];
	Array<InputAction> actions;
	HashMap<String, InputState> inputs;
	Vec2 mouse_pos;
} input_map;

// Forward declaration so this can be implemented on the platform layer.
CONTROLLER_TYPE get_controller_type_from_name(String name);
void handle_ds3  (int id, Controller& c);
void handle_ds4  (int id, Controller& c);
void handle_xbox (int id, Controller& c);
void handle_xbone(int id, Controller& c);

