#pragma once
#include"../platform.hpp"
namespace grid{class cell;}
class glob{
public:
	physics phy;// current physics state
	glo*gl{nullptr};// ref to gl renderable
	grid::cell*grid_cell_ref{nullptr};//managed by grid

	inline glob(){
//		p("new glob %p\n",(void*)this);
		metrics::nglobs++;
		phy.r=.1;
		phy.s={phy.r,phy.r,phy.r};
	}

	inline glob(glo*g):glob(){gl=g;}

	inline virtual~glob(){
//		p("delete glob %p\n",(void*)this);
		metrics::nglobs--;
	}

	inline void render(){
		if(not should_render())
			return;
		metrics::rendered_globs++;
		time_stamp_render=gl::time_stamp;
		if(!gl)return;
//		ginfo=ginfo_nxt;
		matrix_model_world.load_translate(ginfo.p);
		matrix_model_world.append_rotation_about_z_axis(ginfo.a.z);
		matrix_model_world.append_scaling(ginfo.s);
		glUniformMatrix4fv(gl::umtx_mw,1,false,matrix_model_world.c);
		gl->render();
	}

	inline void update(){
		if(not should_update())
			return;
		metrics::updated_globs++;
		globs_updated++;
		time_stamp_update=gl::time_stamp;
		phy.update();
		on_update();
		copy_phy_to_rend();
	}

	inline void copy_phy_to_rend(){
		ginfo.p=phy.p;
		ginfo.a=phy.a;
		ginfo.s=phy.s;
	}

	inline virtual void on_update(){}

private:
	inline bool should_update()const{return time_stamp_update!=gl::time_stamp;}
	inline bool should_render(){return time_stamp_render!=gl::time_stamp;}


	struct glinfo{
		position p{0,0,0};
		angle a{0,0,0};
		scale s{0,0,0};
	};

	longo time_stamp_render{0};
	longo time_stamp_update{0};
	m4 matrix_model_world;
	glinfo ginfo;// info for opengl rendering
};
