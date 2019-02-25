/**************************************************************
* CThreadPool.h
* Copyright 2004-2005 	Aven
* All rights reserved.
* 线程的状态可以分为四种，空闲、忙碌、挂起、终止(包括正常退出和非正常退出)
* 任务的执行都在ThreadProc开始，苏醒挂起的线程，直到任务完成，线程继续挂起，
* 一旦任务执行进入线程池，将无法取消它。线程池工作保持空闲线程的数量在突发
* 中可以应付，如果空闲线程太多就会删除部分空闲线程，如果不够，就会创建一定
* 数量的线程。
* Contact info:
* -------------
* Site:   	   	http://zeng_aven.go.nease.net/
* paid services:	zeng_aven@163.com
*************************************************************/
#ifndef THREADPOOL_H
#define	THREADPOOL_H

#include <list>
#include <vector>
#include <iterator>
#include <algorithm>
#include <iostream>
//#include <unistd.h>
//#include <pthread.h>
//#include "ErrorLog.h"

#include "./mythread.h"

#include "./thread_mutecond.h"


using namespace std;

#define	MAX_THREAD_NUM	600
#define CTHREADPOOL	"CThreadPool"

class CSocketThread;
class CRPCServer;

typedef list<CSocketThread *>	t_IdleThList;
typedef t_IdleThList::iterator	t_iIdleThList;
typedef list<CSocketThread *>	t_BusyThList;
typedef t_BusyThList::iterator	t_iBusyThList;

class CThreadPool
{
public:
	CThreadPool();
	CThreadPool(int num);
	virtual ~CThreadPool();

public:
	//创建新的线程
	void		CreateNewThread();
	void		CreateIdleThread(const int num);
	//删除空闲线程
	void		DeleteIdleThread();
	//获取空闲线程句柄
	CSocketThread	*GetIdleThread();
	void		MoveToIdleList(CSocketThread *pThread);
	void		MoveToBusyList(CSocketThread *pThread);
	//超时等待
	void		Wait();

	//线程处理函数
	bool		IsInitialized() const { return m_IdleThList.size() > 0; }

public:
	void		GetWaitTime(int &m_nWaitSec, long &m_nWaitNsec);
	void		SetWaitTime(const int nWaitSec, const long nWaitNsec);
	int		GetBusyNum();
	int		GetIdleNum();
	int		GetThreadNum() const { return m_nThreadNum; }
	int		GetMaxJobsPending() const { return m_nMaxJobsPending; }
	int		GetMaxNormalThread() const { return m_nNormalThread; }
	int		GetMaxIdleNum() const { return m_nMaxIdleNum; }
	int		GetMinIdleNum() const { return m_nMinIdleNum; }
	int		GetMaxThreadNum() const { return m_nMaxThreadNum; }
	void		SetMaxThreadNum(const int num) { m_nMaxThreadNum = num; }
	void		SetMaxJobsPending(const int num) { m_nMaxJobsPending = num; }
	void		SetMaxNormalThread(const int num) { m_nNormalThread = num; }
	void		SetMaxIdleNum(const int num) { m_nMaxIdleNum = num; }
	void		SetMinIdleNum(const int num) { m_nMinIdleNum = num; }

private:
	//最大允许线程数
	int		m_nMaxThreadNum;
	//实际线程总数
	int		m_nThreadNum;
	//最大并行任务数
	int		m_nMaxJobsPending;
	//最大空闲线程数
	int		m_nMaxIdleNum;
	//最小空闲线程数
	int		m_nMinIdleNum;
	//正常线程总数
	int		m_nNormalThread;
	//等待时间（秒）
	//int		m_nWaitSec;
	//等待时间（纳秒）
	long		m_nWaitNec;
	//正在忙碌的线程链表
	t_BusyThList 	m_BusyThList;
	//空闲线程链表
	t_IdleThList 	m_IdleThList;

	//pthread_mutex_t m_IdleMutex;
	//pthread_mutex_t m_BusyMutex;
	//pthread_mutex_t m_WaitMutex;	

	CMyThreadMutex m_IdleMutex;
	CMyThreadMutex m_BusyMutex;
	CMyThreadMutex m_WaitMutex;

	CMyThreadCondition 	m_WaitCond;
	//struct timespec m_delay;
	//DWORD  m_delay_Milliseconds;
};

#endif
