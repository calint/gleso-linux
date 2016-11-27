#pragma once
#include"../thread.hpp"
#include"../wqueue.hpp"
#include "wque_work.hpp"
#include<atomic>

namespace grid{
	class wque_thread:public thread{
		wqueue<wque_work*>&queue_;

	public:
		wque_thread(wqueue<wque_work*>&queue):queue_(queue){metrics::threads++;}
		~wque_thread(){metrics::threads--;}

		void*run(){
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
