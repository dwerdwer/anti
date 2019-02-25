#pragma once
#include <string>

#define IN
#define OUT

class IRpcMessage;

// IRPC 接口

/////////////////////////////////////////////////////////
//
// RPC server 接口

typedef void* DATAPTR;

//
// Socket连接接口,IO处理
//
class IRpcConnection
{
public:
	// 发送回应数据
	virtual int SendResponse(IRpcMessage *msg) = 0;

	// 关闭连接
	virtual void Stop() = 0;

	//是否已经关闭
	virtual bool Isclosed() = 0;
};

// 服务器接口，处理客户端连接
// 实现具体数据
class IRpcServer
{
public:
	// 接收数据通知  OUT IRpcMessage *msg 用户掉 Release_Msg  释放
	virtual int OnRequest(IRpcConnection* hConnection, OUT IRpcMessage* msg ) = 0;

	// 连接错误通知
	virtual int OnError(IRpcConnection* hConnection, int errCode) = 0;
};

typedef void* IMPLSERVERPTR;

// 启动 RpcServer， 执行调度
 IMPLSERVERPTR  StartServer(IRpcServer* pIRpcServer, int port, int ThreadPoolSize = 5);

 int			StopServer(IMPLSERVERPTR pMyServer);

 bool 			run_server(IMPLSERVERPTR pMyServer);

//创建 IRpcMessage 消息， 无需用户手动释放
 IRpcMessage*	New_IRpcMsg(int cmd, int subcmd);

//NotifyInfo 返回的IRpcMessage 参数， 需要用户手动释放
 void Release_IRpcMsg(IRpcMessage* msg);

