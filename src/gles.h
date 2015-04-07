#ifndef gles_h
#define gles_h

//#define GLESO_NOT_EMBEDDED // macbookair 11
#define GLESO_EMBEDDED // hp 11


#ifdef GLESO_EMBEDDED
#include<GLES2/gl2.h>
#else
#include<GL/glew.h>
#endif
#include<stdio.h>
#define p(...)printf(__VA_ARGS__)
#endif
