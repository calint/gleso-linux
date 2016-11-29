#pragma once
#include"../gleso/gl/glo.hpp"

namespace a_camera{
	class a_camera:public glob{
		int screen_width{320},screen_height{240};
	public:
		inline a_camera():glob(&glo::instance){
			set_glo(&glo::instance);
			const floato s=.1f;
			phy.r=s;
			phy.s=p3{s,s,s};
		}

		inline void viewport(int w,int h){screen_width=w;screen_height=h;}

		inline void pre_render(){
			gl::active_shader->use_program();
			glClearColor(floato{.5},0,floato{.5},1);
			glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

			m4 vw,p;

			m4 m1,m2;
			m1.load_translate(-phy.p);
			m2.load_identity().append_rotation_about_z_axis(-phy.a.z);
			vw=m2*m1;

			const float aspect_ratio=floato(screen_height)/floato(screen_width);
			p.load_ortho_projection(-1,1,-aspect_ratio,aspect_ratio,0,1);

			m4 wvp=p*vw;

			glUniformMatrix4fv(GLint(gl::umtx_wvp),1,false,wvp.c);
		}
	//	virtual void on_update(){
	//	}
	};
}
