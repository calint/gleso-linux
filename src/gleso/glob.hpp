#pragma once
class grid;
class glob{
	struct glinfo{
		position p{0,0,0};
		angle a{0,0,0};
		scale s{0,0,0};
	};
public:
	physics phy;// current physics state
	glo*gl{nullptr};// ref to gl renderable
	grid*grid_that_updates_this_glob{nullptr};//managed by grid
	inline glob(){
//		p("new glob %p\n",(void*)this);
		metrics::nglobs++;
		phy.r=.1;
		phy.s={phy.r,phy.r,phy.r};
	}
	inline glob(glo*g):glob(){gl=g;}
	inline virtual~glob(){
//		p("delete glob %p\n",(void*)this);
		metrics::nglobs--;
	}
	inline void render(){
		metrics::rendered_globs++;
		if(!gl)return;
		ginfo=ginfo_nxt;
		matrix_model_world.load_translate(ginfo.p);
		matrix_model_world.append_rotation_about_z_axis(ginfo.a.z);
		matrix_model_world.append_scaling(ginfo.s);
		glUniformMatrix4fv(gl::umtx_mw,1,false,matrix_model_world.c);
		gl->render();
	}
	inline void update(){
		metrics::updated_globs++;
		phy.update();
		on_update();
		ginfo_nxt.p=phy.p;
		ginfo_nxt.a=phy.a;
		ginfo_nxt.s=phy.s;
	}
	inline virtual void on_update(){}
private:
	m4 matrix_model_world;
	glinfo ginfo;// info for opengl rendering
	glinfo ginfo_nxt;// next renderinfo, updated during render
//	physics phy_prv;// previous physics state
//	physics phy_nxt;// next physics state
};
