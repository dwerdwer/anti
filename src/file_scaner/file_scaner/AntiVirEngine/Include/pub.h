/*******************************************************************************
*                               �ඨ��                                         *
********************************************************************************
*                                                                              *
*   Name : AVE�ඨ��            Origian Place : BJ of China                    *
*   Create Data : 04/29/2005    Now Version :   1.0                            *
*   Modify Time :               Author:         Hegongdao & Wangwei            *
*                                                                              *
*==============================================================================*
*                           Modification History                               *
*==============================================================================*
*                     V1.0  1. Create this program.                            *
*                           2. 06/03/2005 ȫ��C++����ȥȫ�ֱ���                *
*                           3. 06/28/2005 ����������ȫ�����                   *
*                           4. 12/15/2005 �Ӻ����ȷ���ṹ                     *
*                           5. 03/02/2006 ��ֲCE��΢�����ṹ                   *
*                           6. 04/10/2006 ȥͷ�ļ���ȥ������ȥ����             *
*                           7. 04/14/2006 ���ڲ��ⲿ����ϸ��                   *
*                           8. 06/07/2006 ���������                           *
*                           9. 07/17/2006 ���º�ģ��                           *
*                           10.08/05/2006 ���½ű�ģ��                         *
*                           11.08/08/2006 �޸�ͷ�ļ�                           *
*                           12.10/27/2006 �ĺ�Ϊ��                             *
*                           13.08/24/2007 unpack��                             *
*                           14.08/05/2008 ��Ϊ����ֲ                           *
*******************************************************************************/

#ifndef _KV_PUB_H
#define _KV_PUB_H

#ifdef __GNUC__
#define __thiscall 
#define __stdcall 
#endif

#include "VL_Const.h"
#include "VL_Type.h"
#include "VL_Pub.h"
#include "VL_Def.h"
// #include "LibRec.h"

// Debug
extern "C" int DebugWriteFile(char* filename, BYTE* pBuffer, int Len);
extern "C" int DebugAppendFile(char* filename, BYTE* pBuffer, int Len);
// CLIB
extern "C" int DebugPrintf(const char* format,...);
extern "C" int FilePrintf(const char* filename, const char* format,...);


#ifdef USE_FILE_HANDLE

typedef void* KVFILE;

#else
#ifndef NO_USE_IFILEEX

class IFileEx;
typedef IFileEx* KVFILE;

#else

#ifndef _FILE_DEFINED
// struct FILE;
typedef struct _IO_FILE FILE;
#endif // _FILE_DEFINED

typedef FILE* KVFILE;

#endif // USE_IFILEEX
#endif // USE_FILE_HANDLE


//// wCPUType
//enum CPUTYPE
//{
//	CPU_UNKNOWN = 0,
//	CPU_X86,
//	CPU_X64,
//	CPU_ARM32,
//	CPU_ARM64,
//};
//
//// wImageType
//enum IMAGETYPE
//{
//	IMAGE_UNKNOWN = 0,
//	IMAGE_EXE,		// EXE
//	IMAGE_DLL,		// DLL / SO
//	IMAGE_SYS,
//};

#include "FileType.h"

// class CScan;

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
	BYTE		B_Section[0x1000];		// PE/NE/ELF �α�	// Add @2017.9.21
	BYTE		bSectionCount;			// �α�����			// Add @2017.9.21

	BYTE		B_Misc2[0x8000];		// By Lawrence

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


#pragma pack(pop)


class CAntiVirEngine : public CEngData
{

public:


public:

	// ����ӿں�������Ҫ�ⲿ��������
	//_________________________________________________________________________________CB_File.cpp
	DWORD	__thiscall	_LSeek(DWORD pos, DWORD mode);
	DWORD	__thiscall	_Seek(DWORD pos);
	DWORD	__thiscall	_Read(void* ptr, DWORD size);
	DWORD	__thiscall	_Write(void* ptr, DWORD size);
	BOOL	__thiscall	_SetEndOfFile(DWORD size);
	DWORD	__thiscall	Pack_Seek(DWORD pos, DWORD mode);
	DWORD	__thiscall	Pack_Read(void* ptr, DWORD size);
	DWORD	__thiscall	Pack_Copy_File();
	DWORD	__thiscall	MemBlock_Save(void* pBuffer, DWORD dwSize); // ����ָ�������ݿ�
	DWORD	__thiscall	MemBlock_Load(void* pBuffer);				// �ָ����ݿ鵽ָ��ָ��

