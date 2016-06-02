#pragma once
class a_ball:public glob{public:
	inline a_ball(){
		gl=&glo_ball::instance;
		const floato sq=.5;
		phy.p.x=rnd(-sq,sq);
		phy.p.y=rnd(-sq,sq);
		phy.r=.0025;
		phy.s=p3{phy.r,phy.r,phy.r};
		phy.dp.x=rnd(-sq,sq);
		phy.dp.y=rnd(-sq,sq);
//		phy.p.z=-1;
	}
	inline void on_update()override{
//		p("update desk  %f    \n",phy.p.x);
		const floato d=.1;
		const floato sq=.5;
		if(phy.p.x>sq-phy.s.x)
			phy.dp.x=-d;
		else if(phy.p.x<-sq+phy.s.x)
			phy.dp.x=d;
		if(phy.p.y>sq-phy.s.y)
			phy.dp.y=-d;
		else if(phy.p.y<-sq+phy.s.y)
			phy.dp.y=d;
	}
};
