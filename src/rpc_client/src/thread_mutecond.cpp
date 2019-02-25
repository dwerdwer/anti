

#include "./thread_mutecond.h"
#include "cmytimespan.h"

#ifdef _WIN32  
#include <windows.h>  
#include <process.h>

#else
 
#include <pthread.h>  
#include <unistd.h>  
#include <sys/time.h>  
#include <stdio.h>  
#include <errno.h>
#endif


CMyThreadCondition::CMyThreadCondition()
{
	Condition_Init();
}

CMyThreadCondition::~CMyThreadCondition()
{
	Condition_Destroy();
}


#ifndef _WIN32  

CMyThreadMutex::CMyThreadMutex()
{
	pthread_mutex_init(&m_hMutex, NULL);
}
CMyThreadMutex::~CMyThreadMutex()
{
	pthread_mutex_destroy(&m_hMutex);
}
int CMyThreadMutex::Lock()
{
	return pthread_mutex_lock(&m_hMutex) == 0 ? 0 : -1;
}
int CMyThreadMutex::TryLock(unsigned int dwMilliseconds)
{
	// The function pthread_mutex_trylock() returns zero if a lock on the mutex object referenced by mutex is acquired. Otherwise, an error number is returned to indicate the error.  
	unsigned int us = dwMilliseconds * 1000;
	int rt = pthread_mutex_trylock(&m_hMutex);
	if (rt == EBUSY)
	{
		CMyTimeSpan start;
		while (rt == EBUSY)
		{
			if (start.GetSpaninMilliseconds()>dwMilliseconds)
			{
				rt = -1;
			}
			else
			{
				usleep(20000);         //sleep  20ms  
				rt = pthread_mutex_trylock(&m_hMutex);
			}
		}
	}
	return rt;
	
}

void CMyThreadMutex::Unlock()
{
	pthread_mutex_unlock(&m_hMutex);
}

//===============================================================================
bool CMyThreadCondition::Condition_Init()
{
	int rt = 0;

	rt = pthread_cond_init(&m_cond,NULL);

	if (0 == rt)
	{
	    return true;
	}
	else
	{
	    return false;
	}
}


bool CMyThreadCondition::Condition_Destroy()
{
	int rt = 0;

	rt = pthread_cond_destroy(&m_cond);

	if(0 == rt)
	{
	    return true;
	}
	else
	{
	    return false;
	}
}


bool CMyThreadCondition::Condition_TimeWait(long m_nWaitNec, CMyThreadMutex *mutex)
{
	int rt = 0;
	struct timespec m_delay;

	m_delay.tv_sec = 0;
	m_delay.tv_nsec = m_nWaitNec;

	//rt = pthread_cond_wait(&m_cond,&mutex);
	rt = pthread_cond_timedwait(&m_cond,mutex->GetHandle(),&m_delay);

	if (0 == rt)
	{
	    return true;
	}
	else
	{
	    return false;
	}
}

bool CMyThreadCondition::Condition_Wait(CMyThreadMutex *mutex)
{
	int rt = 0;

	rt = pthread_cond_wait(&m_cond,mutex->GetHandle());

	if (0 == rt)
	{
	    return true;
	}
	else
	{
	    return false;
	}
}




bool CMyThreadCondition::Condition_Signal()
{
	int rt = 0;

	rt = pthread_cond_signal(&m_cond);

	if (0 == rt)
	{
	    return true;
	}
	else
	{
	    return false;
	}
}





#else  // WIN32

CMyThreadMutex::CMyThreadMutex()
{
	m_hMutex = CreateMutexA(NULL, FALSE, NULL);
}

CMyThreadMutex::~CMyThreadMutex()
{
	if (m_hMutex)   CloseHandle(m_hMutex);
}
int CMyThreadMutex::Lock()
{
	if (m_hMutex && WaitForSingleObject(m_hMutex, INFINITE) == WAIT_OBJECT_0) return 0;
	return -1;
}
int CMyThreadMutex::TryLock(unsigned int dwMilliseconds)
{
	if (m_hMutex&& WaitForSingleObject(m_hMutex, dwMilliseconds) == WAIT_OBJECT_0) return 0;

	return -1;

}
void CMyThreadMutex::Unlock()
{
	if (m_hMutex) ReleaseMutex(m_hMutex);
}


//===============================================================================
bool CMyThreadCondition::Condition_Init()
{

	m_cond = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (NULL == m_cond)
	{
	    return FALSE;
	}
	else
	{
	    return TRUE;
	}
}


bool CMyThreadCondition::Condition_Destroy()
{
	int rt = 0;

	rt = CloseHandle(m_cond);

	if(0 == rt)
	{
	    return FALSE;
	}
	else
	{
	    return TRUE;
	}
}


bool CMyThreadCondition::Condition_TimeWait(long m_nWaitNec, CMyThreadMutex *mutex)
{
	bool rt = TRUE;

	mutex->Unlock();

	rt = WaitForSingleObject(m_cond, (DWORD)m_nWaitNec);

	if(WAIT_OBJECT_0 != rt)
	{
	    return FALSE;
	}
	mutex->Lock();
	return rt;
}

bool CMyThreadCondition::Condition_Wait(CMyThreadMutex *mutex)
{
	bool rt = TRUE;

	mutex->Unlock();

	rt = WaitForSingleObject(m_cond, INFINITE);

	if(WAIT_OBJECT_0 != rt)
	{
	    return FALSE;
	}
	mutex->Lock();
	return rt;
}



bool CMyThreadCondition::Condition_Signal()
{
	int rt = 0;

	rt = SetEvent(m_cond);

	if(0 == rt)
	{
	    return FALSE;
	}
	else
	{
	    return TRUE;
	}
}



#endif

