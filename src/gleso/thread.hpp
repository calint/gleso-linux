// from: https://vichargrave.github.io/articles/2012-12/java-style-thread-class-in-cpp
#pragma once
#include <pthread.h>

static void*runThread(void*arg);

class thread{
public:
	thread():m_tid(0),m_running(0),m_detached(0){}

	virtual~thread(){
		if(m_running==1 and m_detached==0){
			pthread_detach(m_tid);
		}
		if(m_running==1){
			pthread_cancel(m_tid);
		}
	}

	int start(){
		int result=pthread_create(&m_tid,NULL,runThread,this);
		if(result==0){
			m_running=1;
		}
		return result;
	}

	int join(){
		int result=-1;
		if(m_running==1){
			result=pthread_join(m_tid,NULL);
			if (result==0){
				m_detached=1;
			}
		}
		return result;
	}

	int detach(){
		int result=-1;
		if(m_running==1 and m_detached==0){
			result=pthread_detach(m_tid);
			if(result==0){
				m_detached=1;
			}
		}
		return result;
	}

	pthread_t self(){return m_tid;}

	virtual void*run()=0;

private:
	pthread_t  m_tid;
	int m_running;
	int m_detached;
};

static void*runThread(void*arg){return((thread*)arg)->run();}
