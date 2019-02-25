#pragma once
#include "KVMessageBase.h"

class RequestMsg : public BaseMessage<RequestMessage, IRequestMessage>
{
public:
	RequestMsg() {
	}

	RequestMsg(int cmd, int subCmd) {
		set_cmd(cmd);
		set_subcmd(subCmd);
	}

	virtual int Get_MsgNo() {
		return msgno();
	}

	virtual int Get_Version() {
		return version();
	}

	virtual int Get_Cmd() {
		return cmd();
	}
	
	virtual int Get_SubCmd() {
		return subcmd();
	}
	
	virtual std::string Get_ClientId() {
		return id();
	}
	
	virtual std::string Get_Token() {
		return token();
	}


	// #  转为字符串，调试用
	std::string to_String() {
		std::string str = std_string_format(
			"==Dump RequestMessage@%p\n"
			"  msgno : %d\n"
			"  cmd   : %d\n"
			"  subcmd: %d\n"
			"  clientId: %s\n"
			"  token : %s\n",
			this, Get_MsgNo(), Get_Cmd(), Get_SubCmd(), Get_ClientId().c_str(), Get_Token().c_str() );
		std::string strList = BaseMessage::to_String();

		str += strList;
		return str;
	}

};

class ResponseMsg : public BaseMessage<ResponseMessage, IResponseMessage>
{
public:
	ResponseMsg() {

	}
	ResponseMsg(int error, int info = 0) {
		set_error(error);
		set_info(info);
	}

public:
	virtual int Get_Error() {
		return error();
	}

	virtual int Get_Info() {
		return info();
	}

	// #  转为字符串，调试用
	std::string to_String() {
		std::string str = std_string_format(
			"==Dump ResponseMessage@%p\n"
			"  error: %d\n"
			"  info : %d\n",
			this, Get_Error(), Get_Info() );
		std::string strList = BaseMessage::to_String();

		str += strList;
		return str;
	}

};

