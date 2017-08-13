#!/bin/bash
mkdir -p bin
echo ""
echo "---------Compile---------"

# Copy the dependencies into the bin folder.
if [ ! -f libopenal.so ] 
then
	echo "============ Copying libraries..."
	cp include/linux/libopenal.so ./bin
	cp include/linux/libopenal.so.1 ./bin
	cp include/linux/libopenal.so.1.18.0 ./bin
	echo "============ Copy done."
fi

# The development compilation.
# I tought the line below would work, but apparently not, don't know why.
# CXX -O0 -DDEVELOPER=1 -DDEBUG=1 -o GAME ../src/linux_main.cpp
# -L/lib64/libGL.so -L/lib64/libGLEW.so -L/lib64/libglfw.so.3
clang++ -std=c++11 -g -O0 -DLINUX=1 -DDEVELOPER=1 -DDEBUG=1 -Iinclude/ -o bin/GAME src/linux.cpp -lGL -lGLEW -lglfw -lopenal

if [ $? -ne 0 ]; then
	echo "=========== Compilation failed"
	exit;
fi

if [[ $1 = "debug" ]]; then
	echo ""
	echo "---------- DEBUG ---------"
	gdb ./bin/GAME -x run
elif [[ $1 = "build" ]]; then
	echo ""
elif [[ 1 = 1 ]]; then
	echo ""
	echo "---------- RUN ---------"
	./bin/GAME
fi
echo ""
echo "========== Done"


