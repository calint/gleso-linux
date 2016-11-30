#pragma once

namespace gleso{namespace grid{
	class problems{
	public:
		static bool are_spheres_in_collision(const glob&g1,const glob&g2){
			const auto dx=g2.phy.p.x-g1.phy.p.x;
			const auto dy=g2.phy.p.y-g1.phy.p.y;
			const auto dz=g2.phy.p.z-g1.phy.p.z;
			const auto min_dist=g1.phy.r+g2.phy.r;
			const auto dx2=dx*dx;
			const auto dy2=dy*dy;
			const auto dz2=dz*dz;
			const auto min_dist2=min_dist*min_dist;
			const auto dist2=dx2+dy2+dz2;
			if(dist2<=min_dist2){
				return true;
			}
			return false;
		}
	};

}}