	//_________________________________________________________________________________CB_Disk.cpp
	BOOL	__thiscall	_Read_25(DWORD disk, DWORD ssec, DWORD nsec, BYTE *buffer);
	BOOL	__thiscall	_Write_26(DWORD disk, DWORD ssec, DWORD nsec, BYTE *buffer);
	BOOL	__thiscall	_Write_13(DWORD disk, DWORD cx, DWORD dh, DWORD al, BYTE *buffer);
	BOOL	__thiscall	_Read_13(DWORD disk, DWORD cx, DWORD dh, DWORD al, BYTE *buffer);
	BYTE*	__thiscall	Get_Sector_Image(DWORD type);

	//_________________________________________________________________________________CB_Util.cpp
	DWORD	__thiscall	CheckFileExt();
	DWORD	__thiscall	CheckUserBreak(DWORD r);
	DWORD	__thiscall	Check_Sum(BYTE *data, DWORD size);
	DWORD	__thiscall	Check_Crc32(BYTE *data, DWORD size);

	//_________________________________________________________________________________CB_Memo.cpp
	void*	__thiscall	New(int length);
	void* __thiscall	Realloc(void* ptr, DWORD length);
	void	__thiscall	Delete(void *addr);
	void*	__thiscall	_malloc(DWORD size);  
	void	__thiscall	_free(void* memblock);

	//_________________________________________________________________________________CB_SystemCmd.cpp
	DWORD	__thiscall	SystemCmd(BYTE *pSystemCmdBuf);


	//_________________________________________________________________________________AVE.cpp
	int		CheckFileType_Zip();
	int		CheckFileType_MZ();
	int		CheckFileType_PE();
	int		CheckFileType_Txt();

public:

	// ���������������������
	DWORD	__thiscall	FindFirstEntry();
	DWORD	__thiscall	FindNextEntry();
	DWORD	__thiscall	Smart();	
	DWORD	__thiscall	SkipWeb();	
	DWORD	__thiscall	GetArchOff();
	DWORD	__thiscall	CleanFile();
	DWORD	__thiscall	CleanSector();
	DWORD	__thiscall	Initialize();		
	DWORD	__thiscall	InitializeVM();		
	void 	__thiscall	ClearData();
	void 	__thiscall	ClearDataVM();
	DWORD	__thiscall	Dispose();
	DWORD	__thiscall	DisposeVM();
	DWORD	__thiscall	InitializeDataScan();	
	DWORD	__thiscall	DisposeDataScan();
	DWORD	__thiscall	DataScan();

	DWORD	__thiscall	InitializeScriptScan();
	DWORD	__thiscall	DisposeScriptScan();

	DWORD	__thiscall	InitializeMacroScan();	
	DWORD	__thiscall	DisposeMacroScan();


	// �ѿǽ�ѹ�����ڴ����
#if DEBUG_CODE
	int		__thiscall	decode();
	int		__thiscall	clean();
	int		__thiscall	detect();
	int		__thiscall	unpack();
#else
	//_________________________________________________________________________________call Emul.cpp
	int		__thiscall	VM16();
	int		__thiscall	VM32();
	int		__thiscall	VMW();
	int		__thiscall	SCRIPT();

#endif

