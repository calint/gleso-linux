#pragma once
#include"../../platform.hpp"

namespace grid{
	class cell{
		vector<glob*>globs;

	public:
		cell(){
			metrics::ngrids++;
		}

//		cell(const cell&&c):
//			globs{move(c.globs)}
//		{}

		~cell(){
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
			g->grid_cell_ref=this;
			globs.push_back(g);
		}

		inline void update_globs(){
			for(auto g:globs){
				if(g->grid_cell_ref!=this)
					continue;
				g->update();
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
