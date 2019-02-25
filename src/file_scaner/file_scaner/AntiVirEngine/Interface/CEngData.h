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

	BYTE		B_Misc2[0x8000];		// By Lawrence

	BYTE		B_Section[0x1000];		// PE/NE/ELF 段表	// Add @2017.9.21
	BYTE		bSectionCount;			// 段表项数			// Add @2017.9.21


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

extern "C" void Check_Error(CEngData* pAVE);

#pragma pack(pop)
