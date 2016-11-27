#pragma once
#include<algorithm>
#include<vector>
#include"../platform.hpp"
#include"glo_grid.hpp"
using namespace std;

namespace update_render_sync{
	namespace work_to_do_count{
		bool _initiated=false;
		pthread_mutex_t mutex;
		//atomic<int>count;
		int count;
		pthread_cond_t cond;
		void init(){
			if(_initiated)
				return;
			pthread_mutex_init(&mutex,NULL);
			pthread_cond_init(&cond,NULL);
			_initiated=true;
		}
		void deinit(){
	        pthread_mutex_destroy(&mutex);
	        pthread_cond_destroy(&cond);
		}
	}
}

class grid_cell{
	vector<glob*>globs;

public:
	grid_cell(){metrics::ngrids++;}

	~grid_cell(){
		metrics::ngrids--;
//		for(auto g:globs){
//			if(g->grid_cell__update!=this)//? render timestamp
//				continue;
//			delete g;
//		}
	}
	inline void clear(){
		globs.clear();
	}
	inline void add(glob*g){
		g->grid_cell_ref=this;
		globs.push_back(g);
	}
	inline void update_globs(){
//		p(" globs in cell %p = %d\n",this,globs.size());
		for(auto g:globs){
			if(g->grid_cell_ref!=this){
//				p(" glob %p not in %p\n",this);
				continue;
			}
			g->update();
		}
	}
	inline void render_globs(){
		for(auto g:globs){
			if(g->should_render())
				g->render();
		}
	}
	inline void render_outline(const p3&po,const floato scale){
		m4 m;
		m.load_translate(po);
		m.append_scaling(p3{scale,scale,scale});
		glUniformMatrix4fv(GLint(gl::umtx_mw),1,false,m.c);
		glo_grid::instance.render();
	}

};

#include<atomic>

#include"../gleso/thread.hpp"
#include"../gleso/wqueue.hpp"

class wque_thread_work{
public:
	virtual~wque_thread_work(){}
	virtual void exec()=0;
};

class wque_thread_work_update:public wque_thread_work{
	grid_cell*cell_;
public:
	wque_thread_work_update(grid_cell*c):cell_{c}{}

	virtual void exec()final{
		cell_->update_globs();
		pthread_mutex_lock(&update_render_sync::work_to_do_count::mutex);
		update_render_sync::work_to_do_count::count--;
		if(update_render_sync::work_to_do_count::count==0){
			pthread_cond_signal(&update_render_sync::work_to_do_count::cond);
		}
		pthread_mutex_unlock(&update_render_sync::work_to_do_count::mutex);

	}
};

atomic_int threads_running_count;

class wque_thread:public thread{
	wqueue<wque_thread_work*>&queue_;

public:
	wque_thread(wqueue<wque_thread_work*>&queue):queue_(queue){
		metrics::threads++;
	}

	~wque_thread(){
		metrics::threads--;
	}

	void*run(){
		while(true){
			wque_thread_work*wrk=queue_.remove();
			threads_running_count++;
			wrk->exec();
			threads_running_count--;
			delete wrk;
		}
	}
};

#include <unistd.h>

class grid{
	p3 po_;
	floato cell_size_;
	vector<unique_ptr<grid_cell>>cells_;
	int nrows_;
	int ncols_;
	wqueue<wque_thread_work*>update_grid_queue_;
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
			cells_.push_back(make_unique<grid_cell>());
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
		{
			pthread_mutex_lock(&update_render_sync::work_to_do_count::mutex);
			update_render_sync::work_to_do_count::count=nrows_*ncols_;
			pthread_mutex_unlock(&update_render_sync::work_to_do_count::mutex);
		}
		for(int r=0;r<nrows_;r++){
			for(int c=0;c<ncols_;c++){
				grid_cell*cell=cells_[r*ncols_+c].get();
				wque_thread_work_update*wrk=new wque_thread_work_update(cell);
				update_grid_queue_.add(wrk);
			}
		}
		pthread_mutex_lock(&update_render_sync::work_to_do_count::mutex);
		while(update_render_sync::work_to_do_count::count!=0){
			pthread_cond_wait(&update_render_sync::work_to_do_count::cond,&update_render_sync::work_to_do_count::mutex);
//			p(" work_to_do_count_after_notify: %d\n",update_render_sync::work_to_do_count::count);
		}
		pthread_mutex_unlock(&update_render_sync::work_to_do_count::mutex);
//		int gu=globs_updated;
//		p(" %d ",gu);
	}

//	inline void update_globs(){
//		pthread_mutex_lock(&mutex_work_done);
////		p("  update_globs\n");
//		for(int r=0;r<nrows_;r++){
//			for(int c=0;c<ncols_;c++){
//				grid_cell*cell=cells_[r*ncols_+c].get();
//				wque_thread_work_update*wrk=new wque_thread_work_update(cell,r==nrows_-1 and c==ncols_-1);
//				update_grid_queue_.add(wrk);
//			}
//		}
//		pthread_mutex_unlock(&mutex_work_done);
//	}

	inline void update_globs2(){
//		p("  update_globs\n");
		for(int r=0;r<nrows_;r++){
			for(int c=0;c<ncols_;c++){
				grid_cell*cell=cells_[r*ncols_+c].get();
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
static grid grd;
