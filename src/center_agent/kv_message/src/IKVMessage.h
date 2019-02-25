#pragma once

#include <vector>
using std::vector;

#include "KVMessage.pb.h"
using namespace KVMessage;

#ifndef OUT
#define OUT
#endif // OUT


#define KVMESSAGE_VERSION		0x1

/*
	������Ϣ���ݴ�ȡ�ӿ�
*/
class IBaseMessage
{
public:

	IBaseMessage() {}
	virtual ~IBaseMessage() {}

	/*
	#==================================================================
	# ���Ӽ�ֵ
	������
		key�� ����ʶ
		value��ֵ
	*/
	virtual void Add_StringValue(	int key, const char* value ) = 0;
	virtual void Add_IntValue(		int key, int value) = 0;
	virtual void Add_BinValue(		int key, std::string value) = 0;
	virtual void Add_StringPair(	const char* key, const char* value) = 0;

	/*
	#==================================================================
	# ���ݼ����һ��ֵ --- ���ͬһ���ж��ֵ�������ص�һ��ֵ
	#                 �����ڷ��� 0 �� �մ�;
	������
		key�� ����ʶ
	����ֵ��
		ֵ
	*/
	virtual std::string	Get_StringValue(int key ) = 0;
	virtual int			Get_IntValue(	int key) = 0;
	virtual std::string	Get_BinValue(	int key ) = 0;
	virtual std::string	Get_StringPair(	const char*key ) = 0;

	/*
	#==================================================================
	# ���ݼ��������ֵ --- ����ֵ list
	#                 �����ڷ��� �ձ�
	������
		key�� ����ʶ
	����ֵ��
		ֵ�б�
	*/
	virtual vector<std::string>	Get_StringValue_List(	int key) = 0;
	virtual vector<int>			Get_IntValue_List(		int key) = 0;
	virtual vector<std::string>	Get_BinValue_List(		int key) = 0;
	virtual vector<std::string>	Get_StringPair_List(	const char*key ) = 0;

	/*
	#==================================================================
	// תΪ�ַ��� --- ������
	*/
	virtual std::string to_String() = 0;

	/*
	#==================================================================
	// ���л�/�����л�
	*/
	virtual std::string SerializeToString() = 0;
	virtual int ParseFromString(std::string binobject) = 0;

	/*
	#==================================================================
	// ����/����
	*/
	virtual std::string Encode() = 0;

	/*
		����һ������
		������
			binobject�� ��������Ϣ���ݣ����ܰ���1������Ϣ
		����ֵ��
			-1,-2, ���ݳ��Ȳ���
			-100��-101�����������
			����0������ĵ�һ����Ϣ����
	*/
	virtual int Decode(std::string binobject) = 0;

	/*
	#==================================================================
	// ���� - Dump
	*/
	virtual void Dump() = 0;
};

/*
	������Ϣ��ȡ�ӿ�
*/
class IRequestMessage : public IBaseMessage
{
public:
	IRequestMessage() {}
	virtual ~IRequestMessage() {}

	virtual int Get_MsgNo() = 0;
	virtual int Get_Version() = 0;
	virtual int Get_Cmd() = 0;
	virtual int Get_SubCmd() = 0;
	virtual std::string Get_ClientId() = 0;
	virtual std::string Get_Token() = 0;

};

/*
	��Ӧ��Ϣ��ȡ�ӿ�
*/
class IResponseMessage : public IBaseMessage
{
public:
	IResponseMessage() {}
	virtual ~IResponseMessage() {}

	virtual int Get_Info() = 0;
	virtual int Get_Error() = 0;
};



// ���ÿͻ���Id
void				Set_ClientId(const char* clientId);

// ���ûỰ Token
void				Set_Token(const char* token);

// ���ػỰ Token
std::string			Get_Token();
/*
����һ����������Ϣ
*/
IRequestMessage*	NewInstance_IRequestMessage(int cmd, int subCmd = SUBCMD_STUB);

/*
	����һ��������Ϣ
*/
IRequestMessage*	NewInstance_IRequestMessage(const unsigned char* pb, int len, OUT int* pMsgLen = NULL );
IRequestMessage*	NewInstance_IRequestMessage(const std::string binObject, OUT int* pMsgLen = NULL);


/*
����һ���»�Ӧ��Ϣ
*/
IResponseMessage*	NewInstance_IResponseMessage(int error, int info = 0);

/*
����һ����Ӧ��Ϣ
*/
IResponseMessage*	NewInstance_IResponseMessage(const unsigned char* pb, int len, OUT int* pMsgLen = NULL);
IResponseMessage*	NewInstance_IResponseMessage(const std::string binObject, OUT int* pMsgLen = NULL);

