// cross platform interface
#ifndef GLESO_H
#define GLESO_H
#include"gles.h"
#ifdef __cplusplus
extern "C" {
#endif
	int gleso_init();// called when opengl context needs reload
	void gleso_on_viewport_change(const int width,const int height);// called when screen size changes
	void gleso_step();// called when opengl ready for render
#ifdef __cplusplus
}
#endif
#endif
