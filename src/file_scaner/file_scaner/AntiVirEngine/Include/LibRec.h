#ifndef _LIBREC_H
#define _LIBREC_H

#include "VL_Const.h"
#include "pub.h"


// 库结构类型的几个定义
#define	MAX_SUBTYPE     7
#define MAX_TYPE		7 // 类型定义
#define SYM_BASE        0
#define SYM_BOOT        1
#define SYM_EXEP        2
#define SYM_FILE        3
//#define SYM_MEMO        4
//#define SYM_CA          5
#define SYM_PACK        6

// -------------------------------------------------------------------------------------- 函数调用结构
class CAntiVirEngine;
typedef	DWORD (__thiscall CAntiVirEngine::*_DECODE)();	// 解码函数
typedef	DWORD (__thiscall CAntiVirEngine::*_CLEAR)();	// 解毒函数
typedef	DWORD (__thiscall CAntiVirEngine::*_DETECT)();	// 解码函数
typedef	DWORD (__thiscall CAntiVirEngine::*_UNPACK)();	// 解毒函数
typedef	DWORD (__thiscall CAntiVirEngine::*_INIT)();	// 初始化静态变量函数
typedef DWORD (__thiscall CAntiVirEngine::*AveFunPtr)();// 外引函数
#define KV_AVE_CALL_METHOD(func) (AVE->*func)()	// 执行外引函数宏
extern	AveFunPtr	Smart;
extern	AveFunPtr	SkipWeb;
extern	AveFunPtr	FindFirstEntry;
extern	AveFunPtr	FindNextEntry;
extern	AveFunPtr	GetArchOff;
extern	AveFunPtr	CleanFile;
extern	AveFunPtr	CleanSector;
extern	AveFunPtr	DataScan;


#pragma pack(1)
// -------------------------------------------------------------------------------------- 库结构

// -------------------------------------------------------------------------------------- 特征码结构
typedef struct _TSectorRec
{
	WORD    Ofs1;				// 一偏移
	BYTE	Len1;				// 一长度
	BYTE	SignArr[0x10];		// 一数据
	WORD	Ofs2;				// 二偏移
	BYTE	Len2;				// 二长度
	DWORD	Crc32;				// 二校验
	BYTE    Method1;			// { HiBit 4 SubType, LowBit Clean method }
	WORD    Clean1, Clean2;		// { Clean MBR data }
	BYTE    Clean3;
	BYTE    Method2;
	WORD    Clean4, Clean5;
	BYTE    Clean6;
	WORD    ObjIdx;				// { object no.}
	DWORD   ID;					// { Virus ID  }
} TSectorRec;					// 

typedef struct _TFileRec
{
	BYTE	YearEngineer;		// 添加年号
	BYTE	Type;				// 文件类型
	WORD    Ofs1;				// 一偏移
	BYTE	Len1;				// 一长度
	BYTE	SignArr[0x10];		// 一数据
	WORD	Ofs2;				// 二偏移
	BYTE	Len2;				// 二长度
	DWORD	Crc32;				// 二校验
	BYTE	Method;				// 方法
	WORD	Clean1, Clean2, Clean3, Clean4, LenAdj;	// 方法参数
	WORD	ObjIdx;
	DWORD	ID;
} TFileRec;					// x4_header;

typedef struct __TPackRec
{
	BYTE	Type;				// 文件类型
	WORD    Ofs1;				// 一偏移
	BYTE	Len1;				// 一长度
	BYTE	SignArr[0x10];		// 一数据
	WORD	Ofs2;				// 二偏移
	BYTE	Len2;				// 二长度
	DWORD	Crc32;				// 二校验
	WORD    ObjIdx;				//        { object no.    11  }
	DWORD   ID;					//        { Name ptr      13 15 }
}_TPackRec;

typedef _TPackRec   TExePackRec;// 
typedef _TPackRec   TPackRec;   // 

// OBJ符号地址信息
typedef union tagXADDR
{
	struct	
	{
		_DECODE		DecodeAddr;
		_CLEAR		ClearAddr;
	}clean;
	struct	
	{
		_DETECT		DetectAddr;
		_UNPACK		UnpackAddr;
	}unpack;
}XADDR;

typedef struct tagSUBTYPE
{
	DWORD		dwRecSize;
	DWORD		dwNameOffs;
	DWORD		dwObjIdxOffs;
	DWORD		dwTypeOffs;

	PBYTE		lpRecBuf;		// 记录缓冲区
	DWORD		dwRecBufSize;
	DWORD		dwRecBufOffs;	// 添加记录累加
	DWORD		dwRecCount;		// 记录总条数-初始化时填写
	DWORD		dwRecNo;

	DWORD		dwObjCodeSize;
	PBYTE		lpObjBuf;		// OBJ缓冲区
	DWORD		dwObjBufSize;	// OBJ缓冲区大小-初始化时填写
	DWORD		dwObjBufOffs;	// OBJ缓冲区偏移
	DWORD		dwObjCount;
	DWORD		dwObjNo;

	XADDR*		lpAddr;			// 入口地址缓冲区
} SUBTYPE;
// -------------------------------------------------------------------------------------- 
#pragma pack()


#endif // AVCREC_DEF 
