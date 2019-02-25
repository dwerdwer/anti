#ifndef _TCPCLIENT_
#define _TCPCLIENT_

#include "./sock_wrap.h"

class TCPClient : public CSockWrap
{
public:
    TCPClient(unsigned short int port, const char *ip);
    TCPClient();
    //TCPClient(int clientfd);
    ~TCPClient();    

	int connect();	  
};










#endif   /*!_TCPCLIENT_*/
