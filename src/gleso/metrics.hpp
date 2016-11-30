#pragma once
#include"../platform.hpp"
#include<atomic>
#include<iostream>
using namespace std;
using namespace chrono;

namespace gleso{
	class metrics{
	public:
		atomic_int glob_count{0};
		atomic_int globs_updated{0};
		atomic_int globs_mutex_locks{0};
		int globs_rendered{0};
		floato globs_per_cell{0};
		atomic_int globs_collisions{0};
		atomic_int globs_collisions_overlapping_cells;
		int grid_cell_count{0};
		int shader_count{0};
		int texture_count{0};
		int glo_count{0};

		longo frame{0};
		floato dt{1./60};


		inline~metrics(){print();}

		inline void print_header_row(){
			p("%5s %5s %6s %5s %5s %5s %5s %4s\n","fps","dt","globs","upd","rend","lcks","g/c","glos");

		}
		inline void print(){
			p("%5d %5d %6d %5d %5d %5d %.2f %5d\n",
					int(fps),
					int(dt*1000000),
					int(glob_count),
					int(globs_updated),
					int(globs_rendered),
					int(globs_mutex_locks),
					globs_per_cell,
	//				int(globs_collisions),
	//				int(globs_collisions_overlapping_cells),
					glo_count
			);
		}

		inline void on_frame_start(){
			if(frame==0){
				print_header_row();
				fps_time=system_clock::now();
			}
			frame++;
			globs_rendered=0;
			globs_collisions=0;
		}

		inline void on_frame_end(){
			const auto now=system_clock::now();
			const auto dt_s=duration<floato,ratio<1>>(now-fps_time).count();
			if(dt_s<print_intervall_in_seconds)
				return;
			const auto dframe=frame-fps_frame;
			fps=dframe/dt_s;
			fps_frame=frame;
			fps_time=now;
			print();
	}

	private:
		floato fps{0};

		time_point<system_clock>fps_time;

		int fps_frame{0};

		floato print_intervall_in_seconds{.5};

	}metric;
}
