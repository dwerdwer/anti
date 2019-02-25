#include "./crpc_accept_socket.h"
#include "comm.h"
#include "RpcMessage.h"


AccSock::AccSock(HSocket accept_sock, IRpcServer* pIRpcServer):CSockWrap(SOCK_TCP, accept_sock)
{

	is_closed = false;
	//revbuf[4*1024] = {0};
	memset((void*)revbuf, 0,sizeof(revbuf));
	m_pIRpcServer = pIRpcServer;

	nRecvTotal = 0;
	nRecvOffs = 0;
}


AccSock::~AccSock()
{
	SocketClose(m_hSocket);

}


void AccSock::Close()
{
	if (Is_valid()) 
	{
		
		//FD_CLR(m_hSocket, &m_rfd);
		//m_listSock.remove(m_hSocket);

		
		//m_listAccept.remove(m_hSocket);   //暂时不remove
		//m_mapAccept.erase(m_hSocket);
		//map也要移出

		SocketClose(m_hSocket);


#if 0

#if defined(_LINUX_PLATFORM_) 
		//return close(m_hSocket) != SOCKET_ERROR;
		close(m_hSocket);
#endif

#if defined(_WIN32_PLATFORM_) 
		CloseHandle((HANDLE)m_hSocket);

#endif

#endif

		is_closed = true;
		m_hSocket = INVALID_SOCKET;     //wangzheng

	}

}

int AccSock::SendResponse(IRpcMessage *msg)
{
	//char* resp = "hahaa ... ";
	assert(msg);
	std::string data = msg->Encode();
	delete msg;
	Send((void *)data.c_str(), data.length());

	return 0;
}

void AccSock::Stop()
{
	Close();
}


bool AccSock::OnReceive()
{
	transresult_t rt;  
	std::string strbuf;
	//rt = Recv((void*)(revbuf+ nRecvTotal), sizeof(revbuf)- nRecvTotal);
//	printf("On Receive\n");
	while (true)
	{
//		printf("start recv\n");
		rt = Recv((void*)(revbuf), sizeof(revbuf));
//		printf("Recv return\n");
		if (-1 == rt.nresult || 10054 == rt.nresult || 10053 == rt.nresult)
		{
			m_pIRpcServer->OnError((IRpcConnection*)this, rt.nresult);
			Close();
			//dbg_log(L"有客户端断开");
			puts("client closed\n");
			return false;
		}
		else if (0 == rt.nresult)
		{
			strbuf.append(revbuf, rt.nbytes);
			int msgLen;
			IRpcMessage* msg2 = NewInstance_IRpcMessage(strbuf, &msgLen);

			if (msgLen < 0 && msgLen > -100)
			{
				//dbg_log(L"数据长度不足! %d", msgLen);
				printf("data length is not enough! %d", msgLen);
				
			}
			else if (msgLen < -100)
			{
				//dbg_log(L"数据不合法！%d", msgLen);
				printf("data invalid%d", msgLen);
				break;
			}
			else
			{
				//dbg_logA("客户端收到短连接请求响应，接收到数据 %s", msg2->to_String().c_str());
			//	std::string s = msg2->Get_StringValue(3);
				m_pIRpcServer->OnRequest((IRpcConnection*)this, msg2);
				strbuf.clear();
				
			}

			
			//sprintf((char*)revbuf, "%d");
		}
		else
		{
			//dbg_log(L"网络阻塞或者超时！");
			puts("net block or time out\n");
		}
	}
	return true;
}









