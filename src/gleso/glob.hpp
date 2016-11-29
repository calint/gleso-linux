#pragma once
#include"../platform.hpp"

namespace grid{class cell;}

class glob{
public:
	physics phy;// current physics state
	physics phy_prv;// previous physics state
	glo*gl{nullptr};// ref to gl renderable

	inline glob(){
//		p("new glob %p\n",(void*)this);
		metrics::nglobs++;
		phy.r=.1;
		phy.s={phy.r,phy.r,phy.r};
		pthread_mutex_init(&handled_collisions_mutex,NULL);//? lazyinit
	}

	inline glob(glo*g):glob(){
		gl=g;
		pthread_mutex_destroy(&handled_collisions_mutex);
	}

	inline virtual~glob(){
//		p("delete glob %p\n",(void*)this);
		metrics::nglobs--;
	}

	inline void render(){
		// check if already rendered this render frame, i.e. from a different grid
		if(time_stamp_render==gl::time_stamp)
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
		if(time_stamp_update==gl::time_stamp)
			return;

		metrics::updated_globs++;

		globs_updated++;

		time_stamp_update=gl::time_stamp;

		phy_prv=phy;

		phy.update();

		on_update();

		copy_phy_to_rend();

		handled_collisions.clear(); //? lazy invocation using timestamp

	}

	inline void handle_collision(glob*g){
		on_collision(g);
	}

	inline void copy_phy_to_rend(){
		ginfo.p=phy.p;
		ginfo.a=phy.a;
		ginfo.s=phy.s;
	}




	void handle_overlapped_collision(glob*g){
		pthread_mutex_lock(&handled_collisions_mutex);
		for(auto gg:handled_collisions){
			if(gg==g)// collision already handled
				return;
		}
		handled_collisions.push_back(g);
		pthread_mutex_unlock(&handled_collisions_mutex);
		on_collision(g);
	}











	inline virtual void on_update(){}
	inline virtual void on_collision(glob*g){}




	// grid_cell accessed
	grid::cell*grid_cell_ref{nullptr};//managed by grid
	bool overlaps_cells{false};


	longo time_stamp_update{0};
private:
	vector<glob*>handled_collisions;
	pthread_mutex_t handled_collisions_mutex;

	struct glinfo{
		position p{0,0,0};
		angle a{0,0,0};
		scale s{0,0,0};
	};

	longo time_stamp_render{0};
	m4 matrix_model_world;
	glinfo ginfo;// info for opengl rendering
};
