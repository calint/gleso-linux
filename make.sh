#!/bin/sh -x
# cross platform opengl framework
#	linux
#	android
#	ios	
#	osx

g++ -std=c++17 src/platform-linux/main.cpp -o gleso -lGL -lglfw -lGLEW -lpthread -O3 -Wall -Wextra -pedantic -Wfatal-errors -Wno-unused-function -Wno-unused-parameter &&
ls -la --color gleso