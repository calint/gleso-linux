#pragma once
#include"../../include.hpp"
#include"../gl/glo.hpp"
using namespace gl;

namespace gleso{namespace grid{

	class glo_grid:public glo{
		vector<GLfloat>make_vertices()const override{
			return vector<GLfloat>{
			//	 x  y
				-1, 1,
				-1,-1,
				 1,-1,
				 1, 1
			};
		}
		vector<GLfloat>make_colors()const override{
			return vector<GLfloat>{
			//   R  G  B A
				 1, 1, 1, 1,
				 1, 1, 1, 1,
				 1, 1, 1, 1,
				 1, 1, 1, 1,
			};
		}
		inline void gldraw()const{glDrawArrays(GL_LINE_LOOP,0,4);}
	public:
		static glo_grid instance;
	};
	glo_grid glo_grid::instance=glo_grid{};
}

}
