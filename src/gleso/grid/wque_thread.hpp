#pragma once
#include"wque_work.hpp"
#include<atomic>
#include<pthread.h>

#include"wque.hpp"

namespace grid{

	class wque_thread{
		wque<wque_work*>&q_;
		pthread_t id_;
		static void*thread_run(void*arg){return((wque_thread*)arg)->run();}
	public:
		static atomic_int thread_count;

		inline wque_thread(wque<wque_work*>&q):q_(q){
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
}