	// ����	�ڲ�����
	//_________________________________________________________________________________STR.cpp
	char	*_str_cpy(char *dst0 , const char *src0);
	char	*_str_ncpy(char *dst0 , const char *src0,size_t count);
	char	*_str_cat(char *s1, const char *s2);
	char	*_str_ncat(char* s1, char* s2, size_t n);
	char	*_str_str(const char *searchee, const char *lookfor);
	char	*_str_rev(char * src); 
	char	*_str_lwr(char *a);
	size_t	_str_len(const char *str);
	char	*_str_chr(const char *s1, int i);
	char	*_str_rchr(const char *s, int i);
	int		_str_cmp(const char *s1, const char *s2);
	int		_str_icmp(const char *s1, const char *s2);
	int		_str_ncmp(const char *s1, const char *s2, size_t n);
	int		_str_nicmp(const char *s1, const char *s2, size_t len3);
	wchar_t *_wcs_cpy(wchar_t * s1, const wchar_t * s2);
	size_t	_wcs_len(const wchar_t * s);
	wchar_t	_towlower(wchar_t c);
	int		_wcs_icmp(const wchar_t* pwcz1, const wchar_t* pwcz2);
	int     _wcs_cmp(const wchar_t * s1,const wchar_t * s2);
	int     _wcs_ncmp(const wchar_t * s1,const wchar_t * s2,size_t n);
	int     _wcs_nicmp(const wchar_t * s1,const wchar_t * s2,size_t n);
    size_t  _wcstombs(char *s ,const wchar_t *pwcs,size_t  n);
	size_t _mbstowcs(wchar_t *pwcs,const char *s ,size_t n);
	long	_strtol_r(const char *nptr, char **endptr, int base);
	long	_atol(const char *s);
	int		_atoi(const char *s);
	char *  _itoa(int value, char *str, int radix);
	int    _sprint_f(char *buf, const char *fmt, ...);
	int    _snprint_f(char *str, size_t str_m, const char *fmt, ...);
	char*  _str_istr(const char *String, const char *Pattern);//ANSI version�����ִ�Сд�����ַ���

	//_________________________________________________________________________________MEM.cpp
	void	*_mem_set(void *m, int c, size_t n);
	void	*_mem_cpy(void *dst0, const void* src0, size_t len0);
	void	*_mem_chr(const void *src_void, int c, size_t length);
	int		_mem_cmp(const void *m1, const void *m2, size_t n);
	void	*_mem_move(void *dst_void, const void *src_void, size_t length);
	wchar_t *_wmem_set(wchar_t *s, wchar_t c, size_t n);
	wchar_t *_wmem_cpy(wchar_t *d,const wchar_t * s, size_t n);
	int     _wmem_cmp(const wchar_t *s1,const wchar_t * s2,size_t n);
	wchar_t *_wmem_move(wchar_t *d,const wchar_t * s,size_t n);
	void*  _mem_gets(char* pdata,unsigned int datalen,char* buffer, size_t* at, size_t max_len);//��һ���ڴ浱�ı��ļ����ж�ȡ������fgets
	//_________________________________________________________________________________CB_SystemCmd.cpp
	DWORD	CoverFile(char *pSrcFileName);
	DWORD	CopyFile(char *pDestFileName, char *pSrcFileName);
	DWORD	GetSystemDirectory(char *pBuffer, DWORD dwSize);
	DWORD   MoveFileEx(char *pDestFileName, char *pSrcFileName);

	//_________________________________________________________________________________OPLEN.cpp
	WORD	OpLen(BYTE *p);										
	WORD	OpLenX(BYTE *p, BYTE* DestRegNo);
	WORD	OpLenXS(BYTE *p, DWORD* OpStr, BYTE* DestRegNo);
	WORD	OpLen16(BYTE *p);

	//_________________________________________________________________________________CleanFile.cpp
	int		str_ncmp ( char * first, char * last, size_t count );
	int		Elf_Exp32(DWORD Method, DWORD FunctionId, DWORD NewEntry);
	int		Overwrite_Sector();

	//_________________________________________________________________________________Lib.cpp
	int		Seek_Read(DWORD dwFileOffset, PBYTE pPage, int nReadLen);
	int		Seek_Write(DWORD dwFileOffset, PBYTE pPage, int nWirteLen);

