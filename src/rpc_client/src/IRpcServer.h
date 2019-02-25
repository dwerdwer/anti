#pragma once
#include <string>

#define IN
#define OUT

class IRpcMessage;

// IRPC �ӿ�

/////////////////////////////////////////////////////////
//
// RPC server �ӿ�

typedef void* DATAPTR;

//
// Socket���ӽӿ�,IO����
//
class IRpcConnection
{
public:
	// ���ͻ�Ӧ����
	virtual int SendResponse(IRpcMessage *msg) = 0;

	// �ر�����
	virtual void Stop() = 0;

	//�Ƿ��Ѿ��ر�
	virtual bool Isclosed() = 0;
};

// �������ӿڣ�����ͻ�������
// ʵ�־�������
class IRpcServer
{
public:
	// ��������֪ͨ  OUT IRpcMessage *msg �û��� Release_Msg  �ͷ�
	virtual int OnRequest(IRpcConnection* hConnection, OUT IRpcMessage* msg ) = 0;

	// ���Ӵ���֪ͨ
	virtual int OnError(IRpcConnection* hConnection, int errCode) = 0;
};

typedef void* IMPLSERVERPTR;

// ���� RpcServer�� ִ�е���
 IMPLSERVERPTR  StartServer(IRpcServer* pIRpcServer, int port, int ThreadPoolSize = 5);

 int			StopServer(IMPLSERVERPTR pMyServer);

 bool 			run_server(IMPLSERVERPTR pMyServer);

//���� IRpcMessage ��Ϣ�� �����û��ֶ��ͷ�
 IRpcMessage*	New_IRpcMsg(int cmd, int subcmd);

//NotifyInfo ���ص�IRpcMessage ������ ��Ҫ�û��ֶ��ͷ�
 void Release_IRpcMsg(IRpcMessage* msg);

