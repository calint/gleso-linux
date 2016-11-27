#pragma once
#include"wque_thread.hpp"
#include"update_render_sync.hpp"
#include "wque_work_update_cell.hpp"

namespace grid{
	class grid{
		p3 po_;// location of center of square
		floato cell_size_;// side of the square
		vector<unique_ptr<cell>>cells_;
		int nrows_;
		int ncols_;
		wqueue<wque_work*>update_grid_queue_;
		vector<unique_ptr<wque_thread>>threads_;
		int nthreads_;

	public:

	//	inline grid(const int nthreads=1,const int rows=1,const int cols=1,const floato cell_size=2,const p3&p=p3{})
	//	inline grid(const int nthreads=2,const int rows=2,const int cols=2,const floato cell_size=1,const p3&p=p3{})
	//	inline grid(const int nthreads=1,const int rows=4,const int cols=4,const floato cell_size=.5f,const p3&p=p3{})
		inline grid(const int nthreads=4,const int rows=4,const int cols=4,const floato cell_size=.5f,const p3&p=p3{})
	//		:po_(p),cell_size_(cell_size),cells_(rows*cols),nrows_{rows},ncols_{cols},nthreads_(nthreads)
			:po_(p),cell_size_(cell_size),nrows_{rows},ncols_{cols},nthreads_{nthreads}
		{

			for(int i=0;i<nthreads;i++){
				threads_.push_back(make_unique<wque_thread>(update_grid_queue_));
			}
			const int n=rows*cols;
			for(int k=0;k<n;k++){
				cells_.push_back(make_unique<cell>());
			}
			for(auto&t:threads_)
				t->start();
		}

		inline void clear(){
			for(auto&c:cells_)
				c->clear();
		}

		inline static int clamp(floato v,int min,int max){
			if(v<min)return min;
			if(v>=max)return max-1;
			return int(v);
		}

		inline void addall(const vector<glob*>&ls){
			const floato size=cell_size_;
			const floato bias_x=size*ncols_/2;
			const floato bias_y=size*nrows_/2;
			for(auto&g:ls){
				const floato min_x=g->phy.p.x-g->phy.r;
				const floato max_x=g->phy.p.x+g->phy.r;
				const floato min_y=g->phy.p.y-g->phy.r;
				const floato max_y=g->phy.p.y+g->phy.r;

				const floato cell_min_x=(min_x+bias_x)/size;
				const floato cell_min_y=(min_y+bias_y)/size;
				const floato cell_max_x=(max_x+bias_x)/size;
				const floato cell_max_y=(max_y+bias_y)/size;

				const int cell_min_x_int=clamp(cell_min_x,0,ncols_);
				const int cell_min_y_int=clamp(cell_min_y,0,nrows_);
				const int cell_max_x_int=clamp(cell_max_x,0,ncols_);
				const int cell_max_y_int=clamp(cell_max_y,0,nrows_);

				if(cell_min_x==cell_max_x_int and cell_min_y_int==cell_max_y_int){// no overlap
					int cell_index=cell_min_y_int*ncols_+cell_min_x_int;
					cells_[cell_index]->add(g);
					continue;
				}

				for(int y=cell_min_y_int;y<=cell_max_y_int;y++){
					for(int x=cell_min_x_int;x<=cell_max_x_int;x++){
						int cell_index=y*ncols_+x;
						cells_[cell_index]->add(g);
					}
				}
			}
		}

		inline void update_globs(){
	//		p("  update_globs\n");
			globs_updated=0;
			update_render_sync::work_to_do_count::set(nrows_*ncols_);
			for(int r=0;r<nrows_;r++){
				for(int c=0;c<ncols_;c++){
					cell*cell=cells_[r*ncols_+c].get();
					wque_work_update_cell*wrk=new wque_work_update_cell(cell);
					update_grid_queue_.add(wrk);
				}
			}

			update_render_sync::work_to_do_count::wait_until_count_is_zero();
		}

		inline void update_globs2(){
	//		p("  update_globs2\n");
			for(int r=0;r<nrows_;r++){
				for(int c=0;c<ncols_;c++){
					cell*cell=cells_[r*ncols_+c].get();
					cell->update_globs();
				}
			}
		}

		inline void render_globs(){
			for(auto&c:cells_){
				c->render_globs();
			}
		}

		inline void render_outline(){
			p3 p{po_.x-cell_size_*ncols_/2+cell_size_/2,po_.y-cell_size_*nrows_/2+cell_size_/2,0};
			for(int r=0;r<nrows_;r++){
				for(int c=0;c<ncols_;c++){
					cells_[r*ncols_+c]->render_outline(p,cell_size_/2);
					p.x+=cell_size_;
				}
				p.x=po_.x-cell_size_*ncols_/2+cell_size_/2;
				p.y+=cell_size_;
			}
		}
	};
}
