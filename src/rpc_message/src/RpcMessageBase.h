#pragma once
#ifndef _RPCMESSAGEBASE_H_
#define _RPCMESSAGEBASE_H_
#include <stdarg.h>
#include "IRpcMessage.h"
#include "Utils.h"

/*
#======================================================================
#  消息处理基础类
#    处理请求与回应消息中的strvals / intvals / binvals / strpairs列表项
#    处理消息的序列化和反序列化，编码与解码
#    提供数据存取便捷操作接口:
#       增加键值
#       获取键值
#       获取键值列表
#
*/
#ifndef _WIN32
static int _vscprintf(const char * format, va_list pargs) {
	int retval;
	va_list argcopy;
	va_copy(argcopy, pargs);
	retval = vsnprintf(NULL, 0, format, argcopy);
	va_end(argcopy);
	return retval;
}

#endif


template <class T, class IB >
class BaseMessage: public T, public IB
{
public:
	/*
	#==================================================================
	# 增加键值
	*/
	void Add_StringValue(int key, const char* value)
	{
		StringValue* keyVal = this->add_strvals();

		keyVal->set_key(key);
		keyVal->set_value(value);
	}

	void Add_IntValue(int key, int value)
	{
		IntValue* keyVal = this->add_intvals();

		keyVal->set_key(key);
		keyVal->set_value(value);
	}

	void Add_BinValue(int key, std::string value)
	{
		BinValue* keyVal = this->add_binvals();

		keyVal->set_key(key);
		keyVal->set_value(value);
	}

	void Add_StringPair(const char* key, const char* value)
	{
		StringPair* keyVal = this->add_strpairs();

		keyVal->set_key(key);
		keyVal->set_value(value);
	}


	/*
	#==================================================================
	# 根据键获得一个值 --- 如果同一键有多个值，仅返回第一个值
	#                 不存在返回 0 或 空串
	*/

	const char* Get_StringValue(int key)
	{
		int size = this->strvals_size();
		if (size != 0) {
			for (int i = 0; i < size; i++) {
				const StringValue& keyVal = this->strvals(i);
				if (keyVal.key() == key)
				{
					/*
					const ::std::string& val = keyVal.value();
					int size0 = val.size();
					if (size0 == 0)
					return "";
					return val;
					*/
					return  keyVal.value().c_str();
				}
			}
		}
		return g_empty.c_str();
	}

	int			Get_IntValue(int key)
	{
		int size = this->intvals_size();
		if (size != 0) {
			for (int i = 0; i < size; i++) {
				const IntValue& keyVal = this->intvals(i);
				if (keyVal.key() == key)
					return keyVal.value();
			}
		}
		return 0;
	}

	std::string g_empty;

	const char*	Get_BinValue(int key, OUT int * pDataSize)
	{
		int size = this->binvals_size();
		if (size != 0) {
			for (int i = 0; i < size; i++) {
				const BinValue& keyVal = this->binvals(i);
				if (keyVal.key() == key) 
				{
					*pDataSize = keyVal.value().size();
					return keyVal.value().c_str();
				}		
			}
		}
		*pDataSize = 0;
		return g_empty.c_str();
	}

	const char* Get_StringPair(const char*key)
	{
		int size = this->strpairs_size();
		if (size != 0) {
			for (int i = 0; i < size; i++) {
				const StringPair& keyVal = this->strpairs(i);
				if ( keyVal.key() == key)
					return keyVal.value().c_str();
			}
		}
		return g_empty.c_str();
	}

	/*
	#==================================================================
	# 根据键获得所有值 --- 返回值 list
	#                 不存在返回 None
	*/
	int Get_StringValue_List(int key, size_t* pMaxBufSize, OUT char** plpBuf, size_t iBufCount)
	{
		int size = this->strvals_size();
		if (size == 0)
			return 0;

		int nCount = 0;
		for (int i = 0; i < size; i++) {
			const StringValue& keyVal = this->strvals(i);
			if (keyVal.key() == key) {
				++nCount;
				const std::string vv = keyVal.value();
				if (*pMaxBufSize < vv.size())
				{
					*pMaxBufSize = vv.size();
				}
				if (iBufCount > 0)
				{
#ifdef _WIN32
					::strcpy_s(*plpBuf, *pMaxBufSize,vv.c_str());
#else
					strncpy(*plpBuf, vv.c_str(), *pMaxBufSize);
#endif
					++plpBuf;
					--iBufCount;
				}
			}
		}
		return nCount;
	}

