// cross platform interface
#ifndef GLESO_H
#define GLESO_H
#include "gles.h"
typedef GLfloat floato;
typedef GLint into;
typedef long long longo;
#ifdef __cplusplus
extern "C" {
#endif
//	int gleso_argc;
//	char**gleso_argv;
	void gleso_init();// called when opengl context needs reload
	void gleso_viewport(const int width,const int height);// called when screen size changes
	void gleso_step();// called when opengl ready for render
	void gleso_key(const int key,const int scancode,const int action,const int mods);// called on key event
	//action 0:press   2:drag   1:release
	void gleso_touch(const floato x,const floato y,const int action);// called on touch/pointer event
	void gleso_cleanup();
#ifdef __cplusplus
}
#endif

#include<cmath>
#include<cstdlib>
inline floato rnd(floato min,floato max){return min+(floato(rand())/RAND_MAX)*(max-min);}
#endif
