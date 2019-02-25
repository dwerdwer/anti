#ifndef _TCPSERVER_
#define _TCPSERVER_

#include "sock_wrap.h"
#include "IRpcServer.h"

//#define NO_OnConnect	virtual bool OnConnect() {return true;}
//#define NO_OnAccept	virtual bool OnAccept() {return true;}
//#define NO_OnReceive	virtual bool OnReceive() {return true;}
//#define NO_OnClose	virtual bool OnClose() {return true;}
//#define NO_OnSend	virtual bool OnSend() {return true;}



class TCPServer : public CSockWrap
{
public:
    TCPServer(unsigned short int port, const char *ip);
    TCPServer(HSocket m_sock);
    //TCPClient(int clientfd);
    ~TCPServer();    

	

	//int connect(HSocket hs);	
	int bind();
	

	int listen(int maxconn);
	HSocket accept();

	
	//��ʶ���׽����Ƿ����ڴ���
	bool		IsBusy() const {return m_bBusy;}
	void		SetBusy(const bool b) {m_bBusy = b;}
	
	bool		Is_valid() const {return m_hSocket != INVALID_SOCKET;}
	//���������������
	void		Set_non_blocking(const bool b/*true������*/);



public:
	//virtual bool  OnReceive(){}
	
	bool	OnConnect() ;
	
	//��ͣ�߳�
	//void		StopThread();

private:
	
	bool		m_bBusy;

};










#endif   /*!_TCPSERVER_*/

