#pragma once
#include"../../include.hpp"
#include"../p3.hpp"
using namespace gleso;

namespace gleso{namespace physics{

	class physics{
	public:
		position p{0,0,0},dp{0,0,0},ddp{0,0,0};
		angle a{0,0,0},da{0,0,0},dda{0,0,0};
		scale s{0,0,0};
		radius_m r{0};
		weight_kg w{0};

		inline void update(time_s dt){
			dp.add(ddp,dt);
			p.add(dp,dt);
			da.add(dda,dt);
			a.add(da,dt);
		}
	};

}}
