#pragma once
#include"../../include.hpp"

namespace gleso{namespace gl{

	class glo_camera:public glo{
		int nvertices;

	public:
		inline glo_camera():nvertices(1+3+1){}

	protected:
		inline vector<GLfloat>make_vertices()const override{
			vector<GLfloat>v;
			v.push_back(0);//x
			v.push_back(0);//y
			floato rad=M_PI/2;
			const floato drad=float(2*M_PI/(nvertices-2));
			for(int i=1;i<nvertices;i++){
				const floato x=cosf(rad);
				const floato y=sinf(rad);
				rad+=drad;
				v.push_back(x);
				v.push_back(y);
			}
			v.push_back(0);//x
			v.push_back(0);//y
			return move(v);
		}
		inline vector<GLfloat>make_colors()const override{
			vector<GLfloat>v;
			const int n=nvertices*4;
			for(int k=0;k<n;k++){
				v.push_back(.5);
			}
			return v;
		}
		inline void gldraw()const{
			glDrawArrays(GL_TRIANGLE_FAN,0,nvertices);
		}

	public:
		static glo_camera instance;
	};
	glo_camera glo_camera::instance=glo_camera();

}}
