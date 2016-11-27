#pragma once
#include"../../platform.hpp"
#include"../glo_grid.hpp"

namespace grid{
	class cell{
		vector<glob*>globs;

	public:
		cell(){metrics::ngrids++;}

		~cell(){
			metrics::ngrids--;
	//		for(auto g:globs){
	//			if(g->grid_cell__update!=this)
	//				continue;
	//			delete g;
	//		}
		}
		inline void clear(){
			globs.clear();
		}
		inline void add(glob*g){
			g->grid_cell_ref=this;
			globs.push_back(g);
		}
		inline void update_globs(){
	//		p(" globs in cell %p = %d\n",this,globs.size());
			for(auto g:globs){
				if(g->grid_cell_ref!=this){
	//				p(" glob %p not in %p\n",this);
					continue;
				}
				g->update();
			}
		}
		inline void render_globs(){
			for(auto g:globs){
				if(g->should_render())
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
