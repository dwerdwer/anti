#include "RpcMessage.h"

///////////////////////////////////////////////////////////////////////
//
// 全局变量
//

unsigned int	g_msgNo;		// 消息号


///////////////////////////////////////////////////////////////////////
//
// 请求消息
//

/*
创建一个新请求消息
*/
IRpcMessage* NewInstance_IRpcMessage(int cmd, int subCmd)
{
	RpcMsg* msg = new RpcMsg(cmd, subCmd);

	msg->set_msgno(++g_msgNo);
	return msg;
}

/*
解码一个请求消息
*/
IRpcMessage*	NewInstance_IRpcMessage(const unsigned char* pb, int len, OUT int* pMsgLen)
{
	std::string obj((const char*)pb, len);

	return NewInstance_IRpcMessage(obj, pMsgLen);
}

IRpcMessage*	NewInstance_IRpcMessage(const std::string binObject, OUT int* pMsgLen)
{
	IRpcMessage* msg = new RpcMsg();
	int len = msg->Decode(binObject);
	if (pMsgLen != NULL)
		*pMsgLen = len;
	if (len > 0) {
		return msg;
	}
	delete msg;
	return NULL;
}
