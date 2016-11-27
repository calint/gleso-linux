// from: https://vichargrave.github.io/articles/2012-12/java-style-thread-class-in-cpp
#pragma once
#include<pthread.h>

static void*thread_run(void*arg);

class thread{
public:
	thread():id_(0),running_(0),detached_(0){}

	virtual~thread(){
		if(running_==1 and detached_==0)
			pthread_detach(id_);

		if(running_==1)
			pthread_cancel(id_);
	}

	int start(){
		int result=pthread_create(&id_,NULL,thread_run,this);

		if(result==0)
			running_=true;

		return result;
	}

	int join(){
		int result=-1;
		if(running_){
			result=pthread_join(id_,NULL);
			if(result==0)
				detached_=true;
		}
		return result;
	}

	int detach(){
		int result=-1;
		if(running_ and not detached_){
			result=pthread_detach(id_);
			if(result==0){
				detached_=true;
			}
		}
		return result;
	}

	pthread_t self(){return id_;}

	virtual void*run()=0;

private:
	pthread_t  id_;
	bool running_;
	bool detached_;
};

static void*thread_run(void*arg){return((thread*)arg)->run();}
