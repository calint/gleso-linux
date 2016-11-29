#pragma once
class a_camera:public glob{
	int screen_width{320},screen_height{240};
public:
//	m4 mtx_wvp;// world->view->projection
//	inline a_camera():glob(&glo_circle_xy::instance){}
	inline a_camera():glob(&glo_grid::instance){
		gl=&glo_ball::instance;
		const floato s=.1f;
		phy.r=s;
		phy.s=p3{s,s,s};
	}
	inline void viewport(int w,int h){screen_width=w;screen_height=h;}
	void pre_render(){
		gl::active_shader->use_program();
		glClearColor(floato{.5},0,floato{.5},1);
		glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

		m4 wv,p;
		wv.load_translate(-phy.p);
		wv.append_rotation_about_z_axis(-phy.a.z);

		const float aspect_ratio=floato(screen_height)/floato(screen_width);
		p.load_ortho_projection(-1,1,aspect_ratio,-aspect_ratio,0,1);
//		p.load_ortho_projection(-1,1,-1,1,0,1);

		m4 wvp=p*wv;

		glUniformMatrix4fv(GLint(gl::umtx_wvp),1,false,wvp.c);
	}
//	virtual void on_update(){
//		if(phy.p.x>1)
//			phy.dp.x=-1;
//		else if(phy.p.x<-1)
//			phy.dp.x=1;
//	}
//	inline const m4&matrix_world_view_projection()const{return mtx_wvp;}
	inline virtual void on_collision(glob*g){
		p("frame[%u]   in [%s %p] collision with [%s %p]\n",gl::time_stamp,typeid(*this).name(),this,typeid(*g).name(),g);
	}
};
