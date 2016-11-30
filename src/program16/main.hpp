#pragma once

#include"../include.hpp"
#include"../gleso/grid/grid.hpp"
#include"../gleso/gl/a_camera.hpp"

#include<signal.h>
#include<thread>
#include<unistd.h>
#include "../program16/init.hpp"


/*-----------------------------
     __    _       __     __
    /  \   |      /  \   /  \
   /       |     |    | |
   |  ___  |     |    |  \__
   |    |  |     |    |     \
    \___/  |___|  \__/   \__/

------------------------------
"defglo" */

static grid::grid grd;


static void mainsig(const int i){
	p(" ••• terminated with signal %d\n",i);
	exit(i);
}

void gleso_init(){
	p("* gleso\n");
	for(int i=0;i<32;i++)signal(i,mainsig);//?
	shader::check_gl_error("init");
	shader::print_gl_string("GL_VERSION",GL_VERSION);
	shader::print_gl_string("GL_VENDOR",GL_VENDOR);
	shader::print_gl_string("GL_RENDERER",GL_RENDERER);
	//	    printGLString("Extensions",GL_EXTENSIONS);
	shader::print_gl_string("GL_SHADING_LANGUAGE_VERSION",GL_SHADING_LANGUAGE_VERSION);
	shader::check_gl_error("after opengl info");
	p("* types\n");
	p("%16s %4u B\n","int",(unsigned int)sizeof(int));
	p("%16s %4u B\n","float",(unsigned int)sizeof(float));
	p("%16s %4u B\n","long",(unsigned int)sizeof(long));
	p("%16s %4u B\n","long long",(unsigned int)sizeof(long long));
	p("%16s %4u B\n","double",(unsigned int)sizeof(double));
	p("%16s %4u B\n","long double",(unsigned int)sizeof(long double));
	p("%16s %4u B\n","short",(unsigned int)sizeof(short));
	p("%16s %4u B\n","bool",(unsigned int)sizeof(bool));
	p("%16s %4u B\n","char",(unsigned int)sizeof(char));
	p("%16s %4u B\n","p3",(unsigned int)sizeof(p3));
	p("%16s %4u B\n","m4",(unsigned int)sizeof(m4));
	p("%16s %4u B\n","glo",(unsigned int)sizeof(glo));
	p("%16s %4u B\n","glob",(unsigned int)sizeof(glob));
	p("%16s %4u B\n","grid",(unsigned int)sizeof(grid::grid));
	p("%16s %4u B\n","physics",(unsigned int)sizeof(physics::physics));
	srand(1);// generate same random numbers in different instances
	if(!gl::active_shader){// init
		p("* init\n");
		gl::active_shader=&shader::instance;
		gl::active_camera=new a_camera;
		gl::globs.push_back(gl::active_camera);
		init();
	}
	gl::active_program=0;
	p("* load\n");
	for_each(gl::shaders.begin(),gl::shaders.end(),[](shader*o){
//		p(" shader %p   %s\n",(void*)o,typeid(*o).name());
		o->load();
	});
	for_each(gl::textures.begin(),gl::textures.end(),[](texture*o){
//		p(" texture %p   %s\n",(void*)o,typeid(*o).name());
		o->load();
	});
	for_each(gl::glos.begin(),gl::glos.end(),[](glo*o){
//		p(" glo %p   %s\n",(void*)o,typeid(*o).name());
		o->load();
	});
}
void gleso_viewport(int width,int height){
	p("* viewport  %d x %d\n",width,height);
	if(gl::active_shader)gl::active_shader->viewport(width,height);
	gl::active_camera->viewport(width,height);
}

void gleso_step(){
	metric.on_frame_start();

	grd.clear();

	grd.addall(gl::globs);

	if(gleso::update_grid_cells_in_parallell){
		grd.update_globs(metric.dt);
	}else{
		grd.update_globs_single_thread(metric.dt);
	}

	if(gleso::use_grid){
		gl::active_camera->pre_render(*active_shader);
		if(gleso::render_globs)grd.render_globs(*active_shader);//? thread
		if(gleso::render_grid_outline)grd.render_outline(*active_shader);
	}

//	usleep(100000);

	metric.on_frame_end();
}

void gleso_key(int key,int scancode,int action,int mods){
	if(action==GLFW_REPEAT)
		return;

	p(" ** gleso_key  key=%d   scancode=%d    action=%d   mods=%d\n",key,scancode,action,mods);
	using namespace gl;
	switch(key){
	case 87://w - forward
		switch(action){
			case 1:active_camera->vehicle_forward(4);break;
			case 0:active_camera->vehicle_forward(0);break;
		}
		break;
	case 83://s - backward
		switch(action){
		case 1:active_camera->vehicle_backward(4);break;
		case 0:active_camera->vehicle_backward(0);break;
		}
		break;
	case 68://d - turn right
		switch(action){
			case 1:active_camera->vehicle_turn_right(1);break;
			case 0:active_camera->vehicle_turn_right(0);break;
		}
		break;
	case 65://a - turn left
		switch(action){
			case 1:active_camera->vehicle_turn_left(1);break;
			case 0:active_camera->vehicle_turn_left(0);break;
		}
		break;
	case 46://.
		switch(action){
			case 1:if(gleso::use_grid)grd.clear();gleso::use_grid=!gleso::use_grid;break;
			case 0:break;
		}
		break;
	case 44://,
		switch(action){
			case 1:break;
			case 0:gleso::update_grid_cells_in_parallell=!gleso::update_grid_cells_in_parallell;break;
		}
		break;
	}
}

void gleso_touch(floato x,floato y,int action){
	p(" ** gleso_touch  x=%.1f   y=%.1f    action=%d\n",x,y,action);
}

void gleso_cleanup(){
	p(" *** cleanup");
}






/*
 ascii sprite kit

             ____
            |O  O|
            |_  _|         <- "medusa"
             /||\

 _______________
   |____|     <- stuff to slide under
  //||\|\\


 _______________     <- floor and roof




     _\|/_
 _____/ \_______   <- stuff to jump over



       stuff that scores -->   . o O *     <-- stuff to avoid

*/

































