#pragma once
#include"../platform.hpp"
#include"p3.hpp"

class physics{
public:
	position p{0,0,0},dp{0,0,0},ddp{0,0,0};
	angle a{0,0,0},da{0,0,0},dda{0,0,0};
	scale s{0,0,0};
	radius_m r{0};
	weight_kg w{0};

	inline void update(){
		dp.add(ddp,metrics2.dt);
		p.add(dp,metrics2.dt);
		da.add(dda,metrics2.dt);
		a.add(da,metrics2.dt);
	}
};
