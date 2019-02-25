#pragma once

#include <vector>
using std::vector;

#include "RpcMessage.pb.h"
using namespace nms_RpcMessage;

#ifndef OUT
#define OUT
#endif // OUT


#define KVMESSAGE_VERSION		0x1

/*
	������Ϣ���ݴ�ȡ�ӿ�
*/
class IRPCBaseMessage
{
public:

	IRPCBaseMessage() {}
	virtual ~IRPCBaseMessage() {}

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
	virtual const char*			Get_StringValue(int key ) = 0;
	virtual int					Get_IntValue(	int key) = 0;
	virtual const char*			Get_BinValue(	int key, OUT int * pDataSize ) = 0;
	virtual const char*			Get_StringPair(	const char*key ) = 0;

	/*
	#==================================================================
	# ���ݼ��������ֵ --- ����ֵ list
	#                 �����ڷ��� �ձ�
	������
		key�� ����ʶ
	����ֵ��
		ֵ�б�
	*/
	/*	������
		iBufCount = 0ʱ������ Value ����,pMaxBufSize�����ַ������������Ļ�������С
		����ֵ��
		plpBuf �д��valueֵ��Get_StringValue_List ���ر��浽plpBuf�е�ָ������
	*/ 
	virtual int					Get_StringValue_List(	int key, OUT size_t* pMaxBufSize, OUT char** plpBuf = NULL, size_t iBufCount = 0) = 0;
	/*	������
		iBufCount = 0ʱ������ Value ����
		����ֵ��
		plpInt �д��valueֵ��Get_StringValue_List ���ر��浽plpInt�е�ָ������
	*/
	virtual int					Get_IntValue_List(		int key, OUT int* plpInt = NULL, size_t iIntCount = 0) = 0;
	/*	������
		iBufCount = 0ʱ������ Value ����,pMaxBufSize�����ַ������������Ļ�������С
		����ֵ��
		plpBuf �д��valueֵ��Get_StringValue_List ���ر��浽plpBuf�е�ָ������
	*/
	virtual int					Get_BinValue_List(		int key, OUT size_t* pMaxBufSize,OUT char** plpBuf = NULL, size_t iBufCount = 0) = 0;
	/*	������
		iBufCount = 0ʱ������ Value ����,pMaxBufSize�����ַ������������Ļ�������С
		����ֵ��
		plpBuf �д��valueֵ��Get_StringValue_List ���ر��浽plpBuf�е�ָ������
	*/
	virtual int					Get_StringPair_List(	const char*key, OUT size_t* pMaxBufSize,OUT char** plpBuf = NULL, size_t iBufCount = 0) = 0;

	/*
	#==================================================================
	// תΪ�ַ��� --- �ڲ������ã��ⲿģ�鲻�������
	*/
	virtual std::string			to_String() = 0;

	/*
	#==================================================================
	// ���л�/�����л�--- �ڲ�ʹ�ã��ⲿģ�鲻�������
	*/
	virtual std::string			SerializeToString() = 0;
	virtual int					ParseFromString(const std::string binobject) = 0;

	/*
	#==================================================================
	// ����/����--- �ڲ�ʹ�ã��ⲿģ�鲻�������
	*/
	virtual std::string			Encode() = 0;
	/*
		����һ������
		������
			binobject�� ��������Ϣ���ݣ����ܰ���1������Ϣ
		����ֵ��
			-1,-2, ���ݳ��Ȳ���
			-100��-101�����������
			����0������ĵ�һ����Ϣ����
	*/
	virtual int					Decode(const std::string binobject) = 0;

	/*
	#==================================================================
	// ���� - Dump
	*/
	virtual void				Dump() = 0;
};

/*
	������Ϣ��ȡ�ӿ�
*/
class IRpcMessage : public IRPCBaseMessage
{
public:
	IRpcMessage() {}
	virtual ~IRpcMessage() {}

	virtual int Get_MsgNo() = 0;
//	virtual int Get_Version() = 0;
	virtual int Get_Cmd() = 0;
	virtual int Get_SubCmd() = 0;
	/*virtual std::string Get_ClientId() = 0;
	virtual int Get_Token() = 0;*/

};

/*
����һ����������Ϣ
*/
IRpcMessage*	NewInstance_IRpcMessage(int cmd, int = RPC_SUBCMD_STUB);

/*
	����һ��������Ϣ
*/
IRpcMessage*	NewInstance_IRpcMessage(const unsigned char* pb, int len, OUT int* pMsgLen = NULL );
IRpcMessage*	NewInstance_IRpcMessage(const std::string binObject, OUT int* pMsgLen = NULL);

///*
//����һ���»�Ӧ��Ϣ
//*/
//IResponseMessage*	NewInstance_IResponseMessage(int error, int info = 0);
//
///*
//����һ����Ӧ��Ϣ
//*/
//IResponseMessage*	NewInstance_IResponseMessage(const unsigned char* pb, int len, OUT int* pMsgLen = NULL);
//IResponseMessage*	NewInstance_IResponseMessage(const std::string binObject, OUT int* pMsgLen = NULL);
//
