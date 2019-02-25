#ifndef __UTIL_SPIN_LOCK__
#define __UTIL_SPIN_LOCK__

#include <pthread.h>

namespace util {

class spinlock_t
{
public:
    spinlock_t(int pshared = PTHREAD_PROCESS_PRIVATE)
    {
        pthread_spin_init(&m_lock, pshared);
    }
    ~spinlock_t()
    {
        pthread_spin_destroy(&m_lock);
    }
    void lock()
    {
        pthread_spin_lock(&m_lock);
    }
    void unlock()
    {
        pthread_spin_unlock(&m_lock);
    }
private:
    pthread_spinlock_t m_lock;
};

class rwlock_t
{
public:
    rwlock_t(const pthread_rwlockattr_t *attr = NULL)
    {
        pthread_rwlock_init(&m_lock, attr);
    }
    ~rwlock_t()
    {
        pthread_rwlock_destroy(&m_lock);
    }
    void read()
    {
        pthread_rwlock_rdlock(&m_lock);
    }
    void write()
    {
        pthread_rwlock_wrlock(&m_lock);
    }
    void unlock()
    {
        pthread_rwlock_unlock(&m_lock);
    }
private:
    pthread_rwlock_t m_lock;
};

class CriticalSection
{
public:
    CriticalSection(spinlock_t *p_lock) :
      p_spin(p_lock)
    {
        p_spin->lock();
    }
    CriticalSection(rwlock_t *p_lock, bool isread = false) :
      p_rw(p_lock)
    {
        if(isread)
        {
            this->p_rw->read();
        }
        else
        {
            this->p_rw->write();
        }
    }
    ~CriticalSection()
    {
        if(this->p_spin)
        {
            this->p_spin->unlock();
        }
        if(this->p_rw)
        {
            this->p_rw->unlock();
        }
    }
private:
    spinlock_t  *p_spin = NULL;
    rwlock_t    *p_rw = NULL;
};

} /* namespace util */

#endif /* __UTIL_SPIN_LOCK__ */
