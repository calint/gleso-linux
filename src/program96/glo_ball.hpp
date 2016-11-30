#pragma once
#include"../platform.hpp"
using namespace gleso;
using namespace gl;

namespace program16{
	class glo_ball:public glo{
		int nvertices;
	public:
		inline glo_ball():nvertices(1+12+1){}
	protected:
		inline vector<GLfloat>make_vertices()const override{
			vector<GLfloat>v;
			v.push_back(0);//x
			v.push_back(0);//y
			floato rad=0;
			const floato drad=2*float(M_PI/12);
			for(int i=1;i<=nvertices;i++){
				const floato x=cosf(rad);
				const floato y=sinf(rad);
				rad+=drad;
				v.push_back(x);
				v.push_back(y);
			}
			return v;
		}
		inline vector<GLfloat>make_colors()const override{
			vector<GLfloat>v;
			const int n=nvertices*4;
			for(int k=0;k<n;k++){
				v.push_back(1);
			}
			return v;
		}
		inline void gldraw()const{glDrawArrays(GL_TRIANGLE_FAN,0,nvertices);}

		public:static glo_ball instance;
	};
	glo_ball glo_ball::instance=glo_ball();
}
