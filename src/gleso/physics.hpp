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
		using namespace metrics;
		dp.add(ddp,dt);
		p.add(dp,dt);
		da.add(dda,dt);
		a.add(da,dt);
	}
};
