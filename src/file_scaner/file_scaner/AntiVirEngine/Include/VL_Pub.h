/****************************************************
 *
 *  病毒库中引用的公共外部引用，在此登记
 *
 *  整理：何公道
 *  日期：2003.12.9
 *
 ****************************************************/

#include "VL_Type.h"

#ifndef VL_PUB_H
#define VL_PUB_H

// 1.Prop结构
//--------------------------------------------------------------------------------------------------
#pragma pack(push,1)

// 系统命令
enum SysFunType
{
	SYSCMD_UNKNOWN = 0,
	SYSCMD_COPYFILE,
	SYSCMD_COVERFILE,
	SYSCMD_SYSTEMDIR,
	SYSCMD_MOVEFILE,
};
// 函数传递定义，除参数数据外，其他变量均为WORD
// 命令头＋函数类型＋参数个数＋各个参数长度＋参数数据
typedef struct _SYSTEMCMD
{
	WORD			wSysFunType;	// 函数类型
	WORD			wParamNum;		// 参数个数
} SYSTEMCMD;

#pragma pack(pop)

#endif //!VL_PUB_H
