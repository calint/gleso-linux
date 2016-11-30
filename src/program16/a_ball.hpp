#pragma once
#include"../include.hpp"
#include"../gleso/glob.hpp"
#include "../program16/glo_ball.hpp"
using namespace gleso;
using namespace gl;

namespace program16{
	class a_ball:public glob{
		floato constraint_,velocity_;
	public:
		inline a_ball(const floato radius=.025f,const floato velocity=.1f,const floato constraint=1)
			:glob(&glo_ball::instance),constraint_{constraint},velocity_{velocity}
		{
	//		set_glo(&glo_ball::instance);
	//		gl=&glo_ball::instance;
			phy.p.x=rnd(-constraint,constraint);
			phy.p.y=rnd(-constraint,constraint);
			phy.r=radius;
			phy.s=p3{radius,radius,radius};
			phy.dp.x=rnd(-velocity,velocity);
			phy.dp.y=rnd(-velocity,velocity);
	//		phy.p.z=-1;
		}
		inline void on_update(const time_s dt)override{
	//		p("update desk  %f    \n",phy.p.x);
			if(phy.p.x>constraint_-phy.s.x)
				phy.dp.x=-velocity_;
			else if(phy.p.x<-constraint_+phy.s.x)
				phy.dp.x=velocity_;
			if(phy.p.y>constraint_-phy.s.y)
				phy.dp.y=-velocity_;
			else if(phy.p.y<-constraint_+phy.s.y)
				phy.dp.y=velocity_;
		}
	};
}
