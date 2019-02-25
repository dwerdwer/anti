#include "KVMessage.h"

///////////////////////////////////////////////////////////////////////
//
// 全局变量
//
std::string		g_clientId;		// 客户端Id
std::string		g_token;		// 会话 Token
unsigned int	g_msgNo;		// 消息号


///////////////////////////////////////////////////////////////////////
//
// 设置全局变量
//
// 设置客户端Id
void	Set_ClientId(const char* clientId)
{
	g_clientId = clientId;
}

// 设置会话 Token
void	Set_Token(const char* token)
{
	g_token = token;
}

std::string Get_Token()
{
	return g_token;
}


///////////////////////////////////////////////////////////////////////
//
// 请求消息
//

/*
创建一个新请求消息
*/
IRequestMessage* NewInstance_IRequestMessage(int cmd, int subCmd)
{
	RequestMsg* msg = new RequestMsg(cmd, subCmd);

	msg->set_version(KVMESSAGE_VERSION);
	msg->set_msgno( ++g_msgNo );
	if ( cmd == CMD_ECHO ||
		 cmd == CMD_INSTALL ||
		(cmd == CMD_LOGIN && (subCmd == SUBCMD_LOGIN || subCmd == SUBCMD_REGISTER)))
		msg->set_id(g_clientId);
	else
		msg->set_token(g_token);

	return msg;


}

/*
解码一个请求消息
*/
IRequestMessage*	NewInstance_IRequestMessage(const unsigned char* pb, int len, OUT int* pMsgLen)
{
	std::string obj((const char*)pb, len);

	return NewInstance_IRequestMessage(obj, pMsgLen);
}

IRequestMessage*	NewInstance_IRequestMessage(const std::string binObject, OUT int* pMsgLen)
{
	IRequestMessage* msg = new RequestMsg();
	int len = msg->Decode(binObject);
	if (pMsgLen != NULL)
		*pMsgLen = len;
	if (len > 0) {
		return msg;
	}
	delete msg;
	return NULL;
}

///////////////////////////////////////////////////////////////////////
//
// 回应消息
//

/*
创建一个新回应消息
*/
IResponseMessage*	NewInstance_IResponseMessage(int error, int info )
{
	ResponseMsg* msg = new ResponseMsg(error, info);

	return msg;
}

/*
解码一个回应消息
*/
IResponseMessage*	NewInstance_IResponseMessage(const unsigned char* pb, int len, OUT int* pMsgLen)
{
	std::string obj((const char*)pb, len);

	return NewInstance_IResponseMessage(obj, pMsgLen);
}

IResponseMessage*	NewInstance_IResponseMessage(const std::string binObject, OUT int* pMsgLen)
{
	IResponseMessage* msg = new ResponseMsg();
	int len = msg->Decode(binObject);
	if (pMsgLen != NULL)
		*pMsgLen = len;
	if (len > 0) {
		return msg;
	}
	delete msg;
	return NULL;
}

