#pragma once
#include"../platform.hpp"
#include<sys/time.h>
#include<atomic>

class metrics_data{
public:
	atomic_int glob_count;
	atomic_int globs_updated;
	atomic_int globs_mutex_locks;
	int globs_rendered;
	floato globs_per_cell;
	int grid_cell_count;
	int shader_count;
	int texture_count;





	longo frame;
	floato fps;
	floato dt{1./60};





	void print_header_row(){
		p("%5s %5s %6s %5s %5s %5s %5s\n","fps","dt","globs","upd","rend","lcks","g/c");

	}
	void print(){
		p("%5.0f %5d %6d %5d %5d %5d %5d\n",
				fps,
				int(dt*1000000),
				int(glob_count),
				int(globs_updated),
				int(globs_rendered),
				int(globs_mutex_locks),
				globs_per_cell
		);
	}


	void before_render(){
		if(frame==0)
			print_header_row();

		frame++;

		globs_rendered=0;
	}

	void after_render(){
		struct timeval tv;
		gettimeofday(&tv,NULL);
		const time_t diff_s=tv.tv_sec-fps_prev_time_.tv_sec;
		const int diff_us=tv.tv_usec-fps_prev_time_.tv_usec;
		const floato dt=(float)diff_s+diff_us/1000000.f;
		if(dt<1)
			return;
		const int dframe=frame-fps_prev_frame_count_;
		fps_prev_frame_count_=frame;
		fps=dframe/dt;
		fps_prev_time_=tv;
		print();
	}

private:
	struct timeval fps_prev_time_;
	floato fps_prev_frame_count_;
};
metrics_data metrics2;
