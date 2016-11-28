#pragma once
#include"cell.hpp"
#include"update_render_sync.hpp"
#include"wque_work.hpp"

namespace problems{
	bool are_spheres_in_collision(glob const&g1,glob const&g2){
		floato dx=g2.phy.p.x-g1.phy.p.x;
		floato dy=g2.phy.p.y-g1.phy.p.y;
		floato dz=g2.phy.p.z-g1.phy.p.z;
		floato min_dist=g1.phy.r+g2.phy.r;
		dx*=dx;
		dy*=dy;
		dz*=dz;
		min_dist*=min_dist;
		floato dist2=dx+dy+dz;
		if(dist2<=min_dist){
			return true;
		}
		return false;
	}
}

namespace grid{

	class wque_work_update_cell:public wque_work{
		cell&cell_;
		update_render_sync&urs_;

	public:

		inline wque_work_update_cell(update_render_sync&urs,cell&c):cell_{c},urs_{urs}{}

		virtual void exec()final{
			cell_.update_globs();

			const int n=gl::globs.size();
			for(int i=0;i<n-1;i++){
				for(int j=i+1;j<n;j++){
					auto g1=gl::globs[i];
					auto g2=gl::globs[j];
					if(g1->overlaps_cells and g2->overlaps_cells){
						pthread_mutex_lock(&g1->mutex_for_collision_checked_this_frame);
						if(g1->time_stamp_for_collision_check!=gl::time_stamp){
							g1->collision_checked_this_frame.clear();
							g1->time_stamp_for_collision_check=gl::time_stamp;
						}
						bool found=false;
						for(auto checked_glob:g1->collision_checked_this_frame){
							if(checked_glob==g2){
								found=true;
								break;
							}
						}
						pthread_mutex_unlock(&g1->mutex_for_collision_checked_this_frame);
						if(found){
							continue;
						}

						pthread_mutex_lock(&g2->mutex_for_collision_checked_this_frame);
						if(g2->time_stamp_for_collision_check!=gl::time_stamp){
							g2->collision_checked_this_frame.clear();
							g2->time_stamp_for_collision_check=gl::time_stamp;
						}
						found=false;
						for(auto checked_glob:g2->collision_checked_this_frame){
							if(checked_glob==g1){
								found=true;
								break;
							}
						}
						if(found){
							pthread_mutex_unlock(&g2->mutex_for_collision_checked_this_frame);
							continue;
						}
						if(not found){
							g2->collision_checked_this_frame.push_back(g1);
						}
						pthread_mutex_unlock(&g2->mutex_for_collision_checked_this_frame);
					}
					const bool b=problems::are_spheres_in_collision(*g1,*g2);
					if(b){
//						pthread_mutex_lock(&g1->mutex_for_collision_checked_this_frame);
//						pthread_mutex_lock(&g2->mutex_for_collision_checked_this_frame);
						g1->handle_collision_with(g2);
						g2->handle_collision_with(g1);
//						pthread_mutex_unlock(&g2->mutex_for_collision_checked_this_frame);
//						pthread_mutex_unlock(&g1->mutex_for_collision_checked_this_frame);
					}
				}
			}
			urs_.decrease_and_notify_if_zero();
		}

	};
}
