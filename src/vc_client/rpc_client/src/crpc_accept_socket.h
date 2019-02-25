#ifndef _CRPC_ACCEPT_SOCKET_H_
#define _CRPC_ACCEPT_SOCKET_H_


#include "./sock_wrap.h"
#include "IRpcServer.h"
class AccSock : public CSockWrap, public IRpcConnection
{

public:
	AccSock(HSocket accept_sock, IRpcServer* pIRpcServer);
	~AccSock();
	bool OnReceive();
	void Close();
	virtual bool Isclosed(){return is_closed;}
	void SetIsClosed(bool isclosed){is_closed = isclosed;}

	bool		Is_valid() const {return m_hSocket != INVALID_SOCKET;}
	
	virtual int SendResponse(IRpcMessage *msg);
	virtual void Stop();

	//CMyThreadMutex m_mutex;		//
	//CMyThreadCondition 	m_cond;

private:

	IRpcServer* m_pIRpcServer;

	char revbuf[4*1024];
	int  nRecvOffs;		// 已处理数据
	int  nRecvTotal;	// recvbuf 中收到的数据

	bool is_closed;
};






#endif
