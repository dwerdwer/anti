#ifndef __UTIL_SPIN_LOCK__
#define __UTIL_SPIN_LOCK__

#include <pthread.h>

class spinlock_t
{
public:
    spinlock_t() {
        pthread_spin_init(&m_lock, PTHREAD_PROCESS_PRIVATE);
    }
    ~spinlock_t() {
        pthread_spin_destroy(&m_lock);
    }
    void lock() {
        pthread_spin_lock(&m_lock);
    }
    void unlock() {
        pthread_spin_unlock(&m_lock);
    }
private:
    pthread_spinlock_t m_lock;
};

#endif /* __UTIL_SPIN_LOCK__ */
