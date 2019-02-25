#pragma once
#ifndef _RPCMESSAGE_H_
#define _RPCMESSAGE_H
#include "RpcMessageBase.h"

class RpcMsg :public BaseMessage<RpcMessage, IRpcMessage>
{
public:
	RpcMsg(){}

	RpcMsg(int cmd, int subCmd) {
		set_cmd(cmd);
		set_subcmd(subCmd);
	}

	virtual int Get_MsgNo() {
		return msgno();
	}

	virtual int Get_Cmd() {
		return cmd();
	}

	virtual int Get_SubCmd() {
		return subcmd();
	}

	// #  转为字符串，调试用
	std::string to_String() {
		std::string str = std_string_format(
			"==Dump RequestMessage@%p\n"
			"  msgno : %d\n"
			"  cmd   : %d\n"
			"  subcmd: %d\n"
			,
			this, Get_MsgNo(), Get_Cmd(), Get_SubCmd());
		std::string strList = BaseMessage::to_String();

		str += strList;
		return str;
	}
};
#endif
