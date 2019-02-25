#include "./rpc_client.h"
//#include "RpcMessage.h"
#include <errno.h>
//#include <atlconv.h>
//#include <studio.h>



CRPCClient::CRPCClient(int port, const char* hostInfo, IRpcNotify* pIRpcNotify, IRpcMessage* msg) :TCPClient(port, hostInfo)
{
	pRpcNotify = pIRpcNotify;
	m_SendMsg = msg;
	memset(revbuf, 0, sizeof(revbuf));
}

CRPCClient::~CRPCClient()
{

}


int CRPCClient::SendResponse(IRpcMessage *msg)
{
	//char* resp = "hahaa ... "

	assert(msg);
	std::string data = msg->Encode();
	delete msg;
	Send((void*)data.c_str(), data.length());

	return 0;
}

void CRPCClient::Stop()
{
	::SocketClose(m_hSocket);
}


void CRPCClient::Run(void)
{
	int errorno = -1;
	transresult_t rt = { 0,0 };

	if (0 != connect())
	{
		printf("connect server failed\n");
#ifdef WIN32
		errorno = WSAGetLastError();
		//dbg_log(L"�����������ʧ�ܣ�");
		pRpcNotify->OnDisconnected((IRpcConnection*)this, errorno);
#else
		pRpcNotify->OnDisconnected((IRpcConnection*)this, errno);
#endif
		return;
	}
	else
	{
		printf("long connection build success!\n");
	}
	if (!m_SendMsg)
		return;

	std::string data = m_SendMsg->Encode();
	//dbg_logA("�����������͵�����Ϊ��%s", m_SendMsg->to_String().c_str());
	printf("long connection send data is ��%s", m_SendMsg->to_String().c_str());
	delete m_SendMsg;
	m_SendMsg = NULL;

	rt = Send((void*)data.c_str(), data.length());
	if (0 == rt.nresult)
	{
		//dbg_log(L"�����ӷ������ݳɹ���");
		printf("long connection send data success!\n");
	//	printf((char*));
		//printf("���ͳɹ���");
	}

	else
	{
		printf("send block or time out!\n");
		//errorno = WSAGetLastError();
		//dbg_log(L"�����������߳�ʱ��");
		pRpcNotify->OnDisconnected((IRpcConnection*)this, rt.nresult);
	}

	
	while (true)
	{
		rt = Recv((void*)revbuf, sizeof(revbuf));
		std::string strbuf;
		if (rt.nresult == 0)
		{
			strbuf.append(revbuf, rt.nbytes);
			int msgLen;
			IRpcMessage* msg2 = NewInstance_IRpcMessage(strbuf, &msgLen);

			if (msgLen < 0 && msgLen > -100)
			{
				//dbg_log(L"���ݳ��Ȳ���! %d", msgLen);
				printf("Insufficient data length! %d\n", msgLen);
			}
			else if (msgLen <= -100)
			{
				//dbg_log(L"���ݲ��Ϸ���%d", msgLen);
				printf("Illegal data! %d\n", msgLen);
				break;
			}

			if (msg2)
			{
				if (-1 == pRpcNotify->NotifyInfo((IRpcConnection*)this, msg2))
				{
					break;
				}
				strbuf.clear();
			}
			else
			{

			}
		}
		else if (-1 == rt.nresult || 10054 == rt.nresult || 10053 == rt.nresult)
		{
			//dbg_log(L"�������Ͽ�!��%d", rt.nresult);
			printf("Server disconnected %d\n", rt.nresult);
			if (-1 == pRpcNotify->OnDisconnected((IRpcConnection*)this, rt.nresult))
			{
				break;
			}
		}
		else
		{
			//dbg_log(L"�����������߳�ʱ��");
            printf("Network congestion or timeout\n");
			if(-1 == pRpcNotify->OnDisconnected((IRpcConnection*)this,  rt.nresult))
			{
				break;
			}
		}

	}
	
	delete this;		
	return;

}



int QueryNotify(IN const char* hostInfo, int port, IN IRpcMessage* msg, IRpcNotify* pIRpcNotify)
{
	bool bReuseaddr = true;
	int errorno = -1;
	if (-1 == InitializeSocketEnvironment())
	{
//		dbg_log(L"��ʼ��ʧ��!");
		return -1;
	}
	

	CRPCClient * pRPCClient = new CRPCClient(port, hostInfo, pIRpcNotify, msg);

	//errorno = WSAGetLastError();


	pRPCClient->Start();

	return 0;
}

