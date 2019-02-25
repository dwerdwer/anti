/****************************************************
 *
 *  病毒库中引用的公共常量，在此登记
 *
 *  整理：何公道
 *  日期：2003.12.9
 *
 ****************************************************/

#ifndef VL_CONST_H
#define VL_CONST_H

//decode和clean等程序返回值
#define RC_DECODE_NULL		0		// 不做处理，例如vm16模块，有可能改了缓冲
#define RC_DECODE_OK		1		// 解码成功
#define RC_DECODE_NEXT		2		// 解码继续，如果第二校验相同，则调用clean
#define RC_CLEAN_OK			3		// 解毒成功
#define RC_CLEAN_NEXT		4		// 解毒继续，如果不是宏(0x1d)，则调用方法CleanFile
#define	RC_FAIL				5	    // 发现错误
#define RC_DELETE			6	    // 删除（扇区覆盖）
#define RC_WARNING			7	    // 上报病毒样本

#define RC_UNP_OK			0x10	// 脱壳解压成功
#define RC_UNP_FAIL			0x11	// 脱壳解压失败
#define RC_UNP_UNKNOWN		0x12	// 脱壳解压不知道其类型unknown

// 扫描对象
#define OT_FILE			0			// 文件
#define OT_DISK			0x10		// 磁盘类，只作位验证标志
#define OT_MBR			0x11		// 主引导区
#define OT_BOOT			0x12		// 逻辑盘引导区
#define OT_MEMORY		0x20		// 内存类，只作位验证标志
#define OT_MEMBLOCK		0x21		// 内存块
#define OT_IMAGE		0x22		// 内存映像

//运行中的错误标志代表的意义，保存在RetFlags变量中
#define RF_CORRUPTED	      0x20	// 一般性错误？
#define RF_GENERIC			  0x40	// 一般性结束
#define RF_ENTRYPT		     0x400	// 加密文件？
#define RF_PASSWORD		     0x800  // 文件有密码保护，不能打开
#define RF_NO_DISKSPACE     0x4000  // 无磁盘空间
#define RF_NO_MOMORY	   0x20000	// 无内存
#define RF_RDONLY		0x80000000	// 文件不能写，可能是只读？


// 文件类型，包括磁盘
// CEngData::File_Type
#define	ST_UNKNOW		0		// 所有未定义类型的文件
#define	ST_DOSCOM		1
#define	ST_DOSEXE		2
#define	ST_DOSEXE_HL	3
#define	ST_DOSSYS		4
#define	ST_NE			5
#define	ST_HLP			6
#define	ST_LE			7
#define	ST_LX			8
#define	ST_PE			9		// (Intel)
#define	ST_PE_HL		0xA		// (Intel)
#define	ST_ELF			0xB
#define	ST_DOC			0xC
#define	ST_JAVA     	0xD
#define	ST_SCRIPT		0xE
#define ST_DOTNET		0xF		// 微软.NET
#define ST_DOTNET_HL	0x10
#define ST_EPOC			0x11	// EPOC,Symbian OS
#define ST_EPOC_HL		0x12		
#define ST_OSX			0x13	// Mach-O, Apple OSX
#define ST_OSX_HL		0x14
#define ST_OSX64		0x15	// Mach-O, Apple OSX, 64bit
#define ST_OSX64_HL		0x16
#define ST_EYUYAN		0x17	// 易语言
#define ST_VB_STRING	0x18    // 增加 vb 文件的处理。
#define ST_DEX          0x19    //Android Dex

#define ST_ZIP			0xFA	// 非独立类型
#define ST_BINARY		0xFB	// 非独立类型
#define ST_TEXT			0xFC	// 非独立类型


//-----------------------
// CEngData::wFileType
#define ST2_UKNOWN		0		// 未知类型

// 文档类
#define	ST2_DOC			0x20	// word/excel  复合文档
#define	ST2_DOCX		0x21	// XML
#define ST2_LNK			0x22	// 仅通过扩展名
#define ST2_PDF			0x23	// 
#define	ST2_HLP			0x24	// .HLP/.CHM
#define	ST2_DB			0x25	// 
#define ST2_MEDIA	    0x26    // Image
#define	ST2_SCRIPT		0x27	// VB /JS / Shell

// DOS
#define	ST2_COM			0x30
#define	ST2_DOSSYS		0x31
#define	ST2_DOSEXE		0x32

// Win16
#define	ST2_NE			0x36
#define	ST2_LE			0x37
#define	ST2_LX			0x38

// Win32
#define	ST2_PE			0x40
#define ST2_C			0x41
#define ST2_DOTNET		0x42	// 微软.NET
#define ST2_VBEXE		0x43    // 增加 vb 文件的处理
#define ST2_PYTHONEXE	0x44
#define ST2_EYUYAN		0x45	// 易语言
//#define ST2_QT			0x46
//#define ST2_CYGWIN		0x47
//#define ST2_CBUILDER	0x48
#define ST2_DELPHI		0x49

// Win64
#define ST2_PE64		0x50
#define ST2_C64			0x51
#define ST2_DOTNET64	0x52	// 微软.NET
#define ST2_VBEXE64		0x53    // 增加 vb 文件的处理
#define ST2_PYTHONEXE64	0x54
#define ST2_EYUYAN64	0x55	// 易语言
//#define ST2_QT64		0x56
//#define ST2_CYGWIN64	0x57
//#define ST2_CBUILDER64	0x58
#define ST2_DELPHI64	0x59

