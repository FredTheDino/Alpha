// Global stuff, stored in a struct so it dosen't 
// convolute the global namespace.
struct {
	bool should_quit   = 0;
	GLFWwindow* window = 0;
	int window_width   = 1600;
	int window_height  = 900;
	float window_aspect_ratio = 0;
	char window_title[13] = "Hello world!";
} global;
