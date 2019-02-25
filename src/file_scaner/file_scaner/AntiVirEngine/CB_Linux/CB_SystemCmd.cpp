/*******************************************************************************
*						       文件操作外围函数                                *
********************************************************************************
*	Name : CB_File.cpp          Origian Place : BJ of China                    *
*	Create Data : 1998-2002     Now Version :   1.0                            *
*	Modify Time :               Translater : HeGong                            *
*==============================================================================*
*                        Modification History                                  *
*==============================================================================*
*         V1.0  1. Create this program.                                        *
*               2. 08/11/2005 fixed by Wangwei.运用新机制，改回外围函数        *
*******************************************************************************/

#include "stdafx.h"
// #include <windows.h>

#ifdef WINCE
#include "..\\..\\KVRT_CE\\KV_Scan.h"
#else
#include "../../AVEngP/CScan.h"
#endif

#include "../Include/pub.h"

// API结构
//--------------------------------------------------------------------------------------
class SystemFun;
typedef DWORD (SystemFun::*ApiExecutePtr)();

#pragma pack(1)
// api定义
typedef struct _SYSTEM_API
{
	SysFunType		eSysFunType;	// 函数类型
	char*			pAPIName;		// 名称
	ApiExecutePtr	pAPIFun;		// 所属函数

}SYSTEM_API, *PSYSTEM_API;
#pragma pack()

// 函数类
//--------------------------------------------------------------------------------------
class SystemFun
{
public:
	CAntiVirEngine	*AVE;
	BYTE			*pParamBuf;			// 参数缓冲
	DWORD			dwParamNum;			// 参数个数
	DWORD			dwParamCount;		// 参数计数

	SystemFun(CAntiVirEngine *pAVE);

private:
	static SYSTEM_API	ApiSystem[];	// api数组

public:
	ApiExecutePtr	GetApiPtr(SysFunType eSysFunType);
	DWORD			SetParamNum(WORD wValue);
	DWORD			Push(BYTE *pValue, DWORD dwLen);
	DWORD			Pop(BYTE *pValue, DWORD dwLen);

	// API函数
	DWORD			CopyFile();	
	DWORD			CoverFile();	
	DWORD			GetSystemDirectory();	
	DWORD           MoveFileEx();
};

// 函数数组
SYSTEM_API SystemFun::ApiSystem[] = 
{
	{SYSCMD_COPYFILE, "CopyFile", &SystemFun::CopyFile},
	{SYSCMD_COVERFILE, "CoverFile", &SystemFun::CoverFile},
	{SYSCMD_SYSTEMDIR, "GetSystemDirectory", &SystemFun::GetSystemDirectory},
	{SYSCMD_MOVEFILE, "MoveFileEx", &SystemFun::MoveFileEx},
	{SYSCMD_UNKNOWN, NULL, NULL},
};

// 函数功能：获得API函数地址
ApiExecutePtr SystemFun::GetApiPtr(SysFunType eSysFunType)
{
	SYSTEM_API  *pApiSystem = ApiSystem;

	while (pApiSystem->eSysFunType != SYSCMD_UNKNOWN)
	{
		if (pApiSystem->eSysFunType == eSysFunType)
			return pApiSystem->pAPIFun;
		pApiSystem++;
	}
	return NULL;
}

// 默认函数
SystemFun::SystemFun(CAntiVirEngine *pAVE) 
{
	AVE = pAVE;
	pParamBuf = NULL;
	dwParamCount = 0;
	dwParamNum = 0;
}

// 函数功能：设置参数个数
DWORD SystemFun::SetParamNum(WORD wValue)
{
	dwParamCount = 0;
	dwParamNum = wValue;
	*(WORD*)pParamBuf = wValue;
	return 1;
}

