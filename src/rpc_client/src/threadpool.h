/**************************************************************
* CThreadPool.h
* Copyright 2004-2005 	Aven
* All rights reserved.
* �̵߳�״̬���Է�Ϊ���֣����С�æµ��������ֹ(���������˳��ͷ������˳�)
* �����ִ�ж���ThreadProc��ʼ�����ѹ�����̣߳�ֱ��������ɣ��̼߳�������
* һ������ִ�н����̳߳أ����޷�ȡ�������̳߳ع������ֿ����̵߳�������ͻ��
* �п���Ӧ������������߳�̫��ͻ�ɾ�����ֿ����̣߳�����������ͻᴴ��һ��
* �������̡߳�
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
	//�����µ��߳�
	void		CreateNewThread();
	void		CreateIdleThread(const int num);
	//ɾ�������߳�
	void		DeleteIdleThread();
	//��ȡ�����߳̾��
	CSocketThread	*GetIdleThread();
	void		MoveToIdleList(CSocketThread *pThread);
	void		MoveToBusyList(CSocketThread *pThread);
	//��ʱ�ȴ�
	void		Wait();

	//�̴߳�����
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
	//��������߳���
	int		m_nMaxThreadNum;
	//ʵ���߳�����
	int		m_nThreadNum;
	//�����������
	int		m_nMaxJobsPending;
	//�������߳���
	int		m_nMaxIdleNum;
	//��С�����߳���
	int		m_nMinIdleNum;
	//�����߳�����
	int		m_nNormalThread;
	//�ȴ�ʱ�䣨�룩
	//int		m_nWaitSec;
	//�ȴ�ʱ�䣨���룩
	long		m_nWaitNec;
	//����æµ���߳�����
	t_BusyThList 	m_BusyThList;
	//�����߳�����
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
