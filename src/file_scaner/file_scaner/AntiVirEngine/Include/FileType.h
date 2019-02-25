#pragma once

#include "VL_Const.h"

#define INFO_IS_TEXT			0x0001		// ���ı��ļ�

#define INFO_HAS_LESSDATA		0x0800		// Exe�ļ���С����
#define INFO_HAS_MOREDATA		0x1000		// Exe�ļ�β���и�������
#define INFO_HAS_SIGN			0x2000		// ��ǩ��
#define INFO_HAS_SPECIAL_SEC	0x4000		// PE/NE �������
#define INFO_HAS_EXECDATA		0x8000		// PE/NE �п�ִ�����ݶ�


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

// bRecType;	// ��¼����
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

//Main Offset ƫ������
enum MAINOFFSET_TYPE
{
	MAINOFFSET_TYPE_REL_WORD = 0, // ���ƫ�� WORD
	MAINOFFSET_TYPE_REL_DWORD,    // ���ƫ�� DWORD
	MAINOFFSET_TYPE_ABS_DWORD,    // ����ƫ�� DWORD
	MAINOFFSET_TYPE_ABS_WORD,     // ����ƫ�� WORD
	MAINOFFSET_TYPE_POT_DWORD     // ָ��ƫ�� DWORD
};

#pragma pack(push,1)

// һ��������
typedef struct
{
	WORD	offs;			// ƫ��, ��EntryPoint ��ʼ
	WORD	len;			// ����
	BYTE	code[1];		// �����ֽ����飬������len
}CheckCode;

typedef struct
{
	WORD	wSize;			// ��¼��С
	WORD	wRecNo;			// ��¼���
	BYTE	bCPUType;       // CPU����
	BYTE	bRecType;		// ��¼����
	WORD	wFileType;      // ���ļ�����
	WORD	wSubType;	    // ���ļ�����
	BYTE	bImageType;     // ��������  DLL��EXE��SYS
	BYTE	bSubSystem;	    // Rev(CUI��GUI)
	BYTE	bMainOffsetType;// Mainƫ������
	DWORD	dwMainOffset;	// Mainƫ��
	CheckCode checkCode;	// �����룬�����Ƕ��CheckCode������ţ�ͨ��wSize��CheckCode.len������
}IDRec;

#define TYPE_LIB_MAGIC	(DWORD)'vKtF'

// �ļ����Ϳ������ļ�,ͷ������
typedef struct
{
	DWORD	dwMagic;
	DWORD	dwSize;			// �����ļ���С
	DWORD	dwRecCount;		// ��¼��
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
