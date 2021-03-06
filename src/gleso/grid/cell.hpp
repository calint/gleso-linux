#pragma once
#include "../../include.hpp"
#include"glo_grid.hpp"
#include"problems.hpp"

namespace gleso{namespace grid{

	class cell{
	public:
		vector<glob*>globs;

		inline cell(){metric.grid_cell_count++;}

		inline~cell(){
			metric.grid_cell_count--;
			for(auto g:globs){
				if(g->grid_cell_ref!=this)
					continue;
				delete g;
			}
		}

		inline void clear(){
			globs.clear();
		}

		inline void add(glob*g){
			globs.push_back(g);
		}

		inline void update_globs(const time_s dt){
			for(auto g:globs){
				if(g->grid_cell_ref!=this)
					continue;
				g->update(dt);
			}
		}

		inline void handle_collisions(const time_s dt){
			const int n=globs.size();
			for(auto i=0;i<n-1;i++){
				for(auto j=i+1;j<n;j++){
					auto g1=globs[i];
					auto g2=globs[j];

					// if globs at different frames then in different cells because update is done in this cell
					if(g1->last_frame_update!=g2->last_frame_update){
						// update the glob in the other cell
						if(g1->last_frame_update>g2->last_frame_update){
							g2->update(dt);
						}else{
							g1->update(dt);
						}
					}

					metric.globs_collisions_tried++;
					if(not problems::are_spheres_in_collision(*g1,*g2))
						continue;

//						p(" frame: %u    collision [%s %p] and [%s %p]\n",gl::time_stamp,typeid(g1).name(),g1,typeid(*g2).name(),g2);
					metric.globs_collisions++;
					if(g1->grid_cell_ref==g2->grid_cell_ref){// both handled by same cell, no racing
						if(g1->grid_cell_ref!=this){// ... but buy a different cell
							continue;
						}
						// handled by this cell
						g1->on_collision(g2);
						g2->on_collision(g1);
						continue;

					}
					// handled by different cells, may be multiple calls same collision
					metric.globs_collisions_overlapping_cells++;
					g1->handle_overlapped_collision(g2);
					g2->handle_overlapped_collision(g1);
				}
			}
		}

		inline void render_globs(const shader&s){
			for(auto g:globs)
				g->render(s);
		}

		inline void render_outline(const shader&s,const p3&po,const floato scale){
			m4 m;
			m.load_translate(po);
			m.append_scaling(p3{scale,scale,scale});
			glUniformMatrix4fv(s.umtx_mw,1,false,m.c);
			glo_grid::instance.render(*gl::active_shader);
		}

	};
}}