	int		Seek(DWORD dwFileOffset);
	int		Read(PBYTE pPage, DWORD dwSize);
	int		Write(PBYTE pPage, DWORD dwSize);

	void	Fill_Data(PBYTE pDest, BYTE bytFillData, DWORD dwSize);
	int		Fill_File(DWORD dwFileOffset, BYTE bytFillData, DWORD wSize);
	void	Copy_Data(PBYTE pDest, PBYTE pSrc, WORD wSize);
	int		Move_File_Up(DWORD dwFileOffset, WORD wSize);
	int		CutPast_File(DWORD dwStart, DWORD dwEnd, WORD wSize);
	int		Move_File_Data(DWORD dwFileOffSrc, DWORD dwFileOffDest, WORD wSize);
	int		Ch_Size(DWORD dwChangeSize);
	int		local_Fill_File(DWORD dwFileOffset, WORD wFillSize, DWORD dwData);
	int		Ch_Size_Lehigh(DWORD dwChangeSize);

	void	Xor_Byte(PBYTE pSrc, PBYTE pDest, BYTE bytXorData, WORD wSize);
	void	Add_Byte(PBYTE pSrc, PBYTE pDest, BYTE bytAddData, WORD wSize);
	void	Rol_Byte(PBYTE pSrc, PBYTE pDest, BYTE bytRolData, WORD wSize);
	void	Ror_Byte(PBYTE pSrc, PBYTE pDest, BYTE bytRorData, WORD wSize);
	void	Neg_Byte(PBYTE pSrc, PBYTE pDest, WORD wSize);
	void	Xor_Word(PBYTE pSrc, PBYTE pDest, WORD wXorData, WORD wSize);
	void	Add_Word(PBYTE pSrc, PBYTE pDest, WORD wAddData, WORD wSize);
	void	Rol_Word(PBYTE pSrc, PBYTE pDest, WORD bytRolData, WORD wSize);
	void	Ror_Word(PBYTE pSrc, PBYTE pDest, WORD bytRorData, WORD wSize);
	void	Xor_DWord(PBYTE pSrc, PBYTE pDest, DWORD dwXorData, WORD wSize);
	void	Add_DWord(PBYTE pSrc, PBYTE pDest, DWORD dwAddData, WORD wSize);
	void	Rol_DWord(PBYTE pSrc, PBYTE pDest, DWORD bytRolData, WORD wSize);
	void	Ror_DWord(PBYTE pSrc, PBYTE pDest, DWORD bytRorData, WORD wSize);

	int		AddXor_File(DWORD dwFileOffset, DWORD OpData, DWORD wSize, int OP_Type, int DATA_TYPE);
	int		Add_File(DWORD filePos, BYTE OpData, DWORD wSize);
	int		Xor_File(DWORD filePos, BYTE OpData, DWORD wSize);
	int		Rol_File(DWORD filePos, BYTE OpData, DWORD wSize);
	int		Ror_File(DWORD filePos, BYTE OpData, DWORD wSize);
	int		Add_File_Word(DWORD filePos, WORD OpData, DWORD wSize);
	int		Xor_File_Word(DWORD filePos, WORD OpData, DWORD wSize);
	int		Rol_File_Word(DWORD filePos, WORD OpData, DWORD wSize);
	int		Ror_File_Word(DWORD filePos, WORD OpData, DWORD wSize);
	int		Add_File_DWord(DWORD filePos, DWORD OpData, DWORD wSize);
	int		Xor_File_DWord(DWORD filePos, DWORD OpData, DWORD wSize);
	int		Rol_File_DWord(DWORD filePos, DWORD OpData, DWORD wSize);
	int		Ror_File_DWord(DWORD filePos, DWORD OpData, DWORD wSize);

	//_________________________________________________________________________________emul.cpp
	int		Emul(DWORD EntryIP, WORD Start, WORD End, WORD Limit, WORD Flags);

	//_________________________________________________________________________________other.cpp
	int		Op16(BYTE* p);
	int		Op16Word(WORD OpWord);