	int			Get_IntValue_List(int key, OUT int* plpInt, size_t iIntCount)
	{
		int size = this->intvals_size();
		if (size == 0)
			return 0;

		int nCount = 0;
		for (int i = 0; i < size; i++) {
			const IntValue& keyVal = this->intvals(i);
			if (keyVal.key() == key)
			{
				++nCount;
				if (iIntCount > 0)
				{
					*plpInt = keyVal.value();
					++plpInt;
					--iIntCount;
				}
			}
		}

		return nCount;
	}

	int		Get_BinValue_List(int key, size_t* pMaxBufSize, OUT char** plpBuf, size_t iBufCount)
	{
		int size = this->binvals_size();
		if (size == 0)
			return 0;

		int nCount = 0;
		for (int i = 0; i < size; i++) {
			const BinValue& keyVal = this->binvals(i);
			if (keyVal.key() == key)
			{
				++nCount;
				const std::string vv = keyVal.value();
				if (*pMaxBufSize < vv.size())
				{
					*pMaxBufSize = vv.size();
				}
				if (iBufCount > 0)
				{
#ifdef _WIN32
					::strcpy_s(*plpBuf, *pMaxBufSize, vv.c_str());
#else
					strncpy(*plpBuf, vv.c_str(), *pMaxBufSize);
#endif
					++plpBuf;
					--iBufCount;
				}
			}
		}

		return nCount;
	}

	int Get_StringPair_List(const char*key, size_t* pMaxBufSize, OUT char** plpBuf, size_t iBufCount)
	{
		int size = this->strpairs_size();
		if (size == 0)
			return 0;

		int nCount = 0;
		for (int i = 0; i < size; i++) {
			const StringPair& keyVal = this->strpairs(i);
			if (keyVal.key() == key)
			{
				++nCount;
				const std::string vv = keyVal.value();
				if (*pMaxBufSize < vv.size())
				{
					*pMaxBufSize = vv.size();
				}
				if (iBufCount > 0)
				{
					const std::string vv = keyVal.value();
#ifdef _WIN32				
				::strcpy_s(*plpBuf, *pMaxBufSize, vv.c_str());
#else
				strncpy(*plpBuf, vv.c_str(), *pMaxBufSize);
#endif
					++plpBuf;
					--iBufCount;
				}
			}
		}

		return nCount;
	}

	/*
	#==================================================================
	// 序列化/反序列化
	*/
	std::string SerializeToString() 
	{
		std::string ret;
		T::SerializeToString(&ret);
		return ret;
	}

	int ParseFromString(const std::string binobject)
	{
		if (T::ParseFromString(binobject ))
			return 0;
		return -100;
	}

	/*
	#==================================================================
	// 编码/解码
	'''
	# 序列化并进行编码变换
	#  以二进制字节字符串的方式返回。

	# 变换后格式：2 字节crc16 + 2 字节后续数据长度 + Xor变换数据
	# ..crc16 为crc之后数据的CRC
	*/

	std::string Encode()
	{
		std::string ret;
		std::string src = SerializeToString();

		int len = src.size();
		ret.resize(2 + 2 + len);
		char* p = (char*)ret.data();

		XorData(src.c_str(), 0, len, ret, 4);

		p[2] = (len >> 8) & 0xff;
		p[3] = (len) & 0xff;
		unsigned short crc = crc_16((unsigned char*)ret.c_str() + 2, len+2);
		p[0] = (crc >> 8) & 0xff;
		p[1] = (crc) & 0xff;

		return ret;
	}

