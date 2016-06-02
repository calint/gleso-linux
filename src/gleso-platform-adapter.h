#pragma once

//// opengl
// platform
#undef GLESO_EMBEDDED // macbookair 11
//#define GLESO_EMBEDDED // hp 11
#ifdef GLESO_EMBEDDED
#include<GLES2/gl2.h>
#else
#include<GL/glew.h>
#endif

//// macros
#include<stdio.h>
#define p(...)printf(__VA_ARGS__)
