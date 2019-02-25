#ifndef SOCKETTHREAD_H
#define SOCKETTHREAD_H

#include "./mythread.h"
#include "./thread_mutecond.h"
#include "./threadpool.h"
#include "crpcserver.h"

class ThreadPool;

class CSocketThread : public CMyThread
{
public:
	CSocketThread();
	virtual ~CSocketThread();

public:
	void		SocketThread();
	//线程运行的接口
	void virtual 	Run();
	bool    	Wakeup();		//苏醒

								//添加新任务
	void		SetNewSocket(AccSock *pSocket);
	void		SetPool(ThreadPool* pPool) { m_pPool = pPool; m_blInPool = true;  }

	void		SetThreadPool(CThreadPool *pThreadPool) { m_pThreadPool = pThreadPool; }
	AccSock 	*GetSocket() const { return m_pSocket; }
	void		SetIdleThList(t_IdleThList *pIdleThList) { m_pIdleThList = pIdleThList; }
	void		SetBusyThList(t_BusyThList *pBusyThList) { m_pBusyThList = pBusyThList; }
	AccSock*    GetSockHandle() { return m_pSocket; }
	void        SetsockNull(){m_pSocket = NULL;}
	bool        IsThInPool(){ return m_blInPool; }
	void        SetIsThInPool(bool isInPool) { m_blInPool = isInPool; }

private:

	CMyThreadMutex m_mutex;		//
	CMyThreadCondition 	m_cond;
	AccSock 	*m_pSocket;
	CThreadPool	*m_pThreadPool;
	t_IdleThList	*m_pIdleThList;
	t_BusyThList	*m_pBusyThList;
	bool	m_blInPool;

	ThreadPool* m_pPool;
};

#endif
