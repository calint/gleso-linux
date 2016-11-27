#pragma once
#include<pthread.h>
namespace grid{
	class update_render_sync{
		pthread_mutex_t mutex_;
		//atomic<int>count;//? instead of mutex
		int count_;
		pthread_cond_t cond_;
	public:
		update_render_sync():count_{0}{
			pthread_mutex_init(&mutex_,NULL);
			pthread_cond_init(&cond_,NULL);
		}
		~update_render_sync(){
			pthread_mutex_destroy(&mutex_);
			pthread_cond_destroy(&cond_);
		}
		void decrease_and_notify_if_zero(){
			pthread_mutex_lock(&mutex_);
			count_--;
			if(count_==0)
				pthread_cond_signal(&cond_);
			pthread_mutex_unlock(&mutex_);
		}
		void wait_until_count_is_zero(){
			pthread_mutex_lock(&mutex_);
			while(count_!=0){
				pthread_cond_wait(&cond_,&mutex_);
			}
			pthread_mutex_unlock(&mutex_);
		}
		void set(int work_to_do_count){
			pthread_mutex_lock(&mutex_);
			count_=work_to_do_count;
			pthread_mutex_unlock(&mutex_);
		}
	};
}
