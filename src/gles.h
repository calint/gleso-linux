// ubuntu linux hp 11
#ifndef gles_h
//#define GLESO_NOT_EMBEDDED
#define GLESO_EMBEDDED
#include<GLES3/gl3.h>
//#include<GLES3/gl3ext.h>
typedef GLfloat floato;
#include<stdio.h>
#define p(...)printf(__VA_ARGS__)
#ifdef __cplusplus
extern "C" {
#endif
    int gleso_init();// called when opengl context needs reload
    void gleso_step();// called when opengl ready for render
    void gleso_on_viewport_change(const int width,const int height);// called when screen size changes
#ifdef __cplusplus
}
#endif
#define gles_h
#endif
