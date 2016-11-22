#pragma once
#include<algorithm>
#include<vector>
#include"../platform.hpp"
#include"glo_grid.hpp"

using namespace std;
#define foreach(c,f)std::for_each(c.begin(),c.end(),f)

class grid{
	p3 po;
	float s;
	vector<shared_ptr<glob>>globs;
public:
	inline grid(const floato size,const p3&p=p3{}):po(p),s(size){metrics::ngrids++;}
	inline~grid(){metrics::ngrids--;clear();}
	inline void clear(){
		globs.clear();
	}
	inline void addall(const vector<shared_ptr<glob>>&ls){
		for(auto g:ls){
			g->grid_that_updates_this_glob=this;
			globs.push_back(g);
		}
	}
	inline void update_globs(){
		for(auto g:globs){
			if(g->grid_that_updates_this_glob!=this)
				continue;
			g->update();
		}
	}
	inline void render_globs(){
		for(auto g:globs){
			if(g->grid_that_updates_this_glob!=this)
				continue;
			g->render();
		}

	}
	inline void render_outline(){
		m4 m;
		m.load_translate(po);
		m.append_scaling(p3{s,s,s});
		glUniformMatrix4fv(GLint(gl::umtx_mw),1,false,m.c);
		glo_grid::instance.render();
	}
};
static grid grd(1);
