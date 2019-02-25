//#include "stdafx.h"    //wangzheng

#include <stdio.h>  
#include <time.h>  
#include <string.h>  
#include "mythread.h"


#include "cmytimespan.h"


#ifdef _WIN32 

#include <windows.h>  
#include <process.h>    /* _beginthread, _endthread */  
#define gxstrcpy(d,n,s) strcpy_s(d,n,s)  

#else

#include <pthread.h>  
#include <unistd.h>  
#include <sys/time.h>  
#include <stdio.h>  
#define gxstrcpy(d,n,s) strncpy(d,s,n)  
#define THREAD_IDLE_TIMESLICE_MS   20  
#endif  

#define GX_UNDEFINED      0xffffffff  
#define GX_S_OK           0x00000000 

#ifndef _WIN32
void *ThreadRun(void *pvThread)
{
	CMyThread *pThread = (CMyThread *)pvThread;
	
	pThread->Run();
	
	//pthread_exit(0);
	pThread->EndThread();
}

#else  // WIN32

void ThreadRun(void *pvThread)
{
	CMyThread *pThread = (CMyThread *)pvThread;

	pThread->Run();

	//pthread_exit(0);
	pThread->EndThread();
}
#endif


CMyThread::CMyThread()
{
	SetState(Thread_Idle);
	m_ThreadID = 0;
}

CMyThread::~CMyThread()
{
	//Terminate();
}









#ifndef _WIN32


typedef struct
{
	void(*proc)(void *);
	void * pargs;
} _threadwraper_linux_t;

void * _ThreadWraper_Linux(void *pargs)
{
	_threadwraper_linux_t *pth = (_threadwraper_linux_t *)pargs;
	pth->proc(pth->pargs);
	delete[] pth;
	return NULL;
}
/*
int CMyThread::CreateThread(void(*proc)(void *), void *pargs)
{
	pthread_t ntid;
	_threadwraper_linux_t* pthreadwraper = new _threadwraper_linux_t[1];
	pthreadwraper[0].proc = proc;
	pthreadwraper[0].pargs = pargs;
	return pthread_create(&ntid, NULL, _ThreadWraper_Linux, pthreadwraper);
}
*/

bool  CMyThread::Start()
{

	int nRet	= pthread_create(&m_ThreadID, NULL, &ThreadRun, (void *)this);
	if (0 != nRet) 
	{
		//std::cout<< "create the thread error !!!" << endl;
		return false;
	}
	else
	{
		m_State = Thread_Hup;
		return true;
	}
}

bool  CMyThread::Terminate()
{
	if (pthread_cancel(m_ThreadID) == 0) 
	{
		m_State = Thread_Exit;
		return true;
	}

}



void CMyThread::DiscardTimeSlice()
{
	usleep(THREAD_IDLE_TIMESLICE_MS * 1000);
}


void CMyThread::EndThread()
{
	pthread_exit(NULL);
}

unsigned int CMyThread::GetCurrentThreadId()
{
	return pthread_self();
}
void CMyThread::Sleep(unsigned int milliseconds)
{
	if (milliseconds >= 1000)
	{
		unsigned int s = milliseconds / 1000;
		unsigned int us = milliseconds - s * 1000;
		sleep(s);
		if (us>0)  usleep(us * 1000);
	}
	else
	{
		usleep(milliseconds * 1000);
	}
}
//=====================================================================================  


#else  //WIN32




/********************************************************************************
int CMyThread::CreateThread(void(*proc)(void *), void *pargs)
{
	return _beginthread(proc, 0, pargs);
}
*********************************************************************************/

void CMyThread::EndThread()
{
	_endthread();
}


bool  CMyThread::Start()
{
	
	m_ThreadID = (unsigned long)_beginthread(&ThreadRun, 0, (void *)this);   //wangzheng
	if(-1 == m_ThreadID)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
		
	}
}

bool  CMyThread::Terminate()
{
       bool rt = FALSE;
	//closehandle(m_ThreadID);
	rt = TerminateThread((HANDLE)m_ThreadID,0);
	
	if(0 == rt)
	{
		return FALSE;
	}
	else
	{
		m_State = Thread_Exit;
		return TRUE;
		
	}
}


unsigned int CMyThread::GetCurrentThreadId()
{
	return ::GetCurrentThreadId();
}
void CMyThread::Sleep(unsigned int miniseconds)
{
	::Sleep(miniseconds);
}

void CMyThread::DiscardTimeSlice()
{
	::SwitchToThread();
}
//=====================================================================================  
//=====================================================================================  


#endif  

//=====================================================================================================  
#if 0
CThreadError::CThreadError()
{
	m_pStart = NULL;
}
CThreadError::~CThreadError()
{
	internal_thread_error_t *temp;
	while (m_pStart)
	{
		temp = m_pStart;
		m_pStart = (internal_thread_error_t*)m_pStart->next;
		delete temp;
	}
}
void CThreadError::operator=(int errorno)
{
	unsigned int tid = CMyThread::GetCurrentThreadId();
	internal_thread_error_t *temp = search(tid);
	if (!temp)
	{
		temp = allocMemory(tid);
	}
	temp->threaderror.errorno = errorno;
	temp->threaderror.errormsg[0] = '\0';
}
void CThreadError::operator=(const char * msg)
{
	unsigned int tid = CMyThread::GetCurrentThreadId();
	internal_thread_error_t *temp = search(tid);
	if (!temp)
	{
		temp = allocMemory(tid);
	}
	temp->threaderror.errorno = GX_UNDEFINED;
	gxstrcpy(temp->threaderror.errormsg, 510, msg);
}
void CThreadError::operator=(thread_error_t & st)
{
	unsigned int tid = CMyThread::GetCurrentThreadId();
	internal_thread_error_t *temp = search(tid);
	if (!temp)
	{
		temp = allocMemory(tid);
	}
	memcpy(&temp->threaderror, &st, sizeof(thread_error_t));
}
unsigned int CThreadError::GetLastErrorNo()
{
	unsigned int tid = CMyThread::GetCurrentThreadId();
	internal_thread_error_t *temp = search(tid);
	return temp ? temp->threaderror.errorno : GX_S_OK;
}
const char *CThreadError::GetLastErrorMsg()
{
	unsigned int tid = CMyThread::GetCurrentThreadId();
	internal_thread_error_t *temp = search(tid);
	return temp ? (const char*)temp->threaderror.errormsg : NULL;
}
const thread_error_t *CThreadError::GetLastErrorStruct()
{
	unsigned int tid = CMyThread::GetCurrentThreadId();
	internal_thread_error_t *temp = search(tid);
	return temp ? (const thread_error_t *)(&(temp->threaderror)) : NULL;
}

CThreadError::internal_thread_error_t* CThreadError::allocMemory(unsigned int tid)
{
	internal_thread_error_t *temp = new internal_thread_error_t;
	temp->threadid = tid;
	temp->next = m_pStart;
	m_pStart = temp;
	return temp;
}
CThreadError::internal_thread_error_t * CThreadError::search(unsigned int tid)
{
	internal_thread_error_t *temp = m_pStart;
	while (temp)
	{
		if (temp->threadid == tid) break;
		temp->next = (void *)temp;
	}
	return temp;
}

#endif
