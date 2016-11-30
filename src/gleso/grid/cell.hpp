#pragma once
#include"../../platform.hpp"
#include<vector>
#include"problems.hpp"

namespace grid{
	class cell{
	public:
		vector<glob*>globs;

		inline cell(){
			metrics::ngrids++;
		}

		inline~cell(){
			metrics::ngrids--;
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

		inline void update_globs(){
			for(auto g:globs){
				if(g->grid_cell_ref!=this)
					continue;
				g->update();
			}
		}

		inline void handle_collisions(){
			const int n=globs.size();
			for(int i=0;i<n-1;i++){
				for(int j=i+1;j<n;j++){
					auto g1=globs[i];
					auto g2=globs[j];

					// if globs at different frames then in different cells because update is done in this cell
					if(g1->time_stamp_update!=g2->time_stamp_update){
						// update the glob in the other cell
						if(g1->time_stamp_update>g2->time_stamp_update){
							g2->update();
						}else{
							g1->update();
						}
					}

					const bool b=problems::are_spheres_in_collision(*g1,*g2);
					if(b){
//						p(" frame: %u    collision [%s %p] and [%s %p]\n",gl::time_stamp,typeid(g1).name(),g1,typeid(*g2).name(),g2);
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
						g1->handle_overlapped_collision(g2);
						g2->handle_overlapped_collision(g1);
					}
				}
			}
		}

		inline void render_globs(){
			for(auto g:globs){
				g->render();
			}
		}

		inline void render_outline(const p3&po,const floato scale){
			m4 m;
			m.load_translate(po);
			m.append_scaling(p3{scale,scale,scale});
			glUniformMatrix4fv(GLint(gl::umtx_mw),1,false,m.c);
			glo_grid::instance.render();
		}

	};
}
