/*******************************************************************************
*                               类定义                                         *
********************************************************************************
*                                                                              *
*   Name : AVE类定义            Origian Place : BJ of China                    *
*   Create Data : 04/29/2005    Now Version :   1.0                            *
*   Modify Time :               Author:         Hegongdao & Wangwei            *
*                                                                              *
*==============================================================================*
*                           Modification History                               *
*==============================================================================*
*                     V1.0  1. Create this program.                            *
*                           2. 06/03/2005 全部C++化，去全局变量                *
*                           3. 06/28/2005 根据新流程全面调整                   *
*                           4. 12/15/2005 加宏调整确定结构                     *
*                           5. 03/02/2006 移植CE略微调整结构                   *
*                           6. 04/10/2006 去头文件、去函数、去变量             *
*                           7. 04/14/2006 库内部外部变量细化                   *
*                           8. 06/07/2006 更新虚拟机                           *
*                           9. 07/17/2006 更新宏模块                           *
*                           10.08/05/2006 更新脚本模块                         *
*                           11.08/08/2006 修改头文件                           *
*                           12.10/27/2006 改宏为类                             *
*                           13.08/24/2007 unpack等                             *
*                           14.08/05/2008 分为类移植                           *
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

// 扫描数据区声明
// 类说明：
// 1.变量加*号标志需要扫描类初试化，否则为只读变量
struct CEngData
{
	DWORD		mSize;					// 结构大小
	WORD		Object_Type;			// 扫描对象类型，0/0x11/0x12=文件/主引导区/引导区
	WORD		File_Type;				// 文件类型 - 传统类型

										// 文件识别
	WORD		wFileType;				// 新主文件类型定义，查毒将根据此值使用对应的特征库
	WORD		wSubType;				// 子类型
	WORD		wCPUType;				// CPU 架构		// CPUTYPE
	WORD		wImageType;				// 文件特性		// IMAGETYPE
	WORD		wSubSystem;				// 子系统，1=Native，2=GUI，3=CUI, 5=OS2/CUI, 7=POSIX/CUI, 9=WINCE/CUI,16=IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION
	DWORD		dwFeatureOffs;			// 特征区域文件绝对偏移
	DWORD		dwFeatureLen;			// 特征区域长度
	DWORD		dwInfoFlags;			// 信息标志, INFO_XXX

	KVFILE		hScanFile;				// 文件句柄
	KVFILE		hTempP;					// 临时文件句柄，解压文件
	DWORD		File_Length;			// *文件长度
	DWORD		Part_Length;			// *文件片段长度 - 使用片段查毒，<= File_Length
	DWORD		ArchOffs;				// 文件压缩包偏移  - 根据文件结构计算得到的文件大小
										// File_Length>=ArchOffs, 说明文件尾部有额外数据，可能是签名、Zip包或其它数据
	DWORD		File_Data_Size;			// 计算出的文件大小
	DWORD		RetFlags;				// 扫描返回标志

										// 文件变量
	char		Full_Name[0x500];		// *文件全名程
	char		Zip_Name[0x500];		// *文件解压出的文件名称
	char		*Name;					// *文件名称指针
	char		*Ext;					// *文件后缀指针 

										// 扫描控制变量
	WORD		Entry_Count;			// 扫描遍历入口次数

	BYTE		Del_BreakPoint[0x2f];	// !已经废除 虚拟机断点变量

	class		CScan  *pScan;			// *扫描类指针	Scan

										// 缓冲区变量
	BYTE		B_Header[0x1000];		// 存放文件头
	BYTE		B_Entry[0x400];			// 存放文件第一入口，宏等
	BYTE		B_Next[0x400];			// 存放文件第二入口，宏过滤等
	BYTE		B_Tail[0x800];			// 存放文件尾
	BYTE		B_Misc[0x8000];			// 存放文件头
	BYTE		B_Temp[0x8000];			// 临时变量都可应用
	BYTE		B_Section[0x1000];		// PE/NE/ELF 段表	// Add @2017.9.21
	BYTE		bSectionCount;			// 段表项数			// Add @2017.9.21

	BYTE		B_Misc2[0x8000];		// By Lawrence

										// 程序变量
	WORD		NExe_Format;			// 可执行文件类型
	DWORD		EP;						// 指向第一个入口指针，相对于文件首偏移量(phy off)
	DWORD		EP_Next;				// 指向下一个入口指针，也是相对于文件首偏移量
	DWORD		IP32_Entry;				// 指向第一个入口指针，保存变量
	DWORD		IP32_Next;				// 指向下一个入口指针，保存变量
	DWORD		Header_Offset;			// EXE头的物理偏移地址
	DWORD		Tail;					// 文件尾指针
	DWORD		Del_Exe_IP;				// !已经废除 EXE入口指针
	WORD		Del_Exe_CS;				// !已经废除 EXE段指针
	DWORD		Del_NExe_IP;			// !已经废除 NEXE段指针
	WORD		Del_NExe_CS;			// !已经废除 NEXE入口指针
	DWORD		DosCS;					// CS段值
	DWORD		Sys2Exe;				// SYS文件的入口地址
	WORD		Sect2Num;				// 文件遍历节表数量
	BYTE		Del_Prop[0x240];		// !已经废除
	BYTE		Del_cProp[0x80];		// !已经废除		
	BYTE		ScriptBuff[0x9000];		// 脚本处理、宏解码时用
	UINT_PTR	TempFunCall;			// !已经废除 旧库用


										// 磁盘变量
	BYTE		Disk;					// *磁盘号
	WORD		Disk_Max_CX;			// *磁盘最大扇区数，也就是SectorsPerTrack
	BYTE		Disk_Max_DH;			// *磁盘最大柱面号，也就是TracksPerCylinder
	BYTE		Drive[32];				// *驱动器数组
	DWORD		First_Data_Sector;		// *第一次变量扇区数据

										// 新增变量___内部传参							   
	BYTE		*pFileRecAVE;			// *File特征码
	BYTE		Sect_bytType;			// *扇区
	DWORD		Sect_dwSector;			// *扇区数
	DWORD		Sect_dwTrack;			// *磁道数
	BYTE		Sect_bytSize;			// *增量
	BYTE		*pIsProgramBuf;			// *判断程序缓冲

										// 新增变量___变量集合
	DWORD		dwByteSequence;			// 读写字节序列 0_正序 1_逆序
	class		CVirtualMachine *pVirtualMachine;	// 虚拟机变量   Emul - Lib0
	class		CMacro *pMacro;			// 宏变量	    Macro

										// AVE增加_________
	DWORD		Overlay;				// 程序覆盖部分Offset
	DWORD		OverlaySize;			// 程序覆盖部分大小
	DWORD		MP;						// main函数的文件偏移
	DWORD		IP32_Main;				// main函数的IP地址
	DWORD		dwAutoRec;				// 自动添加特征码标志

										// 保留空间________
	DWORD		IsSkipVM;				// 0－过虚拟机，否则不过虚拟机
	BYTE        *pSkipBuf;              // Skip缓冲区指针
	DWORD       dwSkipBufLen;           // SKip缓冲区长度
	class		CVM		*pVM;			// Bochs 虚拟机变量 VM - 脱壳 - Lib50
	class       CPE		*pPE;			// PE加载后主文件结构		
	class		CLE		*pLE;			// LE文件类型 		
	class		CNE		*pNE;			// NE文件类型 		
	class		CElf	*pElf;			// Elf文件类型 		
	class		CDos	*pDos;			// Dos文件类型 		
	class		CInf	*pInf;			// Inf文件类型 		
	class		CJava	*pJava;			// Java类型	   
	class		CHelp	*pHelp;			// Help类型	  

										// DataScan扫描
	DWORD		bDataScanEnable;		// 是否启用DataScan扫描（0:不启用,非0:启用）
	BYTE		*plib_buf;				// 病毒库指针，传参用
	DWORD		lib_size;				// 病毒库大小，传参用
	class		CDataScan *pDataScan;	// DataScan扫描器
	char		HeruVirName[0x40];		// 启发式病毒名称
	DWORD		HeruRet;				// 启发式病毒结果

										//SCRIPT全局变量
	DWORD		ScriptStatus;			// 脚本是否需要再次遍历
	DWORD		ScriptSign;				// 当前脚本的状态
	DWORD		IsSkipScript;			// 0－过脚本处理模块，否则不过脚本处理模块
	DWORD		IsSkipScript2;			// 同上，姜澎Script_Gen的开关
										//增加脚本扫描 DataScan
	BYTE		*plib_script_buf;				// 病毒库指针，传参用
	DWORD		lib_script_size;				// 病毒库大小，传参用
	class		CDataScan *pDataScanScript;	    // DataScan扫描器

												//增加宏扫描 DataScan
	BYTE		*plib_macro_buf;		// 病毒库指针，传参用
	DWORD		lib_macro_size;		        // 病毒库大小，传参用
	class		CDataScan *pDataScanMacro;	// DataScan扫描器

											//Android dex 文件支持
	class		CDex		*pDex;
	//保留
	//BYTE		ExternVal[0x4000-0xA0]; // 预留拓展变量
	BYTE		ExternVal[0x4000 - 0xBC]; // 预留拓展变量

};


#pragma pack(pop)


class CAntiVirEngine : public CEngData
{

public:


public:

	// 引擎接口函数，需要外部函数链接
	//_________________________________________________________________________________CB_File.cpp
	DWORD	__thiscall	_LSeek(DWORD pos, DWORD mode);
	DWORD	__thiscall	_Seek(DWORD pos);
	DWORD	__thiscall	_Read(void* ptr, DWORD size);
	DWORD	__thiscall	_Write(void* ptr, DWORD size);
	BOOL	__thiscall	_SetEndOfFile(DWORD size);
	DWORD	__thiscall	Pack_Seek(DWORD pos, DWORD mode);
	DWORD	__thiscall	Pack_Read(void* ptr, DWORD size);
	DWORD	__thiscall	Pack_Copy_File();
	DWORD	__thiscall	MemBlock_Save(void* pBuffer, DWORD dwSize); // 保存指定的数据块
	DWORD	__thiscall	MemBlock_Load(void* pBuffer);				// 恢复数据块到指定指针

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

	// 引擎引出函数，引擎调用
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


	// 脱壳解压程序在此添加
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

	// 引擎	内部函数
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
	char*  _str_istr(const char *String, const char *Pattern);//ANSI version不区分大小写查找字符串

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
	void*  _mem_gets(char* pdata,unsigned int datalen,char* buffer, size_t* at, size_t max_len);//把一段内存当文本文件按行读取，类似fgets
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


	// 共有函数
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

	//________________________________支持64K以上文件读写的函数________________________
	DWORD	Read_Large(void* ptr, DWORD size);			// 读数据
	DWORD	Write_Large(void* ptr, DWORD size);			// 写数据
	DWORD	Seek_Read_Large(DWORD dwOffset, BYTE* pBuffer, DWORD dwSize);
	DWORD	Seek_Write_Large(DWORD dwOffset, BYTE* pBuffer, DWORD dwSize);
	DWORD	Pack_Read_Large(void* ptr, DWORD size);		// 读取被解压缩数据
	DWORD	Pack_Seek_Read_Large(DWORD dwOffset, BYTE* pBuffer, DWORD dwSize);
	DWORD	Pack_Copy_Large(DWORD dwPFileOffset, DWORD dwFileOffset, DWORD dwSize);

	//_______________________________EXEPACK解压公共模块_______________________________
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

   	// 临时或测试函数，请勿长期占用！
	int VTest_0();	// 测试查杀病毒
	int VTest_1();
	int VTest1_0();	
	int VTest1_1();
	int VTest2_0();	
	int VTest2_1();
	int VTest3_0();	
	int VTest3_1();

	int PTest_0();	// 测试脱壳
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
// 兼容64位
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