	/*
	#
	# 进行解码变换并反序列化
	#
	*/
	/*
	解码一段数据
	参数：
	binobject： 二进制消息数据，可能包含1或多个消息
	返回值：
	-1,-2, 数据长度不足
	-100，-101，错误的数据
	大于0，解码的第一个消息长度
	*/
	int Decode(const std::string binobject)
	{
		// 完整性检查 / 解码
		// 长度检查
		//std::string strSrc = binobject;
		int len = binobject.size();
		if (len < 4)
			return -1;
		const unsigned char* p = (unsigned char*)binobject.c_str();
		int lens = (((int)p[2]) << 8);
		lens |= p[3];
		if (lens + 4 > len)	// 如果消息长度大于数据区长度，说明消息尚未接收完毕，需要继续接收数据直到消息接收完整
			return -2;
		
		// CRC 校验
		unsigned short crc = crc_16((unsigned char*)p + 2, lens + 2);
		unsigned short crc0 = ((unsigned short)p[0]) << 8;
		crc0 |= p[1];
		if (crc != crc0)
			return - 100;

		// XOR
		std::string dest;
		dest.resize(lens);
		char* p2 = (char*)dest.c_str();
		XorData((char*)p + 4, 0, lens, dest, 0);

		// 
		if (ParseFromString(dest) == 0)
			return lens+4;
		else
			return -101;
	}


	/*
	#==================================================================
	# 转为字符串，调试用
	*/
	std::string std_string_format(const char * _Format, ...) 
	{
		std::string tmp;

		va_list marker;
		va_start(marker, _Format);

		size_t num_of_chars = _vscprintf(_Format, marker);

		tmp.resize(num_of_chars);
		tmp.reserve(num_of_chars+1);
		
#ifdef _WIN32
		vsprintf_s((char *)tmp.data(), tmp.capacity(), _Format, marker);
#else
		vsprintf((char*)tmp.data(), _Format, marker);
#endif
		va_end(marker);

		return tmp;
	}

	std::string toString_StringValue( )
	{
		std::string str = "";
		int size = this->intvals_size();
		for (int i = 0; i < size; i++) {
			if (i == 0)
				str.append("  strvals\n");
			const StringValue& keyVal = this->strvals(i);
			std::string tmp = std_string_format("\t  %5d: %s\n", keyVal.key(), keyVal.value().c_str());
			str += tmp;
		}
		return str;
	}

	std::string toString_IntValue()
	{
		std::string str = "";
		int size = this->intvals_size();
		for (int i = 0; i < size; i++) {
			if (i == 0) {
				str.append("  intvals\n");
			}
			const IntValue& keyVal = this->intvals(i);
			std::string tmp = std_string_format("\t  %5d: %d\n", keyVal.key(), keyVal.value());
			str += tmp;
		}
		return str;
	}

	std::string toString_StringPair()
	{
		std::string str = "";
		int size = this->strpairs_size();
		for (int i = 0; i < size; i++) {
			if (i == 0)
				str.append("  strvals\n");
			const StringPair& keyVal = this->strpairs(i);
			std::string tmp = std_string_format("\t  %5s: %s\n", keyVal.key().c_str(), keyVal.value().c_str());
			str += tmp;
		}
		return str;
	}

#define toString_List( obj_name, type_name, formatStr) \
	std::string toString_##obj_name( ) {		\
		std::string str = "";				\
		int size = this->obj_name##_size();		\
		for (int i = 0; i < size; i++) {	\
			if (i == 0) {					\
				str.append("  " #obj_name "\n");			\
			}								\
			const type_name& keyVal = this->obj_name(i);	\
			std::string tmp = std_string_format(formatStr, keyVal.key(), keyVal.value().c_str()); \
			str += tmp;						\
		}									\
		return str;							\
	}


	toString_List(strvals, StringValue,	"\t  %5d: %s\n")
//	toString_List(intvals, IntValue,	"\t  %d:%d\n")
	toString_List(binvals, BinValue,	"\t  %5d: %s\n")
//	toString_List(strpairs,StringPair,	"\t  %s:%s\n")

	// #  转为字符串，调试用
	std::string to_String( )
	{
		std::string str;
		str += toString_strvals();
//		*str += toString_StringValue();
		str += toString_IntValue();
		str += toString_binvals();
		str += toString_StringPair();

		return str;
	}

	// 调试 - Dump 
	void Dump()
	{
		std::string str = to_String();
		printf("%s\n", str.c_str());
	}

};
#endif
