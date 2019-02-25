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
        pthread_mutex_lock(&mMutex);
    }
    void leave(){
        pthread_mutex_unlock(&mMutex);
    }
};

#endif