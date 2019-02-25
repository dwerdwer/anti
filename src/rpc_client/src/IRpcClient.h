#pragma once

#define RPCCLIENT_API __attribute ((visibility("default")))
#define IN
#define OUT

class IRpcMessage;

//
// Socket连接接口,IO处理
// 没必要导出
class IRpcConnection
{
public:
	// 发送回应数据
	virtual int SendResponse(IRpcMessage *msg) = 0;

	// 关闭连接
	virtual void Stop() = 0;
};

/////////////////////////////////////////////////////////
//
// RPC client 接口

class IRpcNotify
{
public:
	// 通知信息  OUT IRpcMessage *msg 用户掉 Release_Msg  释放
	virtual int NotifyInfo(IRpcConnection* hConnection,  IRpcMessage* msg) = 0;

	// 连接断开通知
	virtual int OnDisconnected(IRpcConnection* hConnection, int error = 0) = 0;
};

/*
// 长连接方式获取信息，非阻塞方式
int QueryNotify( int iQueryId, IRpcNotify* pIRpcNotify );

// 发布命令或获取信息
int QueryData(int iQueryId, int wParam, IN void* params, OUT void* outputBuf, OUT int* size);
*/

// 长连接方式获取信息，非阻塞方式
// 参数：
//		 hostInfo， 主机地址，格式： ip:port 字符串
// 返回 -1，错误
// 返回0，设置成功
RPCCLIENT_API int  QueryNotify( IN const char*  hostInfo, int port, IN IRpcMessage* msg, IRpcNotify* pIRpcNotify);



// 发布命令或获取信息, 阻塞方式
// 参数：
//		 hostInfo， 主机地址，格式： ip:port
// 返回 NULL，错误
// 返回其他值，数据大小
RPCCLIENT_API IRpcMessage* QueryData(IN const char* hostInfo, IN int port, IN IRpcMessage* msg, OUT int * pdwError);

// 发布命令获取信息， 阻塞方式
//参数：
//		hostInfo , 主机地址， 格式： ip:port  keyId: msg 中key值
//返回 -1 , 错误
//返回0， 正确

RPCCLIENT_API int QueryDataInt(IN const char* hostInfo, IN int port, IN IRpcMessage* msg, IN int keyId, OUT int * value );

// 发布命令获取信息， 阻塞方式
//参数：
//		hostInfo , 主机地址， 格式： ip:port  keyId: msg 中key值
//		outputBuf, 获取的字符串首地址， 用户自己开辟空间。
//返回 -1 , 错误
//返回0， 正确
RPCCLIENT_API int QueryDataString(IN const char* hostInfo, IN int port, IN IRpcMessage* msg, IN int keyId, OUT void* outputBuf, IN size_t bufSize);

// 发布命令获取信息， 阻塞方式
//参数：
//		hostInfo , 主机地址， 格式： ip:port  keyId: msg 中key值
//返回 -1 , 错误
//返回0， 正确
RPCCLIENT_API int QueryInt(IN const char* hostInfo, IN int port, IN int cmdId, int subCmd, IN int keyId, OUT int * value);

// 发布命令获取信息， 阻塞方式
//参数：
//		hostInfo , 主机地址， 格式： ip:port  keyId: msg 中key值
//		outputBuf, 获取的字符串首地址， 用户自己开辟空间。
//返回 -1 , 错误
//返回0， 正确
RPCCLIENT_API int QueryString(IN const char* hostInfo, IN int port, IN int cmdId, int subCmd, IN int keyId, OUT void* outputBuf, IN int bufSize);


//创建 IRpcMessage 消息， 无需用户手动释放
RPCCLIENT_API IRpcMessage* New_IRpcMsg(int cmd, int subCmd);

//NotifyInfo 返回的IRpcMessage 参数， 需要用户手动释放
RPCCLIENT_API void Release_IRpcMsg(IRpcMessage* msg);
