#ifndef _LIBREC_H
#define _LIBREC_H

#include "VL_Const.h"
#include "pub.h"


// ��ṹ���͵ļ�������
#define	MAX_SUBTYPE     7
#define MAX_TYPE		7 // ���Ͷ���
#define SYM_BASE        0
#define SYM_BOOT        1
#define SYM_EXEP        2
#define SYM_FILE        3
//#define SYM_MEMO        4
//#define SYM_CA          5
#define SYM_PACK        6

// -------------------------------------------------------------------------------------- �������ýṹ
class CAntiVirEngine;
typedef	DWORD (__thiscall CAntiVirEngine::*_DECODE)();	// ���뺯��
typedef	DWORD (__thiscall CAntiVirEngine::*_CLEAR)();	// �ⶾ����
typedef	DWORD (__thiscall CAntiVirEngine::*_DETECT)();	// ���뺯��
typedef	DWORD (__thiscall CAntiVirEngine::*_UNPACK)();	// �ⶾ����
typedef	DWORD (__thiscall CAntiVirEngine::*_INIT)();	// ��ʼ����̬��������
typedef DWORD (__thiscall CAntiVirEngine::*AveFunPtr)();// ��������
#define KV_AVE_CALL_METHOD(func) (AVE->*func)()	// ִ������������
extern	AveFunPtr	Smart;
extern	AveFunPtr	SkipWeb;
extern	AveFunPtr	FindFirstEntry;
extern	AveFunPtr	FindNextEntry;
extern	AveFunPtr	GetArchOff;
extern	AveFunPtr	CleanFile;
extern	AveFunPtr	CleanSector;
extern	AveFunPtr	DataScan;


#pragma pack(1)
// -------------------------------------------------------------------------------------- ��ṹ

// -------------------------------------------------------------------------------------- ������ṹ
typedef struct _TSectorRec
{
	WORD    Ofs1;				// һƫ��
	BYTE	Len1;				// һ����
	BYTE	SignArr[0x10];		// һ����
	WORD	Ofs2;				// ��ƫ��
	BYTE	Len2;				// ������
	DWORD	Crc32;				// ��У��
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
	BYTE	YearEngineer;		// ������
	BYTE	Type;				// �ļ�����
	WORD    Ofs1;				// һƫ��
	BYTE	Len1;				// һ����
	BYTE	SignArr[0x10];		// һ����
	WORD	Ofs2;				// ��ƫ��
	BYTE	Len2;				// ������
	DWORD	Crc32;				// ��У��
	BYTE	Method;				// ����
	WORD	Clean1, Clean2, Clean3, Clean4, LenAdj;	// ��������
	WORD	ObjIdx;
	DWORD	ID;
} TFileRec;					// x4_header;

typedef struct __TPackRec
{
	BYTE	Type;				// �ļ�����
	WORD    Ofs1;				// һƫ��
	BYTE	Len1;				// һ����
	BYTE	SignArr[0x10];		// һ����
	WORD	Ofs2;				// ��ƫ��
	BYTE	Len2;				// ������
	DWORD	Crc32;				// ��У��
	WORD    ObjIdx;				//        { object no.    11  }
	DWORD   ID;					//        { Name ptr      13 15 }
}_TPackRec;

typedef _TPackRec   TExePackRec;// 
typedef _TPackRec   TPackRec;   // 

// OBJ���ŵ�ַ��Ϣ
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

	PBYTE		lpRecBuf;		// ��¼������
	DWORD		dwRecBufSize;
	DWORD		dwRecBufOffs;	// ��Ӽ�¼�ۼ�
	DWORD		dwRecCount;		// ��¼������-��ʼ��ʱ��д
	DWORD		dwRecNo;

	DWORD		dwObjCodeSize;
	PBYTE		lpObjBuf;		// OBJ������
	DWORD		dwObjBufSize;	// OBJ��������С-��ʼ��ʱ��д
	DWORD		dwObjBufOffs;	// OBJ������ƫ��
	DWORD		dwObjCount;
	DWORD		dwObjNo;

	XADDR*		lpAddr;			// ��ڵ�ַ������
} SUBTYPE;
// -------------------------------------------------------------------------------------- 
#pragma pack()


#endif // AVCREC_DEF 
