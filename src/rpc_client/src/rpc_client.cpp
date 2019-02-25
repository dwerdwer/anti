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

    if (!msg)
        return -1;
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
		//dbg_log(L"与服务器连接失败！");
		pRpcNotify->OnDisconnected((IRpcConnection*)this, errorno);
#else
		pRpcNotify->OnDisconnected((IRpcConnection*)this, errno);
#endif
		return;
	}
	else
	{
//		printf("long connection build success!\n");
	}
	if (!m_SendMsg)
		return;

	std::string data = m_SendMsg->Encode();
	//dbg_logA("长连接请求发送的数据为：%s", m_SendMsg->to_String().c_str());
//	printf("long connection send data is ：%s size:%d ", m_SendMsg->to_String().c_str(), data.length());
	delete m_SendMsg;
	m_SendMsg = NULL;

	rt = Send((void*)data.c_str(), data.length());
	if (0 == rt.nresult)
	{
		//dbg_log(L"长连接发送数据成功。");
//		printf("long connection send data success!\n");
	//	printf((char*));
		//printf("发送成功！");
	}

	else
	{
		printf("send block or time out!\n");
		//errorno = WSAGetLastError();
		//dbg_log(L"发送阻塞或者超时！");
		pRpcNotify->OnDisconnected((IRpcConnection*)this, rt.nresult);
	}


	while (true)
	{
		rt = Recv((void*)revbuf, sizeof(revbuf));
		if (rt.nresult == 0)
		{
			m_strbuf.append(revbuf, rt.nbytes);
            while (m_strbuf.length() > 0)
            {
                int msgLen;
                IRpcMessage* msg2 = NewInstance_IRpcMessage(m_strbuf, &msgLen);

                if (msgLen < 0 && msgLen > -100)
                {
				//    printf("数据长度不足! %d", msgLen);
             //       break;
                }
                else if (msgLen <= -100)
                {
				//    printf("数据不合法！%d", msgLen);
                    m_strbuf.clear();
				    break;
                }

                if (msg2)
                {
                    if (-1 == pRpcNotify->NotifyInfo((IRpcConnection*)this, msg2))
                    {
                        break;
                    }
                    m_strbuf.erase(0, msgLen);
                }
                else
                {
                    break;
                }
            }
		}
		else if (-1 == rt.nresult || 10054 == rt.nresult || 10053 == rt.nresult)
		{
			//dbg_log(L"服务器断开!：%d", rt.nresult);
			printf("服务器断开!：%d", rt.nresult);
			if (-1 == pRpcNotify->OnDisconnected((IRpcConnection*)this, rt.nresult))
			{
				break;
			}
		}
		else
		{
			//dbg_log(L"网络阻塞或者超时！");
			printf("网络阻塞或者超时！");
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
//		dbg_log(L"初始化失败!");
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
//			dbg_log(L"服务器断开！");
			printf("服务器断开！");
			return NULL;
		}
		else if (0 == rt.nresult)
		{
			strbuf.append(revbuf, rt.nbytes);
			int msgLen;
			IRpcMessage* msg2 = NewInstance_IRpcMessage(strbuf, &msgLen);

			if (msgLen < 0 && msgLen > -100)
			{
//				dbg_log(L"数据长度不足! %d", msgLen);
			//	printf("数据长度不足! %d", msgLen);
			}
			else if (msgLen < -100)
			{
//				dbg_log(L"数据不合法！%d", msgLen);
			//	printf("数据不合法！%d", msgLen);
				break;
			}
			else
			{
				return msg2;
			}

//			dbg_logA("客户端收到短连接请求响应，接收到数据: %s", msg2->to_String().c_str());
			//printf("客户端收到短连接请求响应，接收到数据: %s", msg2->to_String().c_str());
			//sprintf((char*)revbuf, "%d");
		}
		else
		{
//			dbg_log(L"网络阻塞或者超时！");
			printf("网络阻塞或者超时！");
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
//		dbg_log(L"初始化失败！");
		printf("初始化失败！");
	}

	TCPClient* pRPCClient = new TCPClient(port, hostInfo);

	if (0 != pRPCClient->connect())
	{
#ifdef WIN32
		errorno = WSAGetLastError();
//		dbg_log(L"与服务器连接失败！");
		printf("与服务器连接失败: errno:%d\n",errorno);
#else
		printf("与服务器连接失败: errno:%d\n", errno);
#endif
		return NULL;
	}
	else
	{
		//dbg_log(L"短连接请求与服务器连接成功！");
	//	printf("短连接请求与服务器连接成功！");
	}
	std::string data = msg->Encode();
	delete msg;
	rt = pRPCClient->Send((void*)data.c_str(), data.length());
	if (0 == rt.nresult)
	{
		//dbg_log(L"短连接请求发送的数据为：%s", data);
	//	printf("短连接请求发送的数据为：%s", data.c_str());
	}
	else if (-1 == rt.nresult)
	{
		//dbg_log(L"短连接请求发送数据失败");
		printf("短连接请求发送数据失败");
	}
	else
	{
		//dbg_log(L"短链接请求发送数据阻塞或超时");
		printf("短链接请求发送数据阻塞或超时");
	}

	IRpcMessage *msg2 =  RecvData(pRPCClient);

    /*
    if(msg2)
    	msg2->Dump();
    */
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

    if(!msg2)
        return -1;
	assert(msg2);
	*value = msg2->Get_IntValue(keyId);

	delete msg2;
	return pdwError;
}


int QueryDataString(IN const char* hostInfo, IN int port, IN IRpcMessage* msg, IN int keyId, OUT void* outputBuf, IN size_t bufSize)
{
	int pdwError;
	IRpcMessage* msg2 = QueryData(hostInfo, port, msg, &pdwError);
    if(!msg2)
        return -1;
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
