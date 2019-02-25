#ifndef _CRPCSERVER_
#define _CRPCSERVER_

#include "tcpserver.h"
#include "threadpool.h"
#include "crpc_accept_socket.h"

#include <map>
using namespace std;

#ifdef _WIN32  
#include <windows.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable:4996)
#endif


typedef list<CSocketThread *>	t_AlloThList;
typedef t_AlloThList::iterator	t_iAlloThList;


//class CThreadPool;

// int svc_reg(unsigned int comm_no, void(*func)(void* data));

void main_run(IMPLSERVERPTR pServer);


class ThreadPool
{
private:
	CMyThread** m_plpThread;
	int    m_count;
	CMyThreadMutex m_mutex;

public:
	ThreadPool()
	{
		m_plpThread = NULL;
		m_count = 0;
	}
	~ThreadPool()
	{
		delete m_plpThread;
		m_count = 0;
	}

	void InitPool(int maxCount, void* para )
	{
		m_plpThread = new CMyThread*[maxCount];
		m_count = maxCount;
		for (int i = 0; i < m_count; i++)
		{
			m_plpThread[i] = new_Thread(para);
		}
	}
	virtual CMyThread* new_Thread(void* para) = 0;

	CMyThread * FindIdle()
	{
		CMyThread* pThreadFind = NULL;
		m_mutex.Lock();
		for (int i = 0; i < m_count; i++)
		{
			CMyThread* pThread = (CMyThread*)m_plpThread[i];
			if (pThread->GetState() == Thread_Idle)
			{
				pThread->SetState(Thread_Busy);
				pThreadFind = pThread;
				break;
			}
		}
		m_mutex.Unlock();
		return pThreadFind;
	}

	void Stop()
	{
		for (int i = 0; i < m_count; i++)
		{
			m_plpThread[i]->Terminate();
			delete m_plpThread[i];
		}
	}
};

class CSocketThread;

class CRPCServer : public TCPServer, public CMyThread, ThreadPool
{
public:
	CRPCServer(int port, IRpcServer* pIRpcServer, int ThreadPoolSize);
	CRPCServer(HSocket accept_sock);
	~CRPCServer();

	void Close();

	//每个新描述符都要加入链表
	//bool 		AddSockInList();          //wangzheng
	bool		AddAcceptInList();

	//int OnRecv(IRpcConnection* hConnection, DATAPTR data );

	int OnError() {}

	//void Init_IRPCConnection();

	//bool OnReceive();
	void sock_assign_thread(AccSock *psock);

	void sock_assign_threadPool(AccSock *psock);

	bool		InitThreadPool(int nThreadNum);
	AccSock *	OnAccept();
	void		BeginThread();
	//多路复用处理
	void		SelectMsg();

	virtual	void Run(void);

	virtual CMyThread* new_Thread(void* para);
protected:
	//判断套接字是否有东西到来
	class SocketIsSet
	{
		friend class CRPCServer;
	public:
		SocketIsSet(fd_set &rfd) : m_rfd(&rfd) {}
		~SocketIsSet() {}

		bool	operator ()(int &sockfd)
		{
			return FD_ISSET(sockfd, m_rfd);
		}

	private:
		fd_set	*m_rfd;

	};

public:
	void  SetThreadPool(CThreadPool *pThread) { m_pThreadPool = pThread; }
	CThreadPool	*GetThreadPool() const { return m_pThreadPool; }
	
	t_AlloThList	m_AlloThList;


private:

private:
	//套接字标识符
	//int		m_sock;
	//sockaddr_in 	m_addr;
	int		m_dsize;
	bool		m_bBusy;

	//描述符链表
	//保存创建套接字的描述符
	//static	t_listSocket	m_listSock;
	//static	t_mapSocket	m_mapSock;

	//t_AlloThList 	m_AlloThList;


	//线程池指针
	CThreadPool	*m_pThreadPool;
	//保存所以描述符的fd_set

	int m_iThreadPoolSize;

public:
	IRpcServer* m_pIRpcServer;
	//static CRPCConnection *pRPC_connection;

};




#endif