// Linux 32
#define	ST2_ELF			0x60	// Linux elf 程序
// Linux 64
#define ST2_ELF64		0x61

// Mac/IOS
#define ST2_OSX			0x70	// Mach-O, Apple OSX 程序
#define ST2_OSX64		0x71	// Mach-O, Apple OSX, 64bit

// Sybian
#define ST2_EPOC		0x75	// EPOC,Symbian OS
#define ST2_SIS			0x76	//
#define ST2_SISX		0x77	//

// Android
#define ST2_APK			0x80
#define ST2_DEX			0x81

// Java
#define	ST2_JAVA     	0x88	// class
#define	ST2_JAR     	0x89	// jar/war


// #define ST2_SHELLCODE	0x3

#define ST2_TEXT        0x90    // 未知文本, // ??? 后续直接匹配一次记录
#define ST2_BINARY      0x91    // 未知二进制文件, 后续直接匹配一次记录
#define ST2_ZIP         0x92    // 压缩包
#define ST2_EXEPACK     0x93    // 程序加壳
#define ST2_KNOWNTEXT   0x94    // 已知文本文件,   无须查毒
#define ST2_KNOWNDATA   0x95    // 已知二进制文件, 无须查毒

#define ST_NEEDINIT		0xFF	// 需要初始化

// 在此之间的文件类型，将会检查是否进行Exep检测
#define ST2_CHECK_EXEP_BEGIN	0x30
#define ST2_CHECK_EXEP_END		0x90


//=======================================
// CEngData::wSubType
typedef enum _SUBFILETYPE
{
	ST2_SUB_UNKNOWN = 0,

	ST2_SUB_VC = 1,		// 旧版本
	ST2_SUB_VC6,
	ST2_SUB_VS2003,		// VC 7
	ST2_SUB_VS2005,		// VC 8
	ST2_SUB_VS2008,		// VC 9
	ST2_SUB_VS2010,		// VC 10
	ST2_SUB_VS2012,		// VC 11
	ST2_SUB_VS2013,		// VC 12
	ST2_SUB_VS2015,		// VC 14
	ST2_SUB_VS2017,		// VC 14.1

	ST2_SUB_GCC=20,		// GCC
	ST2_SUB_GCC_CYGWIN,	// CYGWin GCC
	ST2_SUB_LCC,
	ST2_SUB_DIGITALMARS_C,
	ST2_SUB_GCC_CODEBLOCKS,
	ST2_SUB_GCC_DEV_CPP,
	ST2_SUB_GCJ,		// Java_to_EXE

	ST2_SUB_BORLANDC=30,	// Borland C++ / C++ Builder
	ST2_SUB_CPPBUILDER = 31,// 
	ST2_SUB_QT,

	ST2_SUB_VB = 40,	// 

} SUBFILETYPE;


//磁盘类型
#define	ST_DISK10		0x10
#define	ST_DISK20		0x20
#define	ST_DISK40		0x40
#define	ST_DISK80		0x80
#define	ST_DISK81		0x81

//用Word表示的文件标志常量(File Sign in Word)
#define FSW_MZ1		0x5A4D	// MZ DOS下的文件标志
#define FSW_MZ2		0x4D5A	// ZM 同上
#define FSW_NE		0x454E	// NE Windows3.x以下的格式
#define FSW_PE		0x4550	// PE Windows9.x以上的格式
#define FSW_LE		0x454C  // LE VXD驱动程序
#define FSW_LX		0x584C	// LX SYS驱动程序
#define FSW_ELF     0x457F	// ELF Linux上的可执行文件

//用DWord表示的文件标志常量(File Sign in DWord)
#define FSDW_ELF	0x464C457F  // \x7fELF
#define FSDW_MSFT	0x5446534d	// MSFT
#define FSDW_MSCF	0x4643534d	// MSCabFile
#define FSDW_DOC1	0xe011cfd0	// Compound file
#define FSDW_DOC2	0xe11ab1a1	// 8个字节，这是第2个字节
#define FSDW_ODOC1	0x0dfc110e	// Old Compound file
#define FSDW_ODOC2	0xe011cfd0	// 8个字节，这是第2个字节
#define FSDW_JAVA	0xbebafeca	// Java class file
#define FSDW_HLP	0x35f3f		// .HLP帮助文件
#define FSDW_MDB	0x100		// Microsoft Access .MDB "StandardJet "
#define FSDW_MDB1	0x6E617453	// "Stan"
#define FSDW_MDB2	0x64726164	// "dard"
#define FSDW_MDB3	0x74654A20	// "Jet "
#define FSDW_DOSSYS 0xffffffff	// DOS .SYS 驱动程序
#define FSDW_E32DLL 0x10000079	// E32Image DLL
#define FSDW_E32EXE 0x1000007A	// E32Image EXE
#define FSDW_EPOC   0x434f5045	// E32Image Sign "EPOC",Symbian手机程序
#define FSDW_OSX	0xfeedface	// Mac OS X Mach-O 
#define FSDW_DEX    0x0a786564  //Android dex file

// By WangLei @ 2017.9.14
#define FSDW_ZIP	0x04034b50	// Zip
#define FSDW_RAR	0x21726152	// RAR

#endif //!VL_CONST_H
