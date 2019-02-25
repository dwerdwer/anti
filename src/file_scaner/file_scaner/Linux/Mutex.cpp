
#include "Mutex.h"
Mutex::Mutex(){
    pthread_mutex_init(&mMutex,NULL);
}
Mutex::~Mutex(){
    pthread_mutex_destroy(&mMutex);
}
void Mutex::enter(){
    pthread_mutex_lock(&mMutex);
}
void Mutex::leave(){
    pthread_mutex_unlock(&mMutex);
}
