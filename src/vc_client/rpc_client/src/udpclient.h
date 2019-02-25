#ifndef _UDPCLIENT_
#define _UDPCLIENT_

#include "./sock_wrap.h"



class UDPClient : public CSockWrap
{
public:
    UDPClient(unsigned short int port, const char *ip);
    UDPClient();
    //TCPClient(int clientfd);
    ~UDPClient();    

	//int connect(HSocket hs);	  
};










#endif   /*!_UDPCLIENT_*/

