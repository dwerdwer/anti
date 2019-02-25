#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

#include <pthread.h>

#ifdef __linux__

class spinlock {
public:
    spinlock() : is_locked(false) {
        pthread_spin_init(&m_lock, PTHREAD_PROCESS_PRIVATE);
    }
    void lock() {
        if(is_locked == false) {
            pthread_spin_lock(&m_lock);
            is_locked = true;
        }
    }
    void unlock() {
        if(is_locked == true) {
            pthread_spin_unlock(&m_lock);
            is_locked = false;
        }
    }
    ~spinlock() {
        pthread_spin_destroy(&m_lock);
    }
private:
    bool is_locked;
    pthread_spinlock_t m_lock;
};

#else /* __linux__ */

class spinlock {
    spinlock() : is_locked(false) {
    }
    void lock() {
        if(is_locked == false) {
            pthread_mutex_lock(&m_lock);
            is_locked = true;
        }
    }
    void unlock() {
        if(is_locked == true) {
            pthread_mutex_unlock(&m_lock);
            is_locked = false;
        }
    }
    ~spinlock() {
        pthread_mutex_destroy(&m_lock);
    }
private:
    bool is_locked;
    pthread_mutex_t m_lock = PTHREAD_MUTEX_INITIALIZER;
};

#endif /* __linux__ */

#endif /* __SPINLOCK_H__ */
