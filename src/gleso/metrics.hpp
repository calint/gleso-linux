#pragma once
#include<sys/time.h>

#include "init.hpp"
namespace metrics{
	int nshaders;
	int ngrids;
	int nglos;
	int nglobs;
	int ntextures;
	int updated_globs;
	int rendered_globs;

	floato dt{1./60};
	longo frame;//?? rollover issues when used in comparisons
	floato fps;
	static struct timeval fps_time_prev;
	static floato fps_frame_prev;
	static void before_render(){
		frame++;
		updated_globs=0;
		rendered_globs=0;
	}
	void print(){p("fps:%03.0f – shaders:%01d – textures:%01d – glos:%02d – globs:%05d – updated:%02d – rendered:%02d – grids:%02d \n",fps,nshaders,ntextures,nglos,nglobs,updated_globs,rendered_globs,ngrids);}
	static void after_render(){
		struct timeval tv;
		gettimeofday(&tv,NULL);
		const time_t diff_s=tv.tv_sec-fps_time_prev.tv_sec;
		const int diff_us=tv.tv_usec-fps_time_prev.tv_usec;
		const floato dt=(float)diff_s+diff_us/1000000.f;
		if(dt<1)
			return;
		const int dframe=frame-fps_frame_prev;
		fps_frame_prev=frame;
		fps=dframe/dt;
		fps_time_prev=tv;
		print();
	}
}
