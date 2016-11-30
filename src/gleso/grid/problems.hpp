#pragma once

namespace problems{
	bool are_spheres_in_collision(glob const&g1,glob const&g2){
		const floato dx=g2.phy.p.x-g1.phy.p.x;
		const floato dy=g2.phy.p.y-g1.phy.p.y;
		const floato dz=g2.phy.p.z-g1.phy.p.z;
		const floato min_dist=g1.phy.r+g2.phy.r;
		const floato dx2=dx*dx;
		const floato dy2=dy*dy;
		const floato dz2=dz*dz;
		const floato min_dist2=min_dist*min_dist;
		const floato dist2=dx2+dy2+dz2;
		if(dist2<=min_dist2){
			return true;
		}
		return false;
	}
}
