//
// This file, will be your key to compiling this project.
// Everything that you need to run this will be includede into
// this file, to decrease the number of compilation units and
// make it easy as pie to compile. Some people may disagree,
// to that I say, boo!
//

// The external libraries we will be needing.
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

// STL stuff.
#include <stdio.h>
#include <stdlib.h>


// My stuff.
#include "globals.h"
#include "audio.h"

// The implementations.
#include "audio.cpp"
#include "main.cpp"

int main(int c, char* v[]) {
	game_main();
	return 0;
}

