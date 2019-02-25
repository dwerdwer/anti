#ifndef _SUK_MUTEX_
#define _SUK_MUTEX_
#include <pthread.h>

class Mutex{
private:
    pthread_mutex_t mMutex;
public:
    Mutex(){
        pthread_mutex_init(&mMutex,NULL);
    }
    ~Mutex(){
        pthread_mutex_destroy(&mMutex);
    }
    void enter(){
        int ret = pthread_mutex_lock(&mMutex);
		if (ret == 0)
			printf("locked\n");
    }
    void leave(){
        int ret = pthread_mutex_unlock(&mMutex);
		if (ret == 0)
			printf("unlocked\n");
    }
};

#endif
