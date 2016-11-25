#!/bin/sh -x
# cross platform opengl framework
#	linux
#	android
#	ios	
#	osx

g++ -std=c++17 src/main.cpp -o gleso -lGL -lglfw -lGLEW -lpthread -O3 -Wall -Wextra -pedantic -Wfatal-errors
