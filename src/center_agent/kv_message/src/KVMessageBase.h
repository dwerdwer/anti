#pragma once
#include <stdarg.h>
#include "IKVMessage.h"
#include "Utils.h"

/*
#======================================================================
#  ��Ϣ����������
#    �����������Ӧ��Ϣ�е�strvals / intvals / binvals / strpairs�б���
#    ������Ϣ�����л��ͷ����л������������
#    �ṩ���ݴ�ȡ��ݲ����ӿ�:
#       ���Ӽ�ֵ
#       ��ȡ��ֵ
#       ��ȡ��ֵ�б�
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
	# ���Ӽ�ֵ
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
	# ���ݼ����һ��ֵ --- ���ͬһ���ж��ֵ�������ص�һ��ֵ
	#                 �����ڷ��� 0 �� �մ�
	*/

	std::string Get_StringValue(int key)
	{
		int size = this->strvals_size();
		if (size != 0) {
			for (int i = 0; i < size; i++) {
				const StringValue& keyVal = this->strvals(i);
				if (keyVal.key() == key)
					return keyVal.value();
			}
		}
		return "";
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

	std::string	Get_BinValue(int key)
	{
		int size = this->binvals_size();
		if (size != 0) {
			for (int i = 0; i < size; i++) {
				const BinValue& keyVal = this->binvals(i);
				if (keyVal.key() == key)
					return keyVal.value();
			}
		}
		return "";
	}

	std::string Get_StringPair(const char*key)
	{
		int size = this->strpairs_size();
		if (size != 0) {
			for (int i = 0; i < size; i++) {
				const StringPair& keyVal = this->strpairs(i);
				if ( keyVal.key() == key)
					return keyVal.value();
			}
		}
		return "";
	}

	/*
	#==================================================================
	# ���ݼ��������ֵ --- ����ֵ list
	#                 �����ڷ��� None
	*/
	vector<std::string> Get_StringValue_List(int key)
	{
		vector<std::string> vals;
		int size = this->strvals_size();
		if (size == 0)
			return vals;

		for (int i = 0; i < size; i++) {
			const StringValue& keyVal = this->strvals(i);
			if (keyVal.key() == key)
				vals.push_back(keyVal.value());
		}

		return vals;
	}

	vector<int>			Get_IntValue_List(int key)
	{
		vector<int> vals;
		int size = this->intvals_size();
		if (size == 0)
			return vals;

		for (int i = 0; i < size; i++) {
			const IntValue& keyVal = this->intvals(i);
			if (keyVal.key() == key)
				vals.push_back(keyVal.value());
		}

		return vals;
	}

	vector<std::string>		Get_BinValue_List(int key)
	{
		vector<std::string> vals;
		int size = this->binvals_size();
		if (size == 0)
			return vals;

		for (int i = 0; i < size; i++) {
			const BinValue& keyVal = this->binvals(i);
			if (keyVal.key() == key)
				vals.push_back(keyVal.value());
		}

		return vals;
	}

	vector<std::string> Get_StringPair_List(const char*key)
	{
		vector<std::string> vals;
		int size = this->strpairs_size();
		if (size == 0)
			return vals;

		for (int i = 0; i < size; i++) {
			const StringPair& keyVal = this->strpairs(i);
			if (keyVal.key() == key)
				vals.push_back(keyVal.value());
		}

		return vals;
	}

	/*
	#==================================================================
	// ���л�/�����л�
	*/
	std::string SerializeToString() 
	{
		std::string ret;
		T::SerializeToString(&ret);

		return ret;
	}

	int ParseFromString(std::string binobject)
	{
		if (T::ParseFromString(binobject ))
			return 0;
		return -100;
	}

	/*
	#==================================================================
	// ����/����
	'''
	# ���л������б���任
	#  �Զ������ֽ��ַ����ķ�ʽ���ء�

	# �任���ʽ��2 �ֽ�crc16 + 2 �ֽں������ݳ��� + Xor�任����
	# ..crc16 Ϊcrc֮�����ݵ�CRC
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
	# ���н���任�������л�
	#
	*/
	/*
	����һ������
	������
	binobject�� ��������Ϣ���ݣ����ܰ���1������Ϣ
	����ֵ��
	-1,-2, ���ݳ��Ȳ���
	-100��-101�����������
	����0������ĵ�һ����Ϣ����
	*/
	int Decode(std::string binobject)
	{
		// �����Լ�� / ����
		// ���ȼ��
		int len = binobject.size();
		if (len < 4)
			return -1;
		const unsigned char* p = (unsigned char*)binobject.c_str();
		int lens = (((int)p[2]) << 8);
		lens |= p[3];
		if (lens + 4 < len)
			return -2;
		
		// CRC У��
		unsigned short crc = crc_16((unsigned char*)p + 2, lens + 2);
		unsigned short crc0 = ((unsigned short)p[0]) << 8;
		crc0 |= p[1];
		if (crc != crc0)
			return - 100;

		// XOR
		std::string dest;
		dest.resize(lens);
		XorData((char*)p + 4, 0, lens, dest, 0);

		// 
		if (ParseFromString(dest) == 0)
			return lens+4;
		else
			return -101;
	}


	/*
	#==================================================================
	# תΪ�ַ�����������
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

	// #  תΪ�ַ�����������
	std::string to_String( )
	{
		std::string str = "";
		str += toString_strvals();
//		str += toString_StringValue();
		str += toString_IntValue();
		str += toString_binvals();
		str += toString_StringPair();

		return str;
	}

	// ���� - Dump 
	void Dump()
	{
		std::string str = to_String();
		printf("%s\n", str.c_str());
	}

};