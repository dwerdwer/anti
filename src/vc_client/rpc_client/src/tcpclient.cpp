#include "tcpclient.h"
#include "comm.h"

/** client TCP Socket **/
TCPClient::TCPClient(unsigned short int port, const char *ip):CSockWrap(SOCK_TCP)
{
    SetAddress(ip,port);
	
}
TCPClient::TCPClient():CSockWrap(SOCK_TCP) 
{
}

//TCPClient::TCPClient(int clientfd):CSockWrap(SOCK_TCP)
//{
//    m_sockfd = clientfd;
//}

TCPClient::~TCPClient()
{
}

int TCPClient::connect()
{  
	//return bind(hs, (struct sockaddr *)paddr, sizeof(sockaddr_in));  
	return ::connect(m_hSocket, (struct sockaddr *) &m_stAddr, sizeof(sockaddr_in));//????????IP
}