// 函数功能：压入函数
// 函数参数：pValue-参数指针，dwLen-参数长度
DWORD SystemFun::Push(BYTE *pValue, DWORD dwLen)
{
	BYTE	*pParamOffAddr;	// 参数偏移地址
	WORD	wDataLen = 0;	// 一个参数数据长度
	DWORD	i;

	// 参数是否合法
	if (pValue == NULL || dwParamCount > dwParamNum || dwLen == 0)
		return 0;

	// 存放参数长度的地址
	pParamOffAddr = pParamBuf + 2;

	// 首次压入参数
	if (dwParamCount == 0)
	{
		*(WORD*)(pParamOffAddr) = (WORD)dwLen;						// 压入参数偏移
		memcpy(pParamOffAddr+2*dwParamNum, &pValue, dwLen);	// 压入参数数据
	}
	else
	{
		// 取前几个是数据长度
		for (i = 0; i < dwParamCount; i ++)
			wDataLen += *(WORD*)(pParamOffAddr+i*2);
		*(WORD*)(pParamOffAddr+dwParamCount*2) = (WORD)dwLen;				// 压入参数偏移
		memcpy(pParamOffAddr+2*dwParamNum+wDataLen, &pValue, dwLen);	// 压入参数数据
	}

	dwParamCount++;
	return 1;
}

// 函数功能：弹出参数
// 函数返回：0－失败；1－成功
// 函数参数：dwValue-弹出地址
DWORD SystemFun::Pop(BYTE *pValue, DWORD dwLen)
{
	BYTE	*pParamOffAddr;	// 参数偏移地址
	WORD	wDataLen = 0;	// 一个参数数据长度
	DWORD	i;

	// 参数是否越界
	if (dwLen == 0 || dwParamCount > dwParamNum)
		return 0;

	// 存放参数长度的地址
	pParamOffAddr = pParamBuf + 2;

	// 首次压入参数
	if (dwParamCount == 0)
	{
		dwLen = *(WORD*)(pParamOffAddr);	
		memcpy(pValue, pParamOffAddr+2*dwParamNum, dwLen);
	}
	else
	{
		// 取前几个是数据长度
		for (i = 0; i < dwParamCount; i ++)
			wDataLen += *(WORD*)(pParamOffAddr+i*2);
		dwLen = *(WORD*)(pParamOffAddr+dwParamCount*2);
		memcpy(pValue, pParamOffAddr+2*dwParamNum+wDataLen, dwLen);
	}
	dwParamCount++;
	return 1;
}

// API函数
//--------------------------------------------------------------------------------------

// 函数功能：copyFile
// 输入参数：2
//           参数1：字符串类型；拷贝目的文件名称
//           参数2：字符串类型；拷贝源文件名称
// 输出参数：无
// 函数返回：0-失败；1-成功
DWORD SystemFun::CopyFile()
{
	return 0;
//	char	*pExistingFileName;
//	char	*pNewFileName;
////	DWORD LastErrorNumber;
//
//	// 获得参数
//	if (0 == Pop((BYTE*)&pNewFileName, sizeof(BYTE*)) ||
//		0 == Pop((BYTE*)&pExistingFileName, sizeof(BYTE*)))
//	{
//		return 0;
//	}
//
//	if (pNewFileName == NULL || pExistingFileName == NULL)
//		return 0;
//	// 拷贝文件
//	if (!::CopyFile(pExistingFileName, pNewFileName, TRUE))
//		return 0;
//	else
//		return 1;
}
	//--------------------------------------------------------------------------------------

	// 函数功能：moveFile
	// 输入参数：2
	//           参数1：字符串类型；拷贝目的文件名称
	//           参数2：字符串类型；拷贝源文件名称
	// 输出参数：无
	// 函数返回：0-失败；1-成功
DWORD SystemFun::MoveFileEx()
{
	return 0;
//	char	*pExistingFileName;
//	char	*pNewFileName;
////	DWORD LastErrorNumber;
//
//	// 获得参数
//	if (0 == Pop((BYTE*)&pNewFileName, sizeof(BYTE*)) ||
//		0 == Pop((BYTE*)&pExistingFileName, sizeof(BYTE*)))
//	{
//		return 0;
//	}
//
//	if (pNewFileName == NULL || pExistingFileName == NULL)
//		return 0;
//	// 拷贝文件
//	if (!::MoveFileEx(pExistingFileName, pNewFileName,MOVEFILE_DELAY_UNTIL_REBOOT))
//		return 0;
//	else
//		return 1;
}

