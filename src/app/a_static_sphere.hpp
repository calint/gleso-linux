#pragma once
#include"../platform.hpp"
#include"glo_ball.hpp"
class a_static_sphere:public glob{
public:
	inline a_static_sphere(position p_={},floato r_=.1){
		set_glo(&glo_ball::instance);
		phy.r=r_;
		phy.s={r_,r_,r_};
		phy.p=p_;
	}
	inline virtual void on_collision(glob*g){
		p("frame[%lld]   in [%s %p] collision with [%s %p]\n",gl::time_stamp,typeid(*this).name(),(void*)this,typeid(*g).name(),(void*)g);
	}

};
