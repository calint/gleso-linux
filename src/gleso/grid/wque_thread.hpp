#pragma once
#include"wque_work.hpp"
#include<atomic>
#include<pthread.h>

#include"wque.hpp"

static void*thread_run(void*arg);

namespace grid{
	class wque_thread{
		wque<wque_work*>&queue_;
		pthread_t id_;
	public:

		inline wque_thread(wque<wque_work*>&queue):
			queue_(queue)
		{
			if(pthread_create(&id_,NULL,thread_run,this))throw"could not create work queue tread";
			metrics::threads++;
		}

		inline virtual~wque_thread(){
			pthread_cancel(id_);
			metrics::threads--;
		}

		inline void*run(){
			while(true){
				wque_work*wrk=queue_.remove();
				threads_running_count++;
				wrk->exec();
				threads_running_count--;
				delete wrk;
			}
		}

		static atomic_int threads_running_count;
	};
	atomic_int wque_thread::threads_running_count{0};
}

static void*thread_run(void*arg){return((grid::wque_thread*)arg)->run();}