// 函数功能：覆盖当前文件
// 参数个数：1
//           参数1：源的文件名称
// 输出参数：无
// 函数返回：0-失败；1-成功
DWORD SystemFun::CoverFile()
{
	return 0;
	//char		*pExistingFileName;
	////BYTE		Buf[0x10000];
	//DWORD		nr, r;
	//int			ret;
	//KVFILE		hSaveHandle;
	//DWORD		dwSaveAttr; 
	//FILETIME	aTime,cTime,mTime;
	//char		name[0x200];

	//// 获得参数
	//if (0 == Pop((BYTE*)&pExistingFileName, sizeof(BYTE*)))
	//	return 0;

	//// 打开源文件
	//hSaveHandle = fopen(pExistingFileName, "rb");
	//if (hSaveHandle == NULL) 
	//	return 0;

	//// 改变当前的文件状态
	//strcpy(name,AVE->Full_Name);
	//GetFileTime(AVE->hScanFile, &cTime, &aTime, &mTime);
	//CloseHandle(AVE->hScanFile);
	//dwSaveAttr = GetFileAttributes(name);
	//SetFileAttributes(name, FILE_ATTRIBUTE_NORMAL);
	//AVE->hScanFile = fopen(name, "r+b");
	//if (AVE->hScanFile == NULL) 
	//	return 0;

	//// 写文件
	//SetFilePointer(AVE->hScanFile, 0, 0, SEEK_SET);
	//ret = 1;
	//do 
	//{
	//	// 读源文件内容
	//	ReadFile(hSaveHandle, AVE->B_Temp, sizeof(AVE->B_Temp), &nr, NULL);
	//	if (nr == 0)
	//		break;
	//	// 写引擎文件内容
	//	WriteFile(AVE->hScanFile, AVE->B_Temp, nr, &r, NULL);
	//	if (nr!=r) 
	//	{
	//		ret = 0;
	//		break;
	//	}
	//} while (nr!=0);

	//SetFileTime(AVE->hScanFile, &cTime, &aTime, &mTime);
	//SetFileAttributes(name, dwSaveAttr);
	//return ret;
}

// 函数功能：获得系统路径
// 输入参数：无
// 输出参数：2
//           参数1：字符串指针
//           参数2：DWORD
// 函数返回：0-失败；1-成功
DWORD SystemFun::GetSystemDirectory()
{
	return 0;
	//char	*pBuffer = NULL;
	//char	*pSize = 0;

	//if (dwParamNum != 2)
	//	return 0;

	//// 获得参数
	//if (0 == Pop((BYTE*)&pBuffer, sizeof(BYTE*)) || 
	//	0 == Pop((BYTE*)&pSize, sizeof(BYTE*)))
	//{
	//	return 0;
	//}

	//// 获取系统路径
	//if (!::GetWindowsDirectory(pBuffer, *(DWORD*)pSize))
	//	return 0;
	//else
	//	return 1;
}

//
//--------------------------------------------------------------------------------------
// 函数功能：系统命令调用
// 函数返回：0－失败；1－成功
DWORD __thiscall CAntiVirEngine::SystemCmd(BYTE *pSystemCmdBuf)
{
	//WORD			wCmd = 0;				// 函数命令
	//BYTE			*p = NULL;				// 命令指针
	//SYSTEMCMD		*pSystemCmd;
	//ApiExecutePtr	Execute;
	//SystemFun		aSystemFun(this);

	//// 参数是否合法
	//if (pSystemCmdBuf == NULL)
	//	return 0;

	//// 命令格式
	//pSystemCmd = (SYSTEMCMD*)pSystemCmdBuf;

	//// 获取API函数
	//Execute = aSystemFun.GetApiPtr((SysFunType)pSystemCmd->wSysFunType);
	//if (Execute != NULL)
	//{
	//	// 赋值参数
	//	aSystemFun.pParamBuf = (BYTE*)&pSystemCmd->wParamNum;
	//	aSystemFun.SetParamNum(pSystemCmd->wParamNum);

	//	// 执行API
	//	return (aSystemFun.*Execute)();
	//}

	return 0;
}


