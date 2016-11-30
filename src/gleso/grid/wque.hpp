#pragma once
#include<pthread.h>
#include"cell.hpp"
using namespace std;

namespace gleso{namespace grid{

	//- --- - - - - - -------- --     --- - -- -- - - - -- - -- ----- - -- - - -- - -- -
	class wque_sync{
		atomic_int count_;
		pthread_mutex_t m_;
//		int count_;
		pthread_cond_t c_;
	public:

		inline wque_sync():count_{0}{
			pthread_mutex_init(&m_,NULL);
			pthread_cond_init(&c_,NULL);
		}

		inline~wque_sync(){
			pthread_mutex_destroy(&m_);
			pthread_cond_destroy(&c_);
		}
//		void decrease_and_notify_if_zero(){
//			pthread_mutex_lock(&m_);
//			count_--;
//			if(count_==0)
//				pthread_cond_signal(&c_);
//			pthread_mutex_unlock(&m_);
//		}
		inline void decrease_and_notify_if_zero(){
			if(--count_)
				return;
			pthread_mutex_lock(&m_);
				pthread_cond_signal(&c_);
			pthread_mutex_unlock(&m_);
		}

		inline void wait_until_count_is_zero(){
			pthread_mutex_lock(&m_);
				while(count_!=0)
					pthread_cond_wait(&c_,&m_);
			pthread_mutex_unlock(&m_);
		}

		inline void set_work_to_do_count(int n){
//			pthread_mutex_lock(&m_);
				count_=n;
//			pthread_mutex_unlock(&m_);
		}
	};

	//- --- - - - - - -------- --     --- - -- -- - - - -- - -- ----- - -- - - -- - -- -
	class wque_work{
		time_s dt_;
		int from_including_;
		int to_excluding_;
		wque_sync&s_;
		vector<cell>&cells_;

	public:

		inline wque_work(wque_sync&urs,vector<cell>&cells,time_s dt,int from_including,int to_excluding):
			dt_{dt},
			from_including_{from_including},
			to_excluding_{to_excluding},
			s_{urs},
			cells_{cells}
		{}

		inline void exec(){
			for(int i=from_including_;i<to_excluding_;i++){
				cell&c=cells_[i];
				c.update_globs(dt_);
				c.handle_collisions(dt_);
			}
			s_.decrease_and_notify_if_zero();
		}

	};


	//- --- - - - - - -------- --     --- - -- -- - - - -- - -- ----- - -- - - -- - -- -
	class wque{
		vector<wque_work*>queue_;
		pthread_mutex_t mutex_;
		pthread_cond_t cond_;
	public:
		wque(){
			pthread_mutex_init(&mutex_,NULL);
			pthread_cond_init(&cond_,NULL);
		}

		~wque(){
			pthread_mutex_destroy(&mutex_);
			pthread_cond_destroy(&cond_);
		}

		void add(/**takes*/wque_work*item){
			pthread_mutex_lock(&mutex_);
			queue_.push_back(item);
			pthread_cond_signal(&cond_);
			pthread_mutex_unlock(&mutex_);
		}

		wque_work*remove(){
			pthread_mutex_lock(&mutex_);

			while(queue_.empty())
				pthread_cond_wait(&cond_,&mutex_);

			wque_work*item=queue_.back();

			queue_.pop_back();

			pthread_mutex_unlock(&mutex_);

			return item;
		}

		int size(){
			pthread_mutex_lock(&mutex_);
			int size=queue_.size();
			pthread_mutex_unlock(&mutex_);
			return size;
		}
	};

	//- --- - - - - - -------- --     --- - -- -- - - - -- - -- ----- - -- - - -- - -- -
	class wque_thread{
		wque&q_;
		pthread_t id_;
		static void*thread_run(void*arg){return((wque_thread*)arg)->run();}
	public:
		static atomic_int thread_count;

		inline wque_thread(wque&q):q_(q){
			if(pthread_create(&id_,NULL,thread_run,this))
				throw"could not create work queue tread";
			thread_count++;
		}

		inline virtual~wque_thread(){
			pthread_cancel(id_);
			thread_count--;
		}

		inline void*run(){
			while(true){
				wque_work*wrk=q_.remove();

				thread_count++;

				wrk->exec();

				thread_count--;

				delete wrk;
			}
		}

	};
	atomic_int wque_thread::thread_count{0};

}}
// lifted and modified from https://vichargrave.github.io/articles/2013-01/multithreaded-work-queue-in-cpp
