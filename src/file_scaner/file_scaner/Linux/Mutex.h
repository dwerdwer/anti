
#ifndef _MUTEX_
#define _MUTEX_
#include <pthread.h>
class Mutex{
private:
    pthread_mutex_t mMutex;
public:
    Mutex();
    
    ~Mutex();
    void enter();
    void leave();
};

#endif