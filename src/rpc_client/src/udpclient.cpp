#include "udpclient.h"
#include "comm.h"

/** client UDP Socket **/
UDPClient::UDPClient(unsigned short int port, const char *ip):CSockWrap(SOCK_UDP)
{
    SetAddress(ip,port);
	
}
UDPClient::UDPClient():CSockWrap(SOCK_UDP) 
{
}

//TCPClient::TCPClient(int clientfd):CSockWrap(SOCK_TCP)
//{
//    m_sockfd = clientfd;
//}

UDPClient::~UDPClient()
{
}



