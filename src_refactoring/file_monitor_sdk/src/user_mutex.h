#ifndef _SUK_MUTEX_
#define _SUK_MUTEX_

#include <pthread.h>

class mutex_t
{
public:
    mutex_t(){
        pthread_mutex_init(&mutex,NULL);
    }
    ~mutex_t(){
        pthread_mutex_destroy(&mutex);
    }
    void enter(){
        pthread_mutex_lock(&mutex);
    }
    void leave(){
        pthread_mutex_unlock(&mutex);
    }
private:
    pthread_mutex_t mutex;
};

#endif