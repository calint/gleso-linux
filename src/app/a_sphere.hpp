#pragma once
class a_sphere:public glob{
	floato constraint;

public:
	inline a_sphere(position p_={},floato r_=.1,velocity_vector v_={},floato constraint_=1):
		constraint{constraint_}
	{
		gl=&glo_ball::instance;
		phy.r=r_;
		phy.s={r_,r_,r_};
		phy.p=p_;
		phy.dp=v_;
	}
	inline void on_update()override{
//		p("update desk  %f    \n",phy.p.x);
		if(phy.p.x>constraint-phy.s.x)
			phy.dp.x=0;
		else if(phy.p.x<-constraint+phy.s.x)
			phy.dp.x=0;
		if(phy.p.y>constraint-phy.s.y)
			phy.dp.y=0;
		else if(phy.p.y<-constraint+phy.s.y)
			phy.dp.y=0;
	}
	inline virtual void on_collision(glob*g){
		p(" [ %p ] collided with %p\n",this,g);
		phy.dp=-phy.dp;
	}

};
