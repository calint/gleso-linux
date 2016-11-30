#!/bin/sh -x
# cross platform opengl framework
#	linux
#	android
#	ios	
#	osx

CC='g++ -std=c++17'
SRC=src/platform-linux/main.cpp
BIN=gleso
WARNINGS='-Wall -Wextra -pedantic -Wfatal-errors -Wno-unused-function -Wno-unused-parameter'
LIBS='-lGL -lglfw -lGLEW -lpthread'
OPTIMIZATIONS=-O3
VALGRIND='valgrind --leak-check=full --track-origins=yes'

$CC $SRC -o $BIN $OPTIMIZATIONS $LIBS $WARNINGS &&
ls -la --color $BIN &&
$VALGRIND ./$BIN