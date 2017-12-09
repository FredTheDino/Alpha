// Global stuff, stored in a struct so it dosen't 
// convolute the global namespace.
struct {
	bool should_quit   = 0;
	GLFWwindow* window = 0;
	int window_width;
	int window_height;
	int sample_width;
	int sample_height;
	int msaa;
	float inv_window_width;
	float inv_window_height;
	float window_aspect_ratio;
	char window_title[13] = "Hello world!";
} global;

inline void set_window_info(int width, int height, int msaa = 1) {
	global.window_width = width;
	global.window_height = height;
	global.window_aspect_ratio = (float) width / (float) height;
	global.inv_window_width = 1.0f / width;
	global.inv_window_height = 1.0f / height;
	global.msaa = msaa;
	global.sample_width = width * msaa;
	global.sample_height = height * msaa;
}
