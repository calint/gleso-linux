#pragma once
class physics{
public:
	p3 p{0,0,0},dp{0,0,0},ddp{0,0,0};//position
	p3 a{0,0,0},da{0,0,0},dda{0,0,0};//angle
	p3 s{0,0,0};//scale
	floato r;//radius
	void update(){
		dp.add(ddp,metrics::dt);
		p.add(dp,metrics::dt);
		da.add(dda,metrics::dt);
		a.add(da,metrics::dt);
	}
};
