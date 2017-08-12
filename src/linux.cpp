//
// This file, will be your key to compiling this project.
// Everything that you need to run this will be includede into
// this file, to decrease the number of compilation units and
// make it easy as pie to compile. Some people may disagree,
// to that I say, boo!
//
// (I wonder if I actually need this file...)
// (Yes you do...)
//



#include "main.cpp"

int main(int c, char* v[]) {
	game_main();
	return 0;
}

CONTROLLER_TYPE get_controller_type_from_name(String name) {
	if (name == "Sony PLAYSTATION(R)3 Controller") {
		printf("DS3.\n");
		return CONTROLLER_TYPE::DS3;
	}
	
	if (name == "Sony Computer Entertainment Wireless Controller") {
		printf("DS4\n");
		return CONTROLLER_TYPE::DS4;
	}

	printf("Unknown '%s'\n", name.c_str());
	return CONTROLLER_TYPE::UNKNOWN;
}

#define NORMALIZE(x) x * 0.5f + 0.5f

void handle_ds3  (int id, Controller& c) {
	int count;
	const unsigned char* buttons = glfwGetJoystickButtons(id, &count);
	c.start  = buttons[0];
	c.select = buttons[3];

	c.l3 = buttons[1];
	c.r3 = buttons[2];

	c.dpad_up    = buttons[4];
	c.dpad_right = buttons[5];
	c.dpad_down  = buttons[6];
	c.dpad_left  = buttons[7];
	
	c.home = buttons[16];

	const float* axies = glfwGetJoystickAxes(id, &count);

	float left_x = axies[0];
	float left_y = axies[1];
	c.left_left  = left_x < 0 ? -left_x : 0;
	c.left_right = left_x > 0 ?  left_x : 0;

	c.left_up    = left_y < 0 ? -left_y : 0;
	c.left_down  = left_y > 0 ?  left_y : 0;

	float right_x = axies[2];
	float right_y = axies[3];
	c.right_left  = right_x < 0 ? -right_x : 0;
	c.right_right = right_x > 0 ?  right_x : 0;

	c.right_up    = right_y < 0 ? -right_y : 0;
	c.right_down  = right_y > 0 ?  right_y : 0;

	c.l2 = NORMALIZE(axies[12]);
	c.r2 = NORMALIZE(axies[13]);

	c.l1 = NORMALIZE(axies[14]);
	c.r1 = NORMALIZE(axies[15]);

	c.triangle = NORMALIZE(axies[16]);
	c.circle   = NORMALIZE(axies[17]);
	c.cross    = NORMALIZE(axies[18]);
	c.square   = NORMALIZE(axies[19]);
}
void handle_ds4  (int id, Controller& c) {

}
void handle_xbox (int id, Controller& c) {

}
void handle_xbone(int id, Controller& c) {

}
