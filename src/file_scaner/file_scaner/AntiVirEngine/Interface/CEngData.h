#pragma once

#include "../Include/VL_Const.h"
#include "../Include/VL_Type.h"
#include "../Include/VL_Pub.h"
#include "../Include/VL_Def.h"

#include "../Include/FileType.h"

#ifdef USE_FILE_HANDLE

typedef void* KVFILE;

#else
#ifndef NO_USE_IFILEEX

class IFileEx;
typedef IFileEx* KVFILE;

#else

#ifndef _FILE_DEFINED
struct FILE;
#endif // _FILE_DEFINED

typedef FILE* KVFILE;

#endif // USE_IFILEEX
#endif // USE_FILE_HANDLE

class CScan;

#pragma pack(push,1)

// ɨ������������
// ��˵����
// 1.������*�ű�־��Ҫɨ������Ի�������Ϊֻ������
struct CEngData
{
	DWORD		mSize;					// �ṹ��С
	WORD		Object_Type;			// ɨ��������ͣ�0/0x11/0x12=�ļ�/��������/������
	WORD		File_Type;				// �ļ����� - ��ͳ����

										// �ļ�ʶ��
	WORD		wFileType;				// �����ļ����Ͷ��壬�鶾�����ݴ�ֵʹ�ö�Ӧ��������
	WORD		wSubType;				// ������
	WORD		wCPUType;				// CPU �ܹ�		// CPUTYPE
	WORD		wImageType;				// �ļ�����		// IMAGETYPE
	WORD		wSubSystem;				// ��ϵͳ��1=Native��2=GUI��3=CUI, 5=OS2/CUI, 7=POSIX/CUI, 9=WINCE/CUI,16=IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION
	DWORD		dwFeatureOffs;			// ���������ļ�����ƫ��
	DWORD		dwFeatureLen;			// �������򳤶�
	DWORD		dwInfoFlags;			// ��Ϣ��־, INFO_XXX

	KVFILE		hScanFile;				// �ļ����
	KVFILE		hTempP;					// ��ʱ�ļ��������ѹ�ļ�
	DWORD		File_Length;			// *�ļ�����
	DWORD		Part_Length;			// *�ļ�Ƭ�γ��� - ʹ��Ƭ�β鶾��<= File_Length
	DWORD		ArchOffs;				// �ļ�ѹ����ƫ��  - �����ļ��ṹ����õ����ļ���С
										// File_Length>=ArchOffs, ˵���ļ�β���ж������ݣ�������ǩ����Zip������������
	DWORD		File_Data_Size;			// ��������ļ���С
	DWORD		RetFlags;				// ɨ�践�ر�־

										// �ļ�����
	char		Full_Name[0x500];		// *�ļ�ȫ����
	char		Zip_Name[0x500];		// *�ļ���ѹ�����ļ�����
	char		*Name;					// *�ļ�����ָ��
	char		*Ext;					// *�ļ���׺ָ�� 

										// ɨ����Ʊ���
	WORD		Entry_Count;			// ɨ�������ڴ���

	BYTE		Del_BreakPoint[0x2f];	// !�Ѿ��ϳ� ������ϵ����

	class		CScan  *pScan;			// *ɨ����ָ��	Scan

										// ����������
	BYTE		B_Header[0x1000];		// ����ļ�ͷ
	BYTE		B_Entry[0x400];			// ����ļ���һ��ڣ����
	BYTE		B_Next[0x400];			// ����ļ��ڶ���ڣ�����˵�
	BYTE		B_Tail[0x800];			// ����ļ�β
	BYTE		B_Misc[0x8000];			// ����ļ�ͷ
	BYTE		B_Temp[0x8000];			// ��ʱ��������Ӧ��

	BYTE		B_Misc2[0x8000];		// By Lawrence

	BYTE		B_Section[0x1000];		// PE/NE/ELF �α�	// Add @2017.9.21
	BYTE		bSectionCount;			// �α�����			// Add @2017.9.21


