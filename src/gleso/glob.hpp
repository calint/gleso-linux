#pragma once
#include"../platform.hpp"

namespace grid{class grid;class cell;}

class glob{
	friend class grid::grid;
	friend class grid::cell;

public:
	physics phy;// current physics state

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

	inline void set_glo(glo*gl){
		this->gl=gl;
	}

	inline void render(){
		// check if already rendered this render frame, i.e. from a different grid
		if(time_stamp_render==gl::time_stamp)
			return;

		metrics::rendered_globs++;
		globs_rendered++;
		time_stamp_render=gl::time_stamp;
		if(!gl)return;
//		ginfo=ginfo_nxt;
		update_model_to_world_matrix();
//		matrix_model_world.load_translate(ginfo.p);
//		matrix_model_world.append_rotation_about_z_axis(ginfo.a.z);
//		matrix_model_world.append_scaling(ginfo.s);
		glUniformMatrix4fv(gl::umtx_mw,1,false,model_to_world_matrix_.c);
		gl->render();
	}

	inline const m4&local_to_world_matrix(){
		update_model_to_world_matrix();
		return model_to_world_matrix_;
	}

	inline virtual void on_update(){}

	inline virtual void on_collision(glob*g){}


	//------------------------------ vehicle


	inline void vehicle_turn_left(floato s){
		phy.da.z=180*s;
	}

	inline void vehicle_turn_right(floato s){
		phy.da.z=-180*s;
	}

	inline void vehicle_forward(floato s){
		phy.dp=local_to_world_matrix().y_axis().scale(s);
	}

	inline void vehicle_backward(floato s){
		phy.dp=local_to_world_matrix().y_axis().scale(s).negate();
	}

	//-----------------------------------------
protected:
	inline void restore_previous_physics_state(){phy=phy_prv;}


private:
	//---------------------------------------- grid_cell accessed
	grid::cell*grid_cell_ref{nullptr};//managed by grid
	bool overlaps_cells{false};
	longo time_stamp_update{0};

	inline void handle_overlapped_collision(glob*g){
		pthread_mutex_lock(&handled_collisions_mutex);
		globs_mutex_locks++;
		for(auto gg:handled_collisions){
			if(gg==g){// collision already handled
				pthread_mutex_unlock(&handled_collisions_mutex);
				return;
			}
		}
		handled_collisions.push_back(g);
		pthread_mutex_unlock(&handled_collisions_mutex);
		on_collision(g);
	}
	inline void update(){
		if(time_stamp_update==gl::time_stamp)
			return;

		metrics::updated_globs++;

		globs_updated++;

		time_stamp_update=gl::time_stamp;


		const position p=phy.p;
		const angle a=phy.a;
		const scale s=phy.s;

		phy_prv=phy;

		phy.update();

		if(not model_to_world_matrix_needs_update){
			if(p!=phy.p or a!=phy.a or s!=phy.s){
				model_to_world_matrix_needs_update=true;
			}
		}

		on_update();

		copy_phy_to_rend();

		handled_collisions.clear(); //? lazy invocation using timestamp

	}
	//----------------------------------------


	inline void update_model_to_world_matrix(){
		if(not model_to_world_matrix_needs_update)
			return;

		model_to_world_matrix_.load_translate(phy.p);
		model_to_world_matrix_.append_rotation_about_z_axis(phy.a.z);
		model_to_world_matrix_.append_scaling(phy.s);
		model_to_world_matrix_needs_update=false;
//		p(" [%u]  update_model_to_world_matrix %p\n",gl::time_stamp,this);
	}
	inline void copy_phy_to_rend(){
		ginfo.p=phy.p;
		ginfo.a=phy.a;
		ginfo.s=phy.s;
	}



	physics phy_prv;// previous physics state
	glo*gl{nullptr};// ref to gl renderable

	vector<glob*>handled_collisions;
	pthread_mutex_t handled_collisions_mutex;

	struct glinfo{
		position p{0,0,0};
		angle a{0,0,0};
		scale s{0,0,0};
	};
	longo time_stamp_render{0};
	m4 model_to_world_matrix_;
	bool model_to_world_matrix_needs_update{true};
	glinfo ginfo;
};
