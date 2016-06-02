#pragma once
class p3{
public:
	inline p3&add(const p3&p,const floato dt){x+=p.x*dt;y+=p.y*dt;z+=p.z*dt;return*this;}//? simd
	inline p3 operator-()const{return p3{-x,-y,-z};}
	floato x,y,z;
};
inline p3 operator+(const p3&a,const p3&b){
	return p3{a.x+b.x,a.y+b.y,a.z+b.z};
}
inline p3 operator-(const p3&a,const p3&b){
	return p3{a.x-b.x,a.y-b.y,a.z-b.z};
}
