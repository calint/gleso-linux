/*
     /////  ///\   ///// ///// /////////
     ///   ///\\\  ///   ///  // /// //
    ///   ///  \\\///   ///     ///
  ///// /////   \\\/  /////   /////
*/
#pragma once
#include"../gleso/init.hpp"
#include"a_ball.hpp"
#include"a_camera.hpp"
static void init(){
	using namespace gl;
	shaders.push_back(&shader::instance);
	textures.push_back(&texture::instance);//?? leak
//	glos.push_back(&glo_circle_xy::instance);
	glos.push_back(&glo_grid::instance);
	glos.push_back(&glo_ball::instance);
	const int instances=1024*8;
	for(int n=0;n<instances;n++)
		globs.push_back(new a_ball());
}


/*-----------------------------
     __    _       __     __
    /  \   |      /  \   /  \
   /       |     |    | |
   |  ___  |     |    |  \__
   |    |  |     |    |     \
    \___/  |___|  \__/   \__/

------------------------------
"defglo" */
static void mainsig(const int i){
	p(" ••• terminated with signal %d\n",i);
	exit(i);
}
#include<signal.h>
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
	p("%16s %4u B\n","grid",(unsigned int)sizeof(octgrid));
	p("%16s %4u B\n","physics",(unsigned int)sizeof(physics));
	srand(1);// generate same random numbers in different instances
	if(!gl::active_shader){// init
		p("* init\n");
		gl::active_shader=&shader::instance;
		gl::active_camera=new a_camera();
		gl::globs.push_back(gl::active_camera);
		init();
	}
	gl::active_program=0;
	p("* load\n");
	foreach(gl::shaders,[](shader*o){
//		p(" shader %p   %s\n",(void*)o,typeid(*o).name());
		o->load();
	});
	foreach(gl::textures,[](texture*o){
//		p(" texture %p   %s\n",(void*)o,typeid(*o).name());
		o->load();
	});
	foreach(gl::glos,[](glo*o){
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
	metrics::before_render();
	gl::active_camera->pre_render();
	if(gleso::use_grid){
		grd.clear();
		grd.addall(gl::globs);
		grd.update_globs();//? mt
		if(gleso::render_globs)grd.render_globs();//? thread
		if(gleso::render_grid_outline)grd.render_outline();
	}else{
		foreach(gl::globs,[](glob*g){g->update();});//. async
		foreach(gl::globs,[](glob*g){g->render();});
	}
	metrics::after_render();
}
void gleso_key(int key,int scancode,int action,int mods){
	p("gleso_key  key=%d   scancode=%d    action=%d   mods=%d\n",key,scancode,action,mods);
	switch(key){
	case 87://w
		switch(action){
			case 1:gl::active_camera->phy.dp.y=1;break;
			case 0:gl::active_camera->phy.dp.y=0;break;
		}
		break;
	case 83://s
		switch(action){
			case 1:gl::active_camera->phy.dp.y=-1;break;
			case 0:gl::active_camera->phy.dp.y=0;break;
		}
		break;
	case 65://a
		switch(action){
//			case 1:c->phy.dp.x=-1;break;
//			case 0:c->phy.dp.x=0;break;
			case 1:gl::active_camera->phy.da.z=180;break;
			case 0:gl::active_camera->phy.da.z=0;break;
		}
		break;
	case 68://d
		switch(action){
//			case 1:c->phy.dp.x=1;break;
//			case 0:c->phy.dp.x=0;break;
			case 1:gl::active_camera->phy.da.z=-180;break;
			case 0:gl::active_camera->phy.da.z=0;break;
		}
		break;
	case 46://.
		switch(action){
			case 1:if(gleso::use_grid)grd.clear();gleso::use_grid=!gleso::use_grid;break;
			case 0:break;
		}
		break;
	}
}
void gleso_touch(floato x,floato y,int action){
	p("gleso_touch  x=%.1f   y=%.1f    action=%d\n",x,y,action);
}
void gleso_cleanup(){
	for(auto o:gl::globs){
		delete o;
	}
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

