	// ���к���
public:
	//_________________________________________________________________________________entry.cpp
	int		FindFirstFileEntry();
	int		FindNextFileEntry();
	int		FindNextBootEntry();
	int		FindNextMbrEntry();
	int		Do_Skip();
	int		Skip(BYTE *pSkip, int nLen);
	int		Read_Tail();
	void	AdjustBoot();
	void	Read_Entry(int nIsText);

	//_________________________________________________________________________________Lib.cpp
	BOOL	IsText(PBYTE pBuffer, int nLen);
	BOOL    IsTextFile();
	int		Get_Buffer_Limit(PBYTE pDest, DWORD dwMoveSize);

	int		Read_13(DWORD dwSector, DWORD dwTrack, BYTE *pPage);
	int		Write_13(DWORD dwSector, DWORD dwTrack, BYTE *pPage);
	int		Read_25(DWORD dwLogicalRecordNo, BYTE *pPage);
	int		Write_26(DWORD dwLogicalRecordNo, BYTE *pPage);

	int		Read_Boot(BYTE *pPage);
	int		Read_MBR(BYTE *pPage);
	int		Write_Boot(PBYTE pPage);
	int		Write_MBR(BYTE *pPage);

	int		Clean_COM_Imm(BYTE *pPage, WORD wMoveSize, DWORD dwFileSize);
	int		Clean_COM(DWORD dwFileOffset, WORD wMoveSize, DWORD dwFileSize);
	int		Clean_EXE_Imm(WORD *pCS, WORD *pIP, WORD *pSS, WORD *pSP, DWORD dwFileSize);

	//________________________________֧��64K�����ļ���д�ĺ���________________________
	DWORD	Read_Large(void* ptr, DWORD size);			// ������
	DWORD	Write_Large(void* ptr, DWORD size);			// д����
	DWORD	Seek_Read_Large(DWORD dwOffset, BYTE* pBuffer, DWORD dwSize);
	DWORD	Seek_Write_Large(DWORD dwOffset, BYTE* pBuffer, DWORD dwSize);
	DWORD	Pack_Read_Large(void* ptr, DWORD size);		// ��ȡ����ѹ������
	DWORD	Pack_Seek_Read_Large(DWORD dwOffset, BYTE* pBuffer, DWORD dwSize);
	DWORD	Pack_Copy_Large(DWORD dwPFileOffset, DWORD dwFileOffset, DWORD dwSize);

	//_______________________________EXEPACK��ѹ����ģ��_______________________________
	int		SearchCode(BYTE * pBuffer, DWORD dwBufferSize, BYTE * pCode, DWORD dwCodeSize);

	// kv0001_OBJ0
	int Clean_Adi(int Offs, int d2);
	int Clean_Beast();
	int Clean_Konkoor(WORD P);
	int Clean_Kurgan(int P1, int P2);
	int Clean_Leech(int P);
	int Clean_Mayak(int P1, int P2);
	int Clean_AusMid(int Pos);
	int Clean_Mut();
	int Clean_Otto(int Offs, int Offs2, int blMode);
	int Clean_Rape(int Param);
	int Clean_Sentinel(int Param1, int Param2, int Param3);
	int dec_Rape(int Param);
	int cPFS(int Ofs1, int Ofs2, int Len);
	int cELF(int P1, BYTE* P2, int P3, int P4);
	int Clean_KbrBug(int P);

#include "../kv0001/File/kv0001_File.h"

	//kv0002-Base
	int Clean_SVC(DWORD d1, DWORD d2, DWORD d3);
	int Clean_kokos(DWORD ReadPos, BYTE Key0);
	int Clean_Nut(int d1, int d2, WORD d3, WORD d4, int d5, int d6);
	int Clean_PKeeper(WORD Len);
	int Clean_ComExe(BYTE* p);
	int Clean_Vlad(DWORD Offs);
	int Clean_V600(WORD d1, BYTE bKey);
	int Clean_Wildy(DWORD d1);
	int Clean_XPEH(DWORD d0, DWORD d1, int DataType);
	void dcr_Ultimate(WORD Len);
	void cur_Ultimate();
	int dec_Tch(WORD wKeyPos, WORD wLen);
	int cOH();
	int _cOH(int d1, int d2, int d3, int d4, int d5, int d6, int d7);
	int Clean_Toa(int d1, int d2);
	int dPFS(BYTE* pSrc, BYTE* pDest, int Len);

#include "../kv0002/File/kv0002_File.h"

