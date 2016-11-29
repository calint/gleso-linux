#pragma once

class p3{
public:
	inline p3():x{0},y{0},z{0}{}
	inline p3(floato x_):x{x_},y{0},z{0}{}
	inline p3(floato x_,floato y_):x{x_},y{y_},z{0}{}
	inline p3(floato x_,floato y_,floato z_):x{x_},y{y_},z{z_}{}
	inline p3&add(const p3&p,floato dt){x+=p.x*dt;y+=p.y*dt;z+=p.z*dt;return*this;}//? simd
	inline p3&negate(){x=-x;y=-y;z=-z;return*this;}
	inline p3&scale(floato s){x*=s;y*=s;z*=s;return*this;}
	inline p3 operator-()const{return p3{-x,-y,-z};}
	floato x,y,z;
};

inline p3 operator+(const p3&a,const p3&b){
	return p3{a.x+b.x,a.y+b.y,a.z+b.z};
}

inline p3 operator-(const p3&a,const p3&b){
	return p3{a.x-b.x,a.y-b.y,a.z-b.z};
}

inline bool operator==(const p3&a,const p3&b){
	return a.x==b.x and a.y==b.y and a.z==b.z;
}

inline bool operator!=(const p3&a,const p3&b){
	return not operator==(a,b);
}
