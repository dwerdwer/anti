#pragma once

#include "VL_Const.h"

#define INFO_IS_TEXT			0x0001		// 是文本文件

#define INFO_HAS_LESSDATA		0x0800		// Exe文件大小不足
#define INFO_HAS_MOREDATA		0x1000		// Exe文件尾部有附加数据
#define INFO_HAS_SIGN			0x2000		// 有签名
#define INFO_HAS_SPECIAL_SEC	0x4000		// PE/NE 有特殊节
#define INFO_HAS_EXECDATA		0x8000		// PE/NE 有可执行数据段


// wCPUType
enum CPUTYPE
{
	CPU_UNKNOWN = 0,
	CPU_X86,
	CPU_X64,
	CPU_ARM32,
	CPU_ARM64,
	CPU_MIPS
};

// wImageType
enum IMAGETYPE
{
	IMAGE_UNKNOWN = 0,
	IMAGE_EXE,		// EXE // .OUT
	IMAGE_DLL,		// DLL // .SO
	IMAGE_SYS,		// SYS // .KO
};

// bRecType;	// 记录类型
enum RECTYPE
{
	TYPE_UNKNOWN = 0,
	TYPE_BUILDTOOL = 1,
	TYPE_EXEP = 2,
	TYPE_ARCHIVE = 3,
	TYPE_BINDATA = 4,
};

//Subsystem
enum SUBSYSTEM
{
	SUBSYSTEM_UNKNOWN = 0,
	SUBSYSTEM_CUI,
	SUBSYSTEM_GUI,
	SUBSYSTEM_NATIVE
};

//Main Offset 偏移类型
enum MAINOFFSET_TYPE
{
	MAINOFFSET_TYPE_REL_WORD = 0, // 相对偏移 WORD
	MAINOFFSET_TYPE_REL_DWORD,    // 相对偏移 DWORD
	MAINOFFSET_TYPE_ABS_DWORD,    // 绝对偏移 DWORD
	MAINOFFSET_TYPE_ABS_WORD,     // 绝对偏移 WORD
	MAINOFFSET_TYPE_POT_DWORD     // 指针偏移 DWORD
};

#pragma pack(push,1)

// 一段特征码
typedef struct
{
	WORD	offs;			// 偏移, 从EntryPoint 开始
	WORD	len;			// 长度
	BYTE	code[1];		// 特征字节数组，长度是len
}CheckCode;

typedef struct
{
	WORD	wSize;			// 记录大小
	WORD	wRecNo;			// 记录序号
	BYTE	bCPUType;       // CPU类型
	BYTE	bRecType;		// 记录类型
	WORD	wFileType;      // 主文件类型
	WORD	wSubType;	    // 子文件类型
	BYTE	bImageType;     // 镜像类型  DLL、EXE、SYS
	BYTE	bSubSystem;	    // Rev(CUI、GUI)
	BYTE	bMainOffsetType;// Main偏移类型
	DWORD	dwMainOffset;	// Main偏移
	CheckCode checkCode;	// 特征码，可能是多个CheckCode连续存放，通过wSize与CheckCode.len来计算
}IDRec;

#define TYPE_LIB_MAGIC	(DWORD)'vKtF'

// 文件类型库数据文件,头部定义
typedef struct
{
	DWORD	dwMagic;
	DWORD	dwSize;			// 数据文件大小
	DWORD	dwRecCount;		// 记录数
	DWORD	dwRev[4];

}IDHeader;

#pragma pack(pop)

#pragma pack(push, 1)
typedef struct _TAG_PERFIX
{
	WORD  m_wOrdinal;
	char  m_szCPUType[4];
	char  m_szRecType[5];
	char  m_szMainFileType[20];
	char  m_szSubFileType[20];
	char  m_szImageType[4];
	char  m_szSubSystem[4];
	char  m_szMainOffsetType[20];
	DWORD m_dwMainOffset;
}TAG_PERFX, *PTAG_PERFIX;

typedef struct _TAG_SUBFILE_TYPE
{
	WORD   m_wType;
	char  *m_lpszName;
}TAG_MAINFILE_TYPE, *PTAG_MAINFILE_TYPE,
TAG_SUBFILE_TYPE, *PTAG_SUBFILE_TYPE;

typedef struct
{
	BYTE  m_bType;
	char *m_lpszName;
}TAG_CPUNAME, *PTAG_CPUNAME, TAG_SUBSYSTEM_TYPE, *PTAG_SUBSYSTEM_TYPE,
TAG_MAINOFFSET_TYPE, *PTAG_MAINOFFSET_TYPE, TGA_REC_TYPE, *PTGA_REC_TYPE,
TAG_IMAGE_TYPE, *PTAG_IMAGE_TYPE;
#pragma pack(pop)
