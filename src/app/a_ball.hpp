#pragma once
#include"glo_ball.hpp"
class a_ball:public glob{
	floato sq_,d_;
public:
	inline a_ball(const floato radius=.025f,const floato random_position_scale=1,const floato velocity=.1f)
		:sq_(random_position_scale),d_(velocity)
	{
		gl=&glo_ball::instance;
		phy.p.x=rnd(-random_position_scale,random_position_scale);
		phy.p.y=rnd(-random_position_scale,random_position_scale);
		phy.r=radius;
		phy.s=p3{radius,radius,radius};
		phy.dp.x=rnd(-velocity,velocity);
		phy.dp.y=rnd(-velocity,velocity);
//		phy.p.z=-1;
	}
	inline void on_update()override{
//		p("update desk  %f    \n",phy.p.x);
		if(phy.p.x>sq_-phy.s.x)
			phy.dp.x=-d_;
		else if(phy.p.x<-sq_+phy.s.x)
			phy.dp.x=d_;
		if(phy.p.y>sq_-phy.s.y)
			phy.dp.y=-d_;
		else if(phy.p.y<-sq_+phy.s.y)
			phy.dp.y=d_;
	}
};
