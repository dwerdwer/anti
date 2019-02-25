/**************************************************************
* SocketThread.cpp : implementation file
* Copyright 2004-2005 TianMuLingHang
* All rights reserved.
* Author: Zeng Wenchuan
* Date:   Jan 9, 2005
* Modify:
* Date:
* Email:  zeng_aven@163.com
* Site:   http://zeng_aven.go.nease.net/
*************************************************************/

#include "./SocketThread.h"
//#include "../crpcserver.h"

CSocketThread::CSocketThread() :
	m_pSocket(NULL), m_pThreadPool(NULL)
{
	m_blInPool = false;
	m_pPool = NULL;
	m_pIdleThList = NULL;
	m_pBusyThList = NULL;
}

CSocketThread::~CSocketThread()
{

}


bool  CSocketThread::Wakeup()
{
	//if (pthread_cond_signal(&m_cond) == 0) 
	if (m_cond.Condition_Signal())
	{
		//m_State = Thread_Idle;
		//SetState(Thread_Idle);
		return true;
	}
	else
	{
		return false;
	}

}


void CSocketThread::Run()
{
	SocketThread();
}

void CSocketThread::SocketThread()
{
	for (;;)
	{
		if ((NULL == m_pSocket)||(m_pSocket->Isclosed()))
		{
			if (Thread_Busy == GetState()) 
			{
				SetState(Thread_Idle);
			}
			
			if(m_blInPool)
			{
				if (m_pThreadPool!=NULL)
				m_pThreadPool->MoveToIdleList(this);
			}
			else
			{
			}
			//pthread_mutex_lock(&m_mutex);
			m_mutex.Lock();

			//pthread_cond_wait(&m_cond, &m_mutex);
			m_cond.Condition_Wait(&m_mutex);

			//m_pSocket->SelectMsg();
			m_pSocket->OnReceive();

			//m_pThreadPool->MoveToIdleList(this);	
			//SetState(Thread_Idle);
			//pthread_mutex_unlock(&m_mutex);
			m_mutex.Unlock();
		}
		else
		{
			m_pSocket->OnReceive();
		}

		// 释放不在线程池中的额外创建的线程资源
		if (! m_pSocket->Is_valid())
		{
			SetState(Thread_Idle);

			if (!m_blInPool)
			{
				delete m_pSocket;
				delete this;
				break;
			}
			delete m_pSocket;
			m_pSocket = NULL;
		}
	}
}

void CSocketThread::SetNewSocket(AccSock *pSocket)
{
	m_pSocket = pSocket;
	Wakeup();
}
