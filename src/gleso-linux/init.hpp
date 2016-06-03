#pragma once
#include<stdio.h>
#define p(...)printf(__VA_ARGS__)
#define dbg(...)printf(__VA_ARGS__)

//#define GLESO_EMBEDDED
#undef GLESO_EMBEDDED

#ifdef GLESO_EMBEDDED
	#include<GLES2/gl2.h>
#else
	#include<GL/glew.h>
#endif
