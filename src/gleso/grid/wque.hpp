#pragma once
#include<pthread.h>
#include<list>

using namespace std;
namespace grid{
	template<typename T>class wque{
		list<T>queue_;
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
		void add(T item){
			pthread_mutex_lock(&mutex_);
			queue_.push_back(item);
			pthread_cond_signal(&cond_);
			pthread_mutex_unlock(&mutex_);
		}
		T remove(){
			pthread_mutex_lock(&mutex_);
			while(queue_.empty()){
				pthread_cond_wait(&cond_,&mutex_);
			}
			T item=queue_.front();
			queue_.pop_front();
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
}
// lifted and modified from https://vichargrave.github.io/articles/2013-01/multithreaded-work-queue-in-cpp
