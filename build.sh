#!/bin/bash
mkdir -p bin
cd bin
echo ""
echo "---------Compile---------"

# The development compilation.
# I tought the line below would work, but apparently not, don't know why.
# CXX -O0 -DDEVELOPER=1 -DDEBUG=1 -o GAME ../src/linux_main.cpp
# -L/lib64/libGL.so -L/lib64/libGLEW.so -L/lib64/libglfw.so.3
clang++ -std=c++11 -O0 -DDEVELOPER=1 -DDEBUG=1 -I../include/ -o GAME ../src/linux_main.cpp -lGL -lGLEW -lglfw

if [ $? -ne 0 ]; then
	echo "=========== Compilation failed"
	exit;
fi

if [[ $1 = "debug" ]]; then
	echo ""
	echo "---------- DEBUG ---------"
	gdb ./GAME -x run
elif [[ $1 = "build" ]]; then
	echo ""
elif [[ 1 = 1 ]]; then
	echo ""
	echo "---------- RUN ---------"
	./GAME
fi
echo ""
echo "========== Done"
cd ..
