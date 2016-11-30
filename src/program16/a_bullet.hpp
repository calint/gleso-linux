#pragma once
#include"../include.hpp"
#include"../gleso/glob.hpp"
#include"glo_bullet.hpp"

namespace program16{

	class a_bullet:public glob{
		floato constraint_;

	public:

		inline a_bullet(position p_={},floato r_=.1,velocity_vector v_={},floato constraint=1):
			glob{&glo_bullet::instance},constraint_{constraint}
		{
			phy.r=r_;
			phy.s={r_,r_,r_};
			phy.p=p_;
			phy.dp=v_;
		}

		inline void on_update(const time_s dt)override{
			if(phy.p.x>constraint_-phy.s.x)
				stop();
			else if(phy.p.x<-constraint_+phy.s.x)
				stop();
			if(phy.p.y>constraint_-phy.s.y)
				stop();
			else if(phy.p.y<-constraint_+phy.s.y)
				stop();
		}

		inline void on_collision(glob*g){
//			p("frame[%lld]   in [%s %p] collision with [%s %p]\n",metric.frame,typeid(*this).name(),(void*)this,typeid(*g).name(),(void*)g);
//			restore_previous_physics_state();
//			phy.dp={};
//			phy.da={};
			stop();
		}

	private:
		void stop(){
			phy.dp={};
			phy.da={};
		}

	};

}
