#include "KVMessage.h"

///////////////////////////////////////////////////////////////////////
//
// ȫ�ֱ���
//
std::string		g_clientId;		// �ͻ���Id
std::string		g_token;		// �Ự Token
unsigned int	g_msgNo;		// ��Ϣ��


///////////////////////////////////////////////////////////////////////
//
// ����ȫ�ֱ���
//
// ���ÿͻ���Id
void	Set_ClientId(const char* clientId)
{
	g_clientId = clientId;
}

// ���ûỰ Token
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
// ������Ϣ
//

/*
����һ����������Ϣ
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
����һ��������Ϣ
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
// ��Ӧ��Ϣ
//

/*
����һ���»�Ӧ��Ϣ
*/
IResponseMessage*	NewInstance_IResponseMessage(int error, int info )
{
	ResponseMsg* msg = new ResponseMsg(error, info);

	return msg;
}

/*
����һ����Ӧ��Ϣ
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

