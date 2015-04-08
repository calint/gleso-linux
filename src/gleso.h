// cross platform interface
#ifndef GLESO_H
#define GLESO_H
#include "gles.h"
typedef GLfloat floato;
typedef GLint into;
#ifdef __cplusplus
extern "C" {
#endif
//	int gleso_argc;
//	char**gleso_argv;
	int gleso_init();// called when opengl context needs reload
	void gleso_viewport(const int width,const int height);// called when screen size changes
	void gleso_step();// called when opengl ready for render
	void gleso_key(int key,int scancode,int action,int mods);// called on key event
	//action 0:press   2:drag   1:release
	void gleso_touch(floato x,floato y,int action);// called on touch/pointer event
	void gleso_cleanup();
#ifdef __cplusplus
}
#endif
#endif
