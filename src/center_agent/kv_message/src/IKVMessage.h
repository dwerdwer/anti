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
	基本消息数据存取接口
*/
class IBaseMessage
{
public:

	IBaseMessage() {}
	virtual ~IBaseMessage() {}

	/*
	#==================================================================
	# 增加键值
	参数：
		key： 键标识
		value：值
	*/
	virtual void Add_StringValue(	int key, const char* value ) = 0;
	virtual void Add_IntValue(		int key, int value) = 0;
	virtual void Add_BinValue(		int key, std::string value) = 0;
	virtual void Add_StringPair(	const char* key, const char* value) = 0;

	/*
	#==================================================================
	# 根据键获得一个值 --- 如果同一键有多个值，仅返回第一个值
	#                 不存在返回 0 或 空串;
	参数：
		key： 键标识
	返回值：
		值
	*/
	virtual std::string	Get_StringValue(int key ) = 0;
	virtual int			Get_IntValue(	int key) = 0;
	virtual std::string	Get_BinValue(	int key ) = 0;
	virtual std::string	Get_StringPair(	const char*key ) = 0;

	/*
	#==================================================================
	# 根据键获得所有值 --- 返回值 list
	#                 不存在返回 空表
	参数：
		key： 键标识
	返回值：
		值列表
	*/
	virtual vector<std::string>	Get_StringValue_List(	int key) = 0;
	virtual vector<int>			Get_IntValue_List(		int key) = 0;
	virtual vector<std::string>	Get_BinValue_List(		int key) = 0;
	virtual vector<std::string>	Get_StringPair_List(	const char*key ) = 0;

	/*
	#==================================================================
	// 转为字符串 --- 调试用
	*/
	virtual std::string to_String() = 0;

	/*
	#==================================================================
	// 序列化/反序列化
	*/
	virtual std::string SerializeToString() = 0;
	virtual int ParseFromString(std::string binobject) = 0;

	/*
	#==================================================================
	// 编码/解码
	*/
	virtual std::string Encode() = 0;

	/*
		解码一段数据
		参数：
			binobject： 二进制消息数据，可能包含1或多个消息
		返回值：
			-1,-2, 数据长度不足
			-100，-101，错误的数据
			大于0，解码的第一个消息长度
	*/
	virtual int Decode(std::string binobject) = 0;

	/*
	#==================================================================
	// 调试 - Dump
	*/
	virtual void Dump() = 0;
};

/*
	请求消息存取接口
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
	回应消息存取接口
*/
class IResponseMessage : public IBaseMessage
{
public:
	IResponseMessage() {}
	virtual ~IResponseMessage() {}

	virtual int Get_Info() = 0;
	virtual int Get_Error() = 0;
};



// 设置客户端Id
void				Set_ClientId(const char* clientId);

// 设置会话 Token
void				Set_Token(const char* token);

// 返回会话 Token
std::string			Get_Token();
/*
创建一个新请求消息
*/
IRequestMessage*	NewInstance_IRequestMessage(int cmd, int subCmd = SUBCMD_STUB);

/*
	解码一个请求消息
*/
IRequestMessage*	NewInstance_IRequestMessage(const unsigned char* pb, int len, OUT int* pMsgLen = NULL );
IRequestMessage*	NewInstance_IRequestMessage(const std::string binObject, OUT int* pMsgLen = NULL);


/*
创建一个新回应消息
*/
IResponseMessage*	NewInstance_IResponseMessage(int error, int info = 0);

/*
解码一个回应消息
*/
IResponseMessage*	NewInstance_IResponseMessage(const unsigned char* pb, int len, OUT int* pMsgLen = NULL);
IResponseMessage*	NewInstance_IResponseMessage(const std::string binObject, OUT int* pMsgLen = NULL);