										// �������
	WORD		NExe_Format;			// ��ִ���ļ�����
	DWORD		EP;						// ָ���һ�����ָ�룬������ļ���ƫ����(phy off)
	DWORD		EP_Next;				// ָ����һ�����ָ�룬Ҳ��������ļ���ƫ����
	DWORD		IP32_Entry;				// ָ���һ�����ָ�룬�������
	DWORD		IP32_Next;				// ָ����һ�����ָ�룬�������
	DWORD		Header_Offset;			// EXEͷ������ƫ�Ƶ�ַ
	DWORD		Tail;					// �ļ�βָ��
	DWORD		Del_Exe_IP;				// !�Ѿ��ϳ� EXE���ָ��
	WORD		Del_Exe_CS;				// !�Ѿ��ϳ� EXE��ָ��
	DWORD		Del_NExe_IP;			// !�Ѿ��ϳ� NEXE��ָ��
	WORD		Del_NExe_CS;			// !�Ѿ��ϳ� NEXE���ָ��
	DWORD		DosCS;					// CS��ֵ
	DWORD		Sys2Exe;				// SYS�ļ�����ڵ�ַ
	WORD		Sect2Num;				// �ļ������ڱ�����
	BYTE		Del_Prop[0x240];		// !�Ѿ��ϳ�
	BYTE		Del_cProp[0x80];		// !�Ѿ��ϳ�		
	BYTE		ScriptBuff[0x9000];		// �ű����������ʱ��
	UINT_PTR	TempFunCall;			// !�Ѿ��ϳ� �ɿ���


										// ���̱���
	BYTE		Disk;					// *���̺�
	WORD		Disk_Max_CX;			// *���������������Ҳ����SectorsPerTrack
	BYTE		Disk_Max_DH;			// *�����������ţ�Ҳ����TracksPerCylinder
	BYTE		Drive[32];				// *����������
	DWORD		First_Data_Sector;		// *��һ�α�����������

										// ��������___�ڲ�����							   
	BYTE		*pFileRecAVE;			// *File������
	BYTE		Sect_bytType;			// *����
	DWORD		Sect_dwSector;			// *������
	DWORD		Sect_dwTrack;			// *�ŵ���
	BYTE		Sect_bytSize;			// *����
	BYTE		*pIsProgramBuf;			// *�жϳ��򻺳�

										// ��������___��������
	DWORD		dwByteSequence;			// ��д�ֽ����� 0_���� 1_����
	class		CVirtualMachine *pVirtualMachine;	// ���������   Emul - Lib0
	class		CMacro *pMacro;			// �����	    Macro

										// AVE����_________
	DWORD		Overlay;				// ���򸲸ǲ���Offset
	DWORD		OverlaySize;			// ���򸲸ǲ��ִ�С
	DWORD		MP;						// main�������ļ�ƫ��
	DWORD		IP32_Main;				// main������IP��ַ
	DWORD		dwAutoRec;				// �Զ�����������־

										// �����ռ�________
	DWORD		IsSkipVM;				// 0��������������򲻹������
	BYTE        *pSkipBuf;              // Skip������ָ��
	DWORD       dwSkipBufLen;           // SKip����������
	class		CVM		*pVM;			// Bochs ��������� VM - �ѿ� - Lib50
	class       CPE		*pPE;			// PE���غ����ļ��ṹ		
	class		CLE		*pLE;			// LE�ļ����� 		
	class		CNE		*pNE;			// NE�ļ����� 		
	class		CElf	*pElf;			// Elf�ļ����� 		
	class		CDos	*pDos;			// Dos�ļ����� 		
	class		CInf	*pInf;			// Inf�ļ����� 		
	class		CJava	*pJava;			// Java����	   
	class		CHelp	*pHelp;			// Help����	  

										// DataScanɨ��
	DWORD		bDataScanEnable;		// �Ƿ�����DataScanɨ�裨0:������,��0:���ã�
	BYTE		*plib_buf;				// ������ָ�룬������
	DWORD		lib_size;				// �������С��������
	class		CDataScan *pDataScan;	// DataScanɨ����
	char		HeruVirName[0x40];		// ����ʽ��������
	DWORD		HeruRet;				// ����ʽ�������

										//SCRIPTȫ�ֱ���
	DWORD		ScriptStatus;			// �ű��Ƿ���Ҫ�ٴα���
	DWORD		ScriptSign;				// ��ǰ�ű���״̬
	DWORD		IsSkipScript;			// 0�����ű�����ģ�飬���򲻹��ű�����ģ��
	DWORD		IsSkipScript2;			// ͬ�ϣ�����Script_Gen�Ŀ���
										//���ӽű�ɨ�� DataScan
	BYTE		*plib_script_buf;				// ������ָ�룬������
	DWORD		lib_script_size;				// �������С��������
	class		CDataScan *pDataScanScript;	    // DataScanɨ����

												//���Ӻ�ɨ�� DataScan
	BYTE		*plib_macro_buf;		// ������ָ�룬������
	DWORD		lib_macro_size;		        // �������С��������
	class		CDataScan *pDataScanMacro;	// DataScanɨ����

											//Android dex �ļ�֧��
	class		CDex		*pDex;
	//����
	//BYTE		ExternVal[0x4000-0xA0]; // Ԥ����չ����
	BYTE		ExternVal[0x4000 - 0xBC]; // Ԥ����չ����
};

extern "C" void Check_Error(CEngData* pAVE);

#pragma pack(pop)
