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
	基本消息数据存取接口
*/
class IRPCBaseMessage
{
public:

	IRPCBaseMessage() {}
	virtual ~IRPCBaseMessage() {}

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
	virtual const char*			Get_StringValue(int key ) = 0;
	virtual int					Get_IntValue(	int key) = 0;
	virtual const char*			Get_BinValue(	int key, OUT int * pDataSize ) = 0;
	virtual const char*			Get_StringPair(	const char*key ) = 0;

	/*
	#==================================================================
	# 根据键获得所有值 --- 返回值 list
	#                 不存在返回 空表
	参数：
		key： 键标识
	返回值：
		值列表
	*/
	/*	参数：
		iBufCount = 0时，返回 Value 数量,pMaxBufSize返回字符串数组中最大的缓冲区大小
		返回值：
		plpBuf 中存放value值，Get_StringValue_List 返回保存到plpBuf中的指针数量
	*/ 
	virtual int					Get_StringValue_List(	int key, OUT size_t* pMaxBufSize, OUT char** plpBuf = NULL, size_t iBufCount = 0) = 0;
	/*	参数：
		iBufCount = 0时，返回 Value 数量
		返回值：
		plpInt 中存放value值，Get_StringValue_List 返回保存到plpInt中的指针数量
	*/
	virtual int					Get_IntValue_List(		int key, OUT int* plpInt = NULL, size_t iIntCount = 0) = 0;
	/*	参数：
		iBufCount = 0时，返回 Value 数量,pMaxBufSize返回字符串数组中最大的缓冲区大小
		返回值：
		plpBuf 中存放value值，Get_StringValue_List 返回保存到plpBuf中的指针数量
	*/
	virtual int					Get_BinValue_List(		int key, OUT size_t* pMaxBufSize,OUT char** plpBuf = NULL, size_t iBufCount = 0) = 0;
	/*	参数：
		iBufCount = 0时，返回 Value 数量,pMaxBufSize返回字符串数组中最大的缓冲区大小
		返回值：
		plpBuf 中存放value值，Get_StringValue_List 返回保存到plpBuf中的指针数量
	*/
	virtual int					Get_StringPair_List(	const char*key, OUT size_t* pMaxBufSize,OUT char** plpBuf = NULL, size_t iBufCount = 0) = 0;

	/*
	#==================================================================
	// 转为字符串 --- 内部调试用，外部模块不允许调用
	*/
	virtual std::string			to_String() = 0;

	/*
	#==================================================================
	// 序列化/反序列化--- 内部使用，外部模块不允许调用
	*/
	virtual std::string			SerializeToString() = 0;
	virtual int					ParseFromString(const std::string binobject) = 0;

	/*
	#==================================================================
	// 编码/解码--- 内部使用，外部模块不允许调用
	*/
	virtual std::string			Encode() = 0;
	/*
		解码一段数据
		参数：
			binobject： 二进制消息数据，可能包含1或多个消息
		返回值：
			-1,-2, 数据长度不足
			-100，-101，错误的数据
			大于0，解码的第一个消息长度
	*/
	virtual int					Decode(const std::string binobject) = 0;

	/*
	#==================================================================
	// 调试 - Dump
	*/
	virtual void				Dump() = 0;
};

/*
	请求消息存取接口
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
创建一个新请求消息
*/
IRpcMessage*	NewInstance_IRpcMessage(int cmd, int = RPC_SUBCMD_STUB);

/*
	解码一个请求消息
*/
IRpcMessage*	NewInstance_IRpcMessage(const unsigned char* pb, int len, OUT int* pMsgLen = NULL );
IRpcMessage*	NewInstance_IRpcMessage(const std::string binObject, OUT int* pMsgLen = NULL);

///*
//创建一个新回应消息
//*/
//IResponseMessage*	NewInstance_IResponseMessage(int error, int info = 0);
//
///*
//解码一个回应消息
//*/
//IResponseMessage*	NewInstance_IResponseMessage(const unsigned char* pb, int len, OUT int* pMsgLen = NULL);
//IResponseMessage*	NewInstance_IResponseMessage(const std::string binObject, OUT int* pMsgLen = NULL);
//
