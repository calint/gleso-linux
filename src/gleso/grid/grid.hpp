#pragma once
#include"wque.hpp"
#include"../gl/shader.hpp"
using namespace gleso::gl;

namespace gleso{namespace grid{

	class grid{
		p3 po_;// location of center of square

		floato cell_size_;// side of square

		vector<cell>cells_;

		int nrows_;

		int ncols_;

		wque q_;

		vector<wque_thread>t_;

		wque_sync update_render_sync_;

	public:
		floato metric_globs_per_cell{0};


		inline grid(const int nthreads=4,const int rows=4,const int cols=4,const floato cell_size=.5f,const p3&p=p3{}):
			po_(p),cell_size_(cell_size),nrows_{rows},ncols_{cols}
		{
			const auto ncells=rows*cols;

			cells_.reserve(ncells);

			for(auto i=0;i<ncells;i++)
				cells_.emplace_back();

			t_.reserve(nthreads);

			for(auto i=0;i<nthreads;i++)
				t_.emplace_back(q_);
		}

		inline void clear(){
			for(auto&c:cells_)
				c.clear();
		}

		inline static int clamp(floato v,int min,int max){
			if(v<min)return min;
			if(v>=max)return max;
			return int(v);
		}

		inline void addall(const vector<glob*>&ls){
			const auto size=cell_size_;
			const auto bias_x=size*ncols_/2;
			const auto bias_y=size*nrows_/2;
			for(auto&g:ls){
				const auto min_x=g->phy.p.x-g->phy.r;
				const auto max_x=g->phy.p.x+g->phy.r;
				const auto min_y=g->phy.p.y-g->phy.r;
				const auto max_y=g->phy.p.y+g->phy.r;

				const auto cell_min_x=(min_x+bias_x)/size;
				const auto cell_min_y=(min_y+bias_y)/size;
				const auto cell_max_x=(max_x+bias_x)/size;
				const auto cell_max_y=(max_y+bias_y)/size;

				const auto cell_min_x_int=clamp(cell_min_x,0,ncols_-1);
				const auto cell_min_y_int=clamp(cell_min_y,0,nrows_-1);
				const auto cell_max_x_int=clamp(cell_max_x,0,ncols_-1);
				const auto cell_max_y_int=clamp(cell_max_y,0,nrows_-1);

				if(cell_min_x_int==cell_max_x_int and cell_min_y_int==cell_max_y_int){// no overlap
					auto cell_index=cell_min_y_int*ncols_+cell_min_x_int;
					g->overlaps_cells=false;
					g->grid_cell_ref=&cells_[cell_index];
					g->grid_cell_ref->add(g);
					continue;
				}

				for(auto y=cell_min_y_int;y<=cell_max_y_int;y++){
					for(auto x=cell_min_x_int;x<=cell_max_x_int;x++){
						auto cell_index=y*ncols_+x;
						g->overlaps_cells=true;
						g->grid_cell_ref=&cells_[cell_index];
						g->grid_cell_ref->add(g);
					}
				}
			}
		}

		inline void update_globs(time_s dt){
	//		p("  update_globs\n");
			metric.globs_updated=0;
			metric.globs_mutex_locks=0;
			const int ncells=nrows_*ncols_;
			update_render_sync_.set_work_to_do_count(ncells);
			int number_of_globs_in_grid{0};
			for(auto r=0;r<nrows_;r++){
				for(auto c=0;c<ncols_;c++){
					auto&cc=cells_[r*ncols_+c];
					number_of_globs_in_grid+=cc.globs.size();
					wque_work*wrk=new wque_work(update_render_sync_,cc,dt);
					q_.add(wrk);
				}
			}

			metric.globs_per_cell=number_of_globs_in_grid/ncells;

			update_render_sync_.wait_until_count_is_zero();
		}

		inline void update_globs_single_thread(time_s dt){
	//		p("  update_globs2\n");
			metric.globs_updated=0;
			metric.globs_mutex_locks=0;
			for(auto r=0;r<nrows_;r++){
				for(auto c=0;c<ncols_;c++){
					auto&cc=cells_[r*ncols_+c];
					cc.update_globs(dt);
					cc.handle_collisions(dt);
				}
			}
		}

		inline void render_globs(const shader&s){
			metric.globs_rendered=0;
			for(auto&c:cells_)
				c.render_globs(s);
		}

		inline void render_outline(const shader&s){
			p3 p{po_.x-cell_size_*ncols_/2+cell_size_/2,po_.y-cell_size_*nrows_/2+cell_size_/2,0};
			for(auto r=0;r<nrows_;r++){
				for(auto c=0;c<ncols_;c++){
					cells_[r*ncols_+c].render_outline(s,p,cell_size_/2);
					p.x+=cell_size_;
				}
				p.x=po_.x-cell_size_*ncols_/2+cell_size_/2;
				p.y+=cell_size_;
			}
		}
	};

}}
