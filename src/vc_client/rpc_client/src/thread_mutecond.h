#ifndef _THREAD_MUTECOND_H_  
#define _THREAD_MUTECOND_H_ 


#ifdef _WIN32
typedef void *   Mutex_Handle;
typedef  void *  CONDITION;
#else
#include <pthread.h>
typedef pthread_mutex_t   Mutex_Handle;
typedef pthread_cond_t     CONDITION;
#endif  


class CMyThreadMutex
{
public:
	CMyThreadMutex();
	~CMyThreadMutex();
	Mutex_Handle* GetHandle() { return &m_hMutex; }
	int Lock();
	int TryLock(unsigned int dwMilliseconds);
	void Unlock();
private:
	Mutex_Handle m_hMutex;
};


// begin
class CMyThreadCondition
{
public:
	CMyThreadCondition();
	~CMyThreadCondition();

	bool Condition_Init();
	bool Condition_Destroy();
	
	bool Condition_TimeWait(long m_nWaitNec, CMyThreadMutex *mutex);
	bool Condition_Wait(CMyThreadMutex *mutex);
	bool Condition_Signal();

private:
	CONDITION m_cond;
};



#endif
