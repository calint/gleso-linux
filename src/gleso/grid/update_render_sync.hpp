#pragma once
#include<pthread.h>
namespace grid{
	namespace update_render_sync{
		namespace work_to_do_count{
			bool _initiated=false;
			pthread_mutex_t mutex;
			//atomic<int>count;//? instead of mutex
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
			void decrease_and_notify_if_zero(){
				pthread_mutex_lock(&mutex);
				count--;
				if(count==0)
					pthread_cond_signal(&cond);
				pthread_mutex_unlock(&mutex);
			}
			void wait_until_count_is_zero(){
				pthread_mutex_lock(&mutex);
				while(count!=0){
					pthread_cond_wait(&cond,&mutex);
				}
				pthread_mutex_unlock(&mutex);
			}
			void set(int work_to_do_count){
				pthread_mutex_lock(&mutex);
				count=work_to_do_count;
				pthread_mutex_unlock(&mutex);
			}
		}
	}
}
