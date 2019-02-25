#ifndef _RPC_CLIENT_H_
#define _RPC_CLIENT_H_

#include "mythread.h"
#include "tcpclient.h"
#include "RpcMessage.h"
#include "IRpcClient.h"


//#include <stdio>
//#include <string.h>
//#include <conio.h>
#include <iostream>
//#include "../comm.h"

class CRPCClient : public TCPClient,public CMyThread, IRpcConnection
{
public:
	CRPCClient(int port, const char* hostInfo, IRpcNotify* pIRpcNotify, IRpcMessage* msg);
	virtual ~CRPCClient();

	virtual void Run(void);

	virtual int SendResponse(IRpcMessage *msg);

	virtual void Stop();


private:
	IRpcNotify *pRpcNotify;
	//IRpcConnection *pIRpcConnection;
	IRpcMessage* m_SendMsg;

    std::string m_strbuf;
	char revbuf[4*1024];
};



#endif
