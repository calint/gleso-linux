#pragma once
#include"../platform.hpp"
#include<sys/time.h>
#include<atomic>

class metrics_data{
public:
	atomic_int glob_count;
	atomic_int globs_updated;
	atomic_int globs_mutex_locks;
	atomic_int globs_rendered;
	int globs_per_cell;
};
metrics_data metrics2;
namespace metrics{
	int nshaders;
	int ngrids;
//	int nglos;
//	int nglobs;
	int ntextures;
	int updated_globs;
	int rendered_globs;
	int threads;

	floato dt{1./60};
	longo frame;//? rollover issues when used in comparisons
	floato fps;
	static struct timeval fps_time_prev;
	static floato fps_frame_prev;
	void print_header_row(){
		p("%5s %5s %6s %5s %5s %5s %5s\n","fps","dt","globs","upd","rend","lcks","g/c");

	}
	static void before_render(){
		if(frame==0)
			print_header_row();
		frame++;
		updated_globs=0;
		rendered_globs=0;
	}
	void print(){
		p("%5.0f %5d %6d %5d %5d %5d %5d\n",
				fps,
				int(dt*1000000),
				int(metrics2.glob_count),
				int(metrics2.globs_updated),
				int(metrics2.globs_rendered),
				int(metrics2.globs_mutex_locks),
				metrics2.globs_per_cell
		);
	}
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