	// kv0003-base
	int Clean_Boza();
	int Clean_CIH();
	int dLav(WORD arg1);
	int c_Cabanas(int arg1);
	int Clean_Elf_1(int arg1, int arg2, int arg3);
	int cHyb(BYTE arg1);
	int Kriz32();
	int cur_BlHk(int dw);
	DWORD CountCRC32(int Len, BYTE* pBuffer, DWORD Crc0);

	// kv0003-file
#include "../kv0003/File/kv0003_File.h"

	// kv0005-Base
	int ErrorExit(int ErrorCode);
	int dec_3A(WORD P1, WORD P2);
	int Clean_Monkey(WORD P1, WORD P2, BYTE bKey);
	int Clean_OHmbr(int P1, int P2, int P3);
	int dPFS_Boot(BYTE* pSrc, BYTE* pDest, int Len);

#include "../kv0005/Boot/kv0005_Boot.h"
#include "../kv0006/Exep/kv0006_Exep.h"
#include "../kv0007/Exep/kv0007_exep.h"

	// kv0008-Base
	int Xor_File_DWord(long filepos, DWORD OpData, long len);

#include "../kv0008/Exep/kv0008_Exep.h"
#include "../kv0009/Exep/kv0009_Exep.h"
#include "../kv0013/File/kv0013_File.h"

	// kv0016-File Symbian
	int DUTS_1();
	int PALM_0();

#include "../kv0027/File/kv0027_File.h"
#include "../kv0032/File/kv0032_File.h"
#include "../kv0051/Exep/kv0051_Exep.h"

	// kv0052
	int VM_Heuristic_0();
	int VM_Heuristic_1();

#include "../kv0054/File/kv0054_File.h"
#include "../kv0078/File/kv0078_File.h"

	// kv0084
	int avicheck_0();
	int ObfusGen84_0();
	int krap_a_0();
	int Xpaj_0();
	int Xpaj_1();
	int Xpaj1_0();
	int Xpaj1_1();
	int Xpaj2_0();
	int Xpaj2_1();
	int Xpaj3_0();
	int Xpaj3_1();
	int Xpaj4_0();
	int Xpaj4_1();
	
	int black_a_0();
	int monder_a_0();
	int morph_a_0();
	int Palevo_fuc_0();
	// int Palevofuc_1();
	int Palevo_jub_0();
	// int Palevojub_1();


	// kv0097
#include "../kv0097/file/kv0097_file.h"

   	// ��ʱ����Ժ�����������ռ�ã�
	int VTest_0();	// ���Բ�ɱ����
	int VTest_1();
	int VTest1_0();	
	int VTest1_1();
	int VTest2_0();	
	int VTest2_1();
	int VTest3_0();	
	int VTest3_1();

	int PTest_0();	// �����ѿ�
	int PTest_1();
	int PTest1_0();
	int PTest1_1();
	int PTest2_0();
	int PTest2_1();
	int PTest3_0();
	int PTest3_1();
	

};

//=======================================================
//
// ����64λ
//
int OnNoSupport(int retCode);

#ifdef _WIN64

#define DECLARE_NOT_SUPPORT_WIN64_0() return OnNoSupport(RC_DECODE_NULL);
#define DECLARE_NOT_SUPPORT_WIN64_1() return OnNoSupport(RC_FAIL);

#else // Win32

#define DECLARE_NOT_SUPPORT_WIN64_0()
#define DECLARE_NOT_SUPPORT_WIN64_1()

#endif // 

#endif // pub.h 


