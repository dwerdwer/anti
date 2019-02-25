#pragma once

#include <time.h>
#include "VL_Const.h"
#include "VL_Type.h"
// #include "KVLibRec.h"

#define	MAX_FT	0x20

class CAntiVirEngine;

#ifdef __GNUC__
#define __thiscall

typedef unsigned long long __time64_t;
#endif // __GNUC__

typedef int(__thiscall CAntiVirEngine::*Decode)();
typedef int(__thiscall CAntiVirEngine::*Clean)();
typedef int(__thiscall CAntiVirEngine::*Detect)();
typedef int(__thiscall CAntiVirEngine::*Unpack)();


//typedef struct _TFileRec4
//{
//	BYTE  YearEngineer;                       // 添加年号
//	BYTE  Type;                               // 文件类型
//	WORD  Ofs1;                               // 一偏移
//	BYTE  Len1;                               // 一长度
//	BYTE  SignArr[0x10];                      // 一数据
//	WORD  Ofs2;                               // 二偏移
//	BYTE  Len2;                               // 二长度
//	DWORD Crc32;                              // 二校验
//	BYTE  Method;                             // 方法
//	WORD  Cure1, Cure2, Cure3, Cure4, LenAdj; // 方法参数
//	WORD  ObjIdx;
//	DWORD ID;
//	BYTE  LibNo;
//} TFileRec4;                                   // x4_header;

#pragma pack(push,1)

#define GROUP_SIZE		0x10000					// 每组记录数
#define VLB_MAGIC		(DWORD)'BLvK'

//
// 每0x10000条记录一组
//

typedef struct	// size: 0xC
{
	DWORD   dwBaseID;						// 本组基本Id
	DWORD	dwOffs;							// 本组数据文件偏移，也是Rec记录偏移
	DWORD   dwSize;							// 本组数据大小
	WORD	wRecCount;						// 本组记录数
	WORD	wRev;							// 
}RecGroup;

//
// 老记录库文件格式
//
typedef struct
{
	DWORD	dwMagic;
	DWORD	dwRecCount;						// 记录数
	DWORD	dwInvalidCount;					// 无效记录数
	DWORD	dwFileSize;						// 文件大小

	__time64_t timeCreate;
	__time64_t timeEnd;

	DWORD	dwBaseNo;						// 记录号基准 - 起始记录号
	DWORD	dwEndNo;						// 结束记录号
	DWORD	dwMaxBlockSize;					// 最大分组大小
	WORD    wGroupCount;					// 分组数
	BYTE	Type;							// 文件类型
	BYTE	bRev;

	BYTE	rev[16];

	RecGroup group[256];					// 16 M 条
} LibHead4;


// 老库变长记录

typedef struct	// size: 0xC - 0x1C
{
	WORD  vID;								// 病毒Id & 0xFFFF
	WORD  Ofs1;								// 特征1偏移，为0xFFFF时本记录无效
	WORD  Ofs2;
	BYTE  Len1;								// 特征1 Sign 长度, 当Len1为0时，无Sign0/sign数据
	BYTE  Len2;
	DWORD Crc32;							// 特征2的CRC
	BYTE  nLen;								// 记录长度，用于快速计算下一记录偏移
	BYTE  Method;							// 为0时，后续为Sign0, 无Clean1\Clean2...LenAdj 数据
											// 非0时，后续为结构 _CureAndSign	// 
											// 当Len1为0时，Sign0/sign没有
} FileRecBase4;

typedef struct
{
	WORD  Clean1;
	WORD  Clean2;
	WORD  Clean3;
	WORD  Clean4;
	WORD  LenAdj;    // 方法参数
} CureData4;

typedef struct : FileRecBase4	// size: 0xC - 0x1C
{
	// 当Len1为0时，Sign0/sign没有
	union
	{
		struct // _CureAndSign
		{
			CureData4 cure;					// 清除参数
			BYTE  sign[0x10];				// 特征1，最大0x10
		} Data;
		BYTE Sign0[0x10];					// 特征1，最大0x10
	};
} FileRec4;

// 完整记录格式，构建时
typedef struct
{
	// 基本信息
	DWORD vID;								// 病毒Id
	
	// 特征1
	WORD  Ofs1;								// 特征1偏移，为0xFFFF时本记录无效
	BYTE  Len1;								// 特征1 Sign 长度
	BYTE  SignArr[0x11];					// 特征1，最大0x10

	// 特征2
	WORD  Ofs2;
	BYTE  Len2;
	DWORD Crc32;							// 特征2的CRC

											// 杀毒参数
	BYTE  Method;
	WORD  Clean1;
	WORD  Clean2;
	WORD  Clean3;
	WORD  Clean4;
	WORD  LenAdj;							// 方法参数


	// 附加信息
	char date[12];							// 日期
	WORD  LibNo;							// 所属库序号
	BYTE  Type;

	char  virName[65];						// 病毒名
	char  *modName;							// 模块名		--- 用于查找cpp模块以及接口函数名
	char  *func0;							// 函数0名字	--- 
	char  *func1;							// 函数1名字

} FileRec4Full, * RecPtr;


// FTRecs.cpp 中使用
typedef struct
{
	// 基本信息
	DWORD vID;								// 病毒Id

	Decode func0;							// 函数0名字	--- 
	Clean  func1;							// 函数1名字

											// 特征1
	WORD  Ofs1;								// 特征1偏移，为0xFFFF时本记录无效
	BYTE  Len1;								// 特征1 Sign 长度
	BYTE  SignArr[0x11];					// 特征1，最大0x10

											// 特征2
	WORD  Ofs2;
	BYTE  Len2;
	DWORD Crc32;							// 特征2的CRC

											// 杀毒参数
	BYTE  Method;
	WORD  Clean1;
	WORD  Clean2;
	WORD  Clean3;
	WORD  Clean4;
	WORD  LenAdj;							// 方法参数

	char  virName[65];						// 病毒名

											//										// 附加信息
	//char date[12];							// 日期
	//WORD  LibNo;							// 所属库序号
	//char  Type;

	//char  *modName;							// 模块名		--- 用于查找cpp模块以及接口函数名

} FileCodeRec4;

// 内部含代码记录信息结构
typedef struct
{
	FileCodeRec4* pRec;				// 指向内部记录分类数组
	DWORD count;						// 
} FileCodeRec4_Info;

#pragma pack(pop)

char* Bin2Hex(BYTE *buffer, int Len);
