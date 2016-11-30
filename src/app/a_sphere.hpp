#pragma once

class a_sphere:public glob{

	floato constraint_;

public:

	inline a_sphere(position p_={},floato r_=.1,velocity_vector v_={},floato constraint=1):constraint_{constraint}{
		phy.r=r_;
		phy.s={r_,r_,r_};
		phy.p=p_;
		phy.dp=v_;
		set_glo(&glo_ball::instance);
	}

	inline void on_update()override{
		if(phy.p.x>constraint_-phy.s.x)
			phy.dp.x=0;
		else if(phy.p.x<-constraint_+phy.s.x)
			phy.dp.x=0;
		if(phy.p.y>constraint_-phy.s.y)
			phy.dp.y=0;
		else if(phy.p.y<-constraint_+phy.s.y)
			phy.dp.y=0;
	}

	inline virtual void on_collision(glob*g){
		p("frame[%lld]   in [%s %p] collision with [%s %p]\n",metric.frame,typeid(*this).name(),(void*)this,typeid(*g).name(),(void*)g);
		restore_previous_physics_state();
		phy.dp=-phy.dp;
	}

};
