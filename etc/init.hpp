#pragma once
#include"../platform.hpp"
////
// cross-platform
typedef GLfloat floato;
typedef GLint into;
typedef long long longo;


typedef floato weight_kg;
typedef floato radius_m;

#include<cstdlib>
inline floato rnd(floato min,floato max){return min+(floato(random())/RAND_MAX)*(max-min);}

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

#include"gleso.hpp"
#include"metrics.hpp"
#include"gl/gl.hpp"
#include"gl/shader.hpp"
#include"gl/texture.hpp"
#include"gl/glo.hpp"
#include"p3.hpp"
typedef p3 scale;
typedef p3 angle;
typedef p3 position;
typedef p3 velocity_vector;
typedef longo frame_count;
#include"physics.hpp"
#include"m4.hpp"
#include"glob.hpp"
#include"grid/glo_grid.hpp"
#include"grid/grid.hpp"
#include<algorithm>

#define foreach(c,f)std::for_each(c.begin(),c.end(),f);
