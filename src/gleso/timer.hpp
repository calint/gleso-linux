#pragma once

class timer{
	double t0;
public:
	timer():t0(glfwGetTime()){}

	inline double dt(){
		const double t1=glfwGetTime();
		const double dt=t1-t0;
		t0=t1;
		return dt;
	}

};
