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
//	BYTE  YearEngineer;                       // ������
//	BYTE  Type;                               // �ļ�����
//	WORD  Ofs1;                               // һƫ��
//	BYTE  Len1;                               // һ����
//	BYTE  SignArr[0x10];                      // һ����
//	WORD  Ofs2;                               // ��ƫ��
//	BYTE  Len2;                               // ������
//	DWORD Crc32;                              // ��У��
//	BYTE  Method;                             // ����
//	WORD  Cure1, Cure2, Cure3, Cure4, LenAdj; // ��������
//	WORD  ObjIdx;
//	DWORD ID;
//	BYTE  LibNo;
//} TFileRec4;                                   // x4_header;

#pragma pack(push,1)

#define GROUP_SIZE		0x10000					// ÿ���¼��
#define VLB_MAGIC		(DWORD)'BLvK'

//
// ÿ0x10000����¼һ��
//

typedef struct	// size: 0xC
{
	DWORD   dwBaseID;						// �������Id
	DWORD	dwOffs;							// ���������ļ�ƫ�ƣ�Ҳ��Rec��¼ƫ��
	DWORD   dwSize;							// �������ݴ�С
	WORD	wRecCount;						// �����¼��
	WORD	wRev;							// 
}RecGroup;

//
// �ϼ�¼���ļ���ʽ
//
typedef struct
{
	DWORD	dwMagic;
	DWORD	dwRecCount;						// ��¼��
	DWORD	dwInvalidCount;					// ��Ч��¼��
	DWORD	dwFileSize;						// �ļ���С

	__time64_t timeCreate;
	__time64_t timeEnd;

	DWORD	dwBaseNo;						// ��¼�Ż�׼ - ��ʼ��¼��
	DWORD	dwEndNo;						// ������¼��
	DWORD	dwMaxBlockSize;					// �������С
	WORD    wGroupCount;					// ������
	BYTE	Type;							// �ļ�����
	BYTE	bRev;

	BYTE	rev[16];

	RecGroup group[256];					// 16 M ��
} LibHead4;


// �Ͽ�䳤��¼

typedef struct	// size: 0xC - 0x1C
{
	WORD  vID;								// ����Id & 0xFFFF
	WORD  Ofs1;								// ����1ƫ�ƣ�Ϊ0xFFFFʱ����¼��Ч
	WORD  Ofs2;
	BYTE  Len1;								// ����1 Sign ����, ��Len1Ϊ0ʱ����Sign0/sign����
	BYTE  Len2;
	DWORD Crc32;							// ����2��CRC
	BYTE  nLen;								// ��¼���ȣ����ڿ��ټ�����һ��¼ƫ��
	BYTE  Method;							// Ϊ0ʱ������ΪSign0, ��Clean1\Clean2...LenAdj ����
											// ��0ʱ������Ϊ�ṹ _CureAndSign	// 
											// ��Len1Ϊ0ʱ��Sign0/signû��
} FileRecBase4;

typedef struct
{
	WORD  Clean1;
	WORD  Clean2;
	WORD  Clean3;
	WORD  Clean4;
	WORD  LenAdj;    // ��������
} CureData4;

typedef struct : FileRecBase4	// size: 0xC - 0x1C
{
	// ��Len1Ϊ0ʱ��Sign0/signû��
	union
	{
		struct // _CureAndSign
		{
			CureData4 cure;					// �������
			BYTE  sign[0x10];				// ����1�����0x10
		} Data;
		BYTE Sign0[0x10];					// ����1�����0x10
	};
} FileRec4;

// ������¼��ʽ������ʱ
typedef struct
{
	// ������Ϣ
	DWORD vID;								// ����Id
	
	// ����1
	WORD  Ofs1;								// ����1ƫ�ƣ�Ϊ0xFFFFʱ����¼��Ч
	BYTE  Len1;								// ����1 Sign ����
	BYTE  SignArr[0x11];					// ����1�����0x10

	// ����2
	WORD  Ofs2;
	BYTE  Len2;
	DWORD Crc32;							// ����2��CRC

											// ɱ������
	BYTE  Method;
	WORD  Clean1;
	WORD  Clean2;
	WORD  Clean3;
	WORD  Clean4;
	WORD  LenAdj;							// ��������


	// ������Ϣ
	char date[12];							// ����
	WORD  LibNo;							// ���������
	BYTE  Type;

	char  virName[65];						// ������
	char  *modName;							// ģ����		--- ���ڲ���cppģ���Լ��ӿں�����
	char  *func0;							// ����0����	--- 
	char  *func1;							// ����1����

} FileRec4Full, * RecPtr;


// FTRecs.cpp ��ʹ��
typedef struct
{
	// ������Ϣ
	DWORD vID;								// ����Id

	Decode func0;							// ����0����	--- 
	Clean  func1;							// ����1����

											// ����1
	WORD  Ofs1;								// ����1ƫ�ƣ�Ϊ0xFFFFʱ����¼��Ч
	BYTE  Len1;								// ����1 Sign ����
	BYTE  SignArr[0x11];					// ����1�����0x10

											// ����2
	WORD  Ofs2;
	BYTE  Len2;
	DWORD Crc32;							// ����2��CRC

											// ɱ������
	BYTE  Method;
	WORD  Clean1;
	WORD  Clean2;
	WORD  Clean3;
	WORD  Clean4;
	WORD  LenAdj;							// ��������

	char  virName[65];						// ������

											//										// ������Ϣ
	//char date[12];							// ����
	//WORD  LibNo;							// ���������
	//char  Type;

	//char  *modName;							// ģ����		--- ���ڲ���cppģ���Լ��ӿں�����

} FileCodeRec4;

// �ڲ��������¼��Ϣ�ṹ
typedef struct
{
	FileCodeRec4* pRec;				// ָ���ڲ���¼��������
	DWORD count;						// 
} FileCodeRec4_Info;

#pragma pack(pop)

char* Bin2Hex(BYTE *buffer, int Len);
