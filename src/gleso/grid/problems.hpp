#pragma once

namespace problems{
	bool are_spheres_in_collision(glob const&g1,glob const&g2){
		floato dx=g2.phy.p.x-g1.phy.p.x;
		floato dy=g2.phy.p.y-g1.phy.p.y;
		floato dz=g2.phy.p.z-g1.phy.p.z;
		floato min_dist=g1.phy.r+g2.phy.r;
		dx*=dx;
		dy*=dy;
		dz*=dz;
		min_dist*=min_dist;
		floato dist2=dx+dy+dz;
		if(dist2<=min_dist){
			return true;
		}
		return false;
	}
}
