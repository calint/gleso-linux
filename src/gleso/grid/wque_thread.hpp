#pragma once
#include"thread.hpp"
#include"wque_work.hpp"
#include<atomic>
#include "wque.hpp"

namespace grid{
	class wque_thread:public thread{
		wque<wque_work*>&queue_;

	public:

		inline wque_thread(wque<wque_work*>&queue):
			queue_(queue)
		{
			metrics::threads++;
		}

		inline virtual~wque_thread(){
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
