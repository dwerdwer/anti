
#include "./threadpool.h"
#include "./SocketThread.h"


#include <map>
using namespace std;

CThreadPool::CThreadPool(int num) :
	m_nMaxThreadNum(MAX_THREAD_NUM), m_nWaitNec(30000000), m_nThreadNum(0)

{
	//pthread_mutex_init(&m_BusyMutex, NULL);
	//pthread_mutex_init(&m_IdleMutex, NULL);
	//pthread_mutex_init(&m_WaitMutex, NULL);
	//pthread_cond_init(&m_WaitCond, NULL);

	if (num > MAX_THREAD_NUM)
		num = MAX_THREAD_NUM;

	for (int i = 0; i<num; i++)
	{
		CreateNewThread();
	}

	m_nNormalThread = m_nThreadNum;
}

CThreadPool::CThreadPool()
{
	//pthread_mutex_init(&m_BusyMutex, NULL);
	//pthread_mutex_init(&m_IdleMutex, NULL);
}

CThreadPool::~CThreadPool()
{
	//pthread_mutex_unlock(&m_IdleMutex);
	m_IdleMutex.Unlock();

	for (t_iIdleThList iIdle = m_IdleThList.begin(); iIdle != m_IdleThList.end(); iIdle++)
	{
		delete	*iIdle;
		*iIdle = NULL;
	}
	//pthread_mutex_unlock(&m_IdleMutex);
	m_IdleMutex.Unlock();

	//pthread_mutex_lock(&m_BusyMutex);
	m_BusyMutex.Lock();

	for (t_iBusyThList iBusy = m_BusyThList.begin(); iBusy != m_BusyThList.end(); iBusy++)
	{
		delete	*iBusy;
		*iBusy = NULL;
	}

	m_BusyThList.clear();
	//pthread_mutex_unlock(&m_BusyMutex);
	m_BusyMutex.Unlock();

	//pthread_mutex_destroy(&m_BusyMutex);
	//pthread_mutex_destroy(&m_IdleMutex);
}



void  CThreadPool::CreateNewThread()
{
	CSocketThread *pThread = new CSocketThread;
	if (pThread->Start()) {
		//pthread_mutex_lock(&m_IdleMutex);
		m_IdleMutex.Lock();

		pThread->SetIdleThList(&m_IdleThList);
		pThread->SetBusyThList(&m_BusyThList);
		pThread->SetThreadPool(this);
		m_IdleThList.push_back(pThread);
		pThread->SetState(Thread_Idle);
		m_nThreadNum++;

		//pthread_mutex_unlock(&m_IdleMutex);
		m_IdleMutex.Unlock();
	}
	else
		delete pThread;
}

CSocketThread *CThreadPool::GetIdleThread()
{
	if (GetIdleNum() > 0) {
		//pthread_mutex_lock(&m_IdleMutex);
		m_IdleMutex.Lock();

		CSocketThread	*pThread = (CSocketThread *)m_IdleThList.front();
		//pthread_mutex_unlock(&m_IdleMutex);
		m_IdleMutex.Unlock();

		//pThread->m_blInPool = true;
		pThread->SetIsThInPool(true);

		return 	pThread;
	}

	return NULL;
}

void CThreadPool::MoveToIdleList(CSocketThread *pThread)
{
	//pthread_mutex_lock(&m_IdleMutex);
	m_IdleMutex.Lock();

	m_IdleThList.push_back(pThread);
	//pthread_mutex_unlock(&m_IdleMutex);
	m_IdleMutex.Unlock();

	if (GetBusyNum() <= 0)
		return;

	//pthread_mutex_lock(&m_BusyMutex);
	m_BusyMutex.Lock();

	m_BusyThList.remove(pThread);

	//pthread_mutex_unlock(&m_BusyMutex);
	m_BusyMutex.Unlock();
}

void CThreadPool::MoveToBusyList(CSocketThread *pThread)
{
	//pthread_mutex_lock(&m_BusyMutex);
	m_BusyMutex.Lock();

	m_BusyThList.push_back(pThread);
	//pthread_mutex_unlock(&m_BusyMutex);
	m_BusyMutex.Unlock();

	//if (GetBusyNum() <= 0)
	//	return;

	//pthread_mutex_lock(&m_IdleMutex);
	m_IdleMutex.Lock();

	m_IdleThList.remove(pThread);
	//pthread_mutex_unlock(&m_IdleMutex);
	m_IdleMutex.Unlock();
}

int CThreadPool::GetBusyNum()
{
	//pthread_mutex_lock(&m_BusyMutex);
	m_BusyMutex.Lock();

	int num = m_BusyThList.size();

	//pthread_mutex_unlock(&m_BusyMutex);
	m_BusyMutex.Unlock();
	return num;
}

int CThreadPool::GetIdleNum()
{
	//pthread_mutex_lock(&m_IdleMutex);
	m_IdleMutex.Lock();

	int num = m_IdleThList.size();
	//pthread_mutex_unlock(&m_IdleMutex);
	m_IdleMutex.Unlock();

	return num;
}

void CThreadPool::CreateIdleThread(const int num)
{

	//pthread_mutex_lock(&m_IdleMutex);
	m_IdleMutex.Lock();

	if (m_nThreadNum == m_nMaxThreadNum)
		return;
	if (m_nThreadNum + num > m_nMaxThreadNum) {
		CreateIdleThread(m_nMaxThreadNum - m_nThreadNum);
	}

	//pthread_mutex_unlock(&m_IdleMutex);
	m_IdleMutex.Unlock();

}

void CThreadPool::DeleteIdleThread()
{

	//pthread_mutex_lock(&m_IdleMutex);
	m_IdleMutex.Lock();

	m_IdleThList.pop_back();
	//pthread_mutex_unlock(&m_IdleMutex);
	m_IdleMutex.Unlock();

	//delete               wangzheng

}

void CThreadPool::Wait()
{

	//pthread_mutex_lock(&m_WaitMutex);
	m_WaitMutex.Lock();

	//m_delay.tv_nsec = m_nWaitNec;
	m_WaitCond.Condition_TimeWait(m_nWaitNec, &m_WaitMutex);
	//pthread_cond_timedwait(&m_WaitCond, &m_WaitMutex, &m_delay);


	//pthread_mutex_unlock(&m_WaitMutex);
	m_WaitMutex.Unlock();

}

void CThreadPool::GetWaitTime(int &nWaitSec, long &nWaitNec)
{
	//nWaitSec	= m_nWaitSec;
	nWaitNec = m_nWaitNec;
}

void CThreadPool::SetWaitTime(const int nWaitSec, const long nWaitNec)
{
	//m_nWaitSec	= nWaitSec;
	m_nWaitNec = nWaitNec;
}

