#pragma once
#include"../include.hpp"
#include"gl/glo.hpp"
#include"physics/physics.hpp"
#include"m4.hpp"
#include"metrics.hpp"
using namespace gl;
using namespace physics;

namespace gleso{
	namespace grid{class grid;class cell;}

	class glob{
		friend class grid::grid;
		friend class grid::cell;

	public:

		physics::physics phy;// current physics state

		inline glob(){
	//		p("new glob %p\n",(void*)this);
			metric.glob_count++;
			phy.r=.1;
			phy.s={phy.r,phy.r,phy.r};
			pthread_mutex_init(&handled_collisions_mutex,NULL);//? lazyinit
			pthread_mutex_init(&needs_update_mutex,NULL);//? lazyinit
		}

		inline glob(glo*g):gl{g}{
			metric.glob_count++;
			phy.r=.1;
			phy.s={phy.r,phy.r,phy.r};
			pthread_mutex_init(&handled_collisions_mutex,NULL);//? lazyinit
			pthread_mutex_init(&needs_update_mutex,NULL);//? lazyinit
		}

		inline virtual~glob(){
	//		p("delete glob %p\n",(void*)this);
			metric.glob_count--;
			pthread_mutex_destroy(&handled_collisions_mutex);
		}

		inline void set_glo(glo*gl){
			this->gl=gl;
		}

		inline void render(const shader&s){
			// check if already rendered this render frame, i.e. from a different grid
			if(prev_rendered_frame==metric.frame)
				return;

			metric.globs_rendered++;
			prev_rendered_frame=metric.frame;
			if(!gl)return;
	//		ginfo=ginfo_nxt;
			update_model_to_world_matrix();
	//		matrix_model_world.load_translate(ginfo.p);
	//		matrix_model_world.append_rotation_about_z_axis(ginfo.a.z);
	//		matrix_model_world.append_scaling(ginfo.s);
			glUniformMatrix4fv(s.umtx_mw,1,false,model_to_world_matrix_.c);
			gl->render(*gl::active_shader);
		}

		inline const m4&local_to_world_matrix(){
			update_model_to_world_matrix();
			return model_to_world_matrix_;
		}

		inline virtual void on_update(const time_s dt){}

		inline virtual void on_collision(glob*g){}


		//------------------------------ vehicle

		floato turn_rate_per_second=90;

		inline void vehicle_turn_left(floato s){
			phy.da.z=turn_rate_per_second*s;
		}

		inline void vehicle_turn_right(floato s){
			phy.da.z=-turn_rate_per_second*s;
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

		inline void handle_overlapped_collision(glob*g){
			pthread_mutex_lock(&handled_collisions_mutex);
			metric.globs_mutex_locks++;
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


		// no locks
		inline bool check_needs_update1(){
			//? ----- racing
			if(last_frame_update==metric.frame)
				return false;

			last_frame_update=metric.frame;
			return true;
			//? -----
		}

		// atomic lock
		atomic_flag lock=ATOMIC_FLAG_INIT;
		inline bool check_needs_update2(){
			int busy_waits{0};
			while(lock.test_and_set()){busy_waits++;}
			if(last_frame_update==metric.frame){
				lock.clear();
				if(busy_waits)p(" busy wait  %d\n",busy_waits);
				return false;
			}
			last_frame_update=metric.frame;
			lock.clear();
			if(busy_waits)p(" busy wait  %d\n",busy_waits);
			return true;
		}

		// mutex
		inline bool check_needs_update3(){
			pthread_mutex_lock(&needs_update_mutex);
			if(last_frame_update==metric.frame){
				pthread_mutex_unlock(&needs_update_mutex);
				return false;
			}
			last_frame_update=metric.frame;
			pthread_mutex_unlock(&needs_update_mutex);
			return true;
		}

		inline void update(const time_s dt){
			if(not check_needs_update2())
				return;
//			metric.globs_updated.fetch_add(1,memory_order_relaxed);
			metric.globs_updated++;

			const position p=phy.p;
			const angle a=phy.a;
			const scale s=phy.s;

			phy_prv=phy;

			phy.update(dt);

			if(not model_to_world_matrix_needs_update){
				if(p!=phy.p or a!=phy.a or s!=phy.s){
					model_to_world_matrix_needs_update=true;
				}
			}

			on_update(dt);

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


		struct glinfo{
			position p{0,0,0};
			angle a{0,0,0};
			scale s{0,0,0};
		};

		physics::physics phy_prv;// previous physics state

		glo*gl{nullptr};// ref to gl renderable

		vector<glob*>handled_collisions;

		pthread_mutex_t handled_collisions_mutex;

		longo prev_rendered_frame{0};

		longo last_frame_update{0};

		m4 model_to_world_matrix_;

		bool model_to_world_matrix_needs_update{true};

		glinfo ginfo;

		pthread_mutex_t needs_update_mutex;


	};
}