IRpcMessage*  RecvData(TCPClient* pRPCClient)
{
	transresult_t rt = { 0, 0 };
	char revbuf[1024] = {0};
	std::string strbuf;
	
	while (true)
	{
		rt = pRPCClient->Recv(revbuf, 1024);

		if (-1 == rt.nresult || 10054 == rt.nresult || 10053 == rt.nresult)
		{
//			dbg_log(L"�������Ͽ���");
			printf("Server disconnected %d\n", rt.nresult);
			return NULL;
		}
		else if (0 == rt.nresult)
		{
			strbuf.append(revbuf, rt.nbytes);
			int msgLen;
			IRpcMessage* msg2 = NewInstance_IRpcMessage(strbuf, &msgLen);

			if (msgLen < 0 && msgLen > -100)
			{
//				dbg_log(L"���ݳ��Ȳ���! %d", msgLen);
				printf("Insufficient data length! %d\n", msgLen);
			}
			else if (msgLen < -100)
			{
//				dbg_log(L"���ݲ��Ϸ���%d", msgLen);
				printf("Illegal data! %d\n", msgLen);
				break;
			}
			else
			{
				return msg2;
			}
		
//			dbg_logA("�ͻ����յ�������������Ӧ�����յ�����: %s", msg2->to_String().c_str());
//			printf("�ͻ����յ�������������Ӧ�����յ�����: %s", msg2->to_String().c_str());
			//sprintf((char*)revbuf, "%d");
		}
		else
		{
//			dbg_log(L"�����������߳�ʱ��");
            printf("Network congestion or timeout\n");
		}

	}
	return NULL;
}

IRpcMessage*  QueryData(IN const char* hostInfo, IN int port, IN IRpcMessage* msg, OUT int * pdwError)
{
	bool bReuseaddr = true;
	int errorno = -1;
	transresult_t rt = { 0, 0 };

	if (-1 == InitializeSocketEnvironment())
	{
//		dbg_log(L"��ʼ��ʧ�ܣ�");
		printf("Initial error\n");
	}
	
	TCPClient* pRPCClient = new TCPClient(port, hostInfo);

	if (0 != pRPCClient->connect())
	{
#ifdef WIN32
		errorno = WSAGetLastError();
//		dbg_log(L"�����������ʧ�ܣ�");
		printf("�����������ʧ��: errno:%d\n",errorno);
#else 
		printf("Connection failure: errno:%d\n", errno);
#endif
		return NULL;
	}
	else
	{
		//dbg_log(L"��������������������ӳɹ���");
        printf("Short connection request and server connection is successful\n");
	}
	std::string data = msg->Encode();
	delete msg;
	rt = pRPCClient->Send((void*)data.c_str(), data.length());
	if (0 == rt.nresult)
	{
		//dbg_log(L"�����������͵�����Ϊ��%s", data);
		printf("Short connection request data: %s\n", data.c_str());
	}
	else if (-1 == rt.nresult)
	{
		//dbg_log(L"����������������ʧ��");
		printf("Short connection request error\n");
	}
	else
	{
		//dbg_log(L"����������������������ʱ");
		printf("Short connection request blocking or timeout\n");
	}

	IRpcMessage *msg2 =  RecvData(pRPCClient);

	// msg2->Dump();
	if (rt.nresult == -1 && rt.nbytes == 0) 
		*pdwError = 0;
	else
		*pdwError = rt.nresult;

	delete pRPCClient;
	
	return msg2;
}

int QueryDataInt(IN const char* hostInfo, IN int port, IN IRpcMessage* msg, IN int keyId, OUT int * value)
{
	int pdwError;
	IRpcMessage * msg2 =  QueryData(hostInfo, port, msg, &pdwError);

	assert(msg2);
	*value = msg2->Get_IntValue(keyId);

	delete msg2;
	return pdwError;
}


int QueryDataString(IN const char* hostInfo, IN int port, IN IRpcMessage* msg, IN int keyId, OUT void* outputBuf, IN size_t bufSize)
{
	int pdwError;
	IRpcMessage* msg2 = QueryData(hostInfo, port, msg, &pdwError);

	assert(msg2);
	std::string strRet =  msg2->Get_StringValue(keyId);

	delete msg2;

	if (strRet.length() > bufSize)
		return -1;
	else
#ifdef WIN32
		memcpy_s(outputBuf, bufSize, strRet.c_str(), strRet.length());
#else
		memcpy(outputBuf, strRet.c_str(), strRet.length());
#endif
	return pdwError;
}


int QueryInt(IN const char* hostInfo, IN int port, IN int cmdId, int subCmd, IN int keyId, OUT int *value)
{
	IRpcMessage* msg = NewInstance_IRpcMessage(cmdId, subCmd);

	int iRet =  QueryDataInt(hostInfo, port, msg, keyId, value);
	//delete msg;
	return iRet;
}

int QueryString(IN const char* hostInfo, IN int port, IN int cmdId, int subCmd, IN int keyId, OUT void* outputBuf, IN int bufSize)
{
	IRpcMessage* msg = NewInstance_IRpcMessage(cmdId, subCmd);

	int iRet =  QueryDataString(hostInfo, port, msg, keyId, outputBuf, bufSize);
	//delete msg;
	return iRet;
}

IRpcMessage* New_IRpcMsg(int cmd, int subCmd)
{
	return NewInstance_IRpcMessage(cmd, subCmd);
}

void Release_IRpcMsg(IRpcMessage* msg)
{
	delete msg;
}
