#pragma once

#include "TypeDef.h"

//////////////////////////////////////////////////////////////////////////
// 日志询问模块

class IFileEx;

enum LogLevel{
	logFatal	= 50000,
	logError	= 40000,
	logWarn		= 30000,
	logInfo		= 20000,
	logDebug	= 10000,
	logTrace	= 0,
};

extern "C" void AV_CALLTYPE Log(const wchar_t* szModule, LogLevel level, const wchar_t* szInfo) ;
