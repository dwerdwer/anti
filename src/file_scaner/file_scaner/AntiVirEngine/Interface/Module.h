#pragma once

#include "TypeDef.h"

// 更新机制相关接口 根据需要引出，升级程序调用
// int32_t AV_CALLTYPE OnUpdated(  );

//////////////////////////////////////////////////////////////////////////
// 文件类型识别模块
//		
enum EFTRESULT
{
	EFT_NEEDNOT_CHECK	  = 65535,	// 是文本、媒体、数据文件，不需要继续查毒
	EFT_NEED_UNPACK_ZIP	  = 1,	// 已知Arch壳，需要脱壳。
								// wSubType 是壳类别
	EFT_NEED_UNEXEPACK	  = 2,	// 已知Exep壳，需要脱壳。
								// wSubType 是壳类别
	EFT_NEED_CHECK_UNPACK = 3,	// 需要后续识别加壳
	EFT_NEED_CHECK_FILE   = 4,	// 已识别编译器/构造，需要查毒，
								// wFileType/ wSubType 是后续查毒特征库类别

	EFT_UNKNOWN_PROG	  = 10,	// 不能识别的程序，不属于已知分类
								// 可能是变形病毒、未知壳、被感染
								// 查毒时后续将使用应急查毒特征库处理，
								// 特征提取时后续需要人工处理
								// wFileType 是初步分类，如PE/NE等
};

#define ST2_ZIP         0x92    // 压缩包

// 在此之间的文件类型，将会检查是否进行Exep检测
#define ST2_CHECK_EXEP_BEGIN	0x30
#define ST2_CHECK_EXEP_END		0x90

/*
// 文件主类定义，查毒将根据此值使用对应的特征库
enum FileMainType {
	FTYPE_ZIP		= 1,		// 压缩包
	FTYPE_PKEXE		= 2,		// 可执行程序加壳
	FTYPE_APK		= 3,		// Android 包，单独处理
	FTYPE_IOSAPP	= 4,		// MacOS/IOS app 包，单独处理
	FTYPE_DISK		= 5,		// 磁盘、磁盘引导区文件
	FTYPE_MACRODOC  = 6,		// Office 文档，docx，xlsx，处理宏病毒
	FTYPE_SCRIPT	= 7,		// 脚本文件，包括shell脚本、Bat批命令、vbs、java_script等，将会查毒
	FTYPE_KNOWNIMAGE= 8,		// 可能包含病毒的Media文件，将查毒
	FTYPE_KNOWNDAT  = 9,		// 可能包含病毒的数据文件，将查毒
	FTYPE_DOSCOM	= 10,		// .COM, DOS 程序
	FTYPE_DOSMZ		= 11,		// DOS EXE
	FTYPE_NE		= 12,		// Win16 NE, 包括 LX
	FTYPE_PE		= 13,		// 标准Windows程序，包括已知和不能识别的子类
	FTYPE_ELF		= 14,		// 标准Linux程序，  包括已知和不能识别的程序子类

	FTYPE_LIB		= 20,		// 用于连接的动态lib或静态库，将不查毒
	FTYPE_TXT		= 21,		// 文本文件，将不查毒
	FTYPE_BIN		= 22,		// 已知二进制文件，将不查毒
	FTYPE_MEDIA		= 23,		// 已知Media，将不查毒
	FTYPE_UNKNOWN   = 24,		// 未知文件，但不像是com，将不查毒
};

// 文件子类定义，每个子类均属于一个主类，每个主类的子类从1开始定义, 0 表示未知
// 不需要查毒调度程序关注的主类可不定义，内部使用
enum FileSubType {
	// 类别： FTYPE_ZIP,		// 压缩包
	FSUB_7Z			= 1,		// 7Z 库可以识别的类型
	FSUB_CAB		= 2,
	FSUB_SIS		= 3,		// Symbin 程序包
	FSUB_SISX		= 4,		// Symbin 程序包 
	// ...


	// 类别： FTYPE_PKEXE,		// 可执行程序加壳
	// ...

	// 类别： FTYPE_APK,		// Android 包，单独处理

	// 类别： FTYPE_IOSAPP,		// MacOS/IOS app 包，单独处理
	FSUB_IPA		= 1,		// IOS ipa
	FSUB_MACAPP		= 2,		// MacOS app

	// 类别： FTYPE_DISK,		// 磁盘、磁盘引导区文件
	FSUB_MBRFILE	= 1,
	FSUB_BOOTFILE	= 2,
	FSUB_MBRDISK	= 3,		// 物理磁盘引导区
	FSUB_BOOTDISK   = 4,		// 逻辑磁盘引导区

	// 类别： FTYPE_MACRODOC,		// Office 文档，docx，xlsx，处理宏病毒

	// 类别： FTYPE_SCRIPT,		// 脚本文件，包括shell脚本、Bat批命令、vbs、java_script等，将会查毒
	FSUB_BAT		= 1,		// ;.BAT;.CMD;
	FSUB_PY			= 2,		// ;.PY;.PYW
	FSUB_VBS		= 3,		// ;.VBS;.VBE;WSF;.WSH;.MSC
	FSUB_JS			= 4,		// ;.JS;.JSE;.WSF;.WSH;.MSC
	FSUB_SHELL		= 5,		// Linux shell

	// 类别： FTYPE_NE,			//
	FSUB_NE			= 1,
	FSUB_LX			= 2,

	// 类别： FTYPE_PE,			// 生成器
	FSUB_MSVC		= 1,
	FSUB_MSVB		= 2,
	FSUB_DOTNET		= 3,
	FSUB_CSHARP		= 4,

	FSUB_BORLANDC	= 4,

	FSUB_YI			= 10,		// 易语言
	FSUB_PY2EXE		= 11,		// PY 
	FSUB_PYEASY		= 12,		// PY 


	// 类别： FTYPE_ELF,		// 标准Linux程序，  包括已知和不能识别的程序子类
	// 类别： FTYPE_KNOWNIMAGE,		// 可能包含病毒的Media文件，将查毒
	// 类别： FTYPE_KNOWNDAT,		// 可能包含病毒的Media文件，将查毒

	// 类别： FTYPE_TXT,		// 文本文件，将不查毒
	// 类别： FTYPE_BIN,		// 已知二进制文件，将不查毒
	// 类别： FTYPE_MEDIA,		// 已知Media，将不查毒
};
*/

/*
#define CPU_X16				0x01
#define CPU_X86				0x02
#define CPU_X64				0x03
#define CPU_ARM16			0x10
#define CPU_ARM32			0x11
#define CPU_ARM64			0x12
#define CPU_MIPS			0x20

#define CPU_MASK			0x00FF		// CPU 架构掩码

#define CPU_LITTLE_ENDIAN	0x0000
#define CPU_BIG_ENDIAN		0x8000

#define CPU_ENDIAN_MASK		0x8000		// CPU 大小端模式掩码
*/

struct CEngData;

#include "PshPack2.h"
typedef struct _FTRESULT
{
	uint16_t wFileType;		// 主文件类型定义，查毒将根据此值使用对应的特征库
	uint16_t wSubType;		// 子类型
	uint16_t wArchFlag;		// CPU 架构、代码模式
	uint16_t wResult;		// 识别结果，据此进行后续处理
	uint32_t dwOffs;		// 特征区域文件绝对偏移
	uint32_t dwLen;			// 特征区域长度
	uint32_t dwRecNo;		// 匹配记录号，用于分析调试
	uint32_t dwRev;			// 保留
} FTRESULT;
#include "PopPack.h"

#define MIN_FILE_SIZE		6	// 文件最小大小

/** 
 * @param szDataFolder 识别特征数据文件所在文件夹的路径，文件夹以 PATH_SPLIT_CHAR 结尾 
 * @retval 0 成功，其他值表示失败
 */
int32_t AV_CALLTYPE FT_Init( IN const char* szDataFolder);
void    AV_CALLTYPE FT_Clear();

/*
  * 获取扫描缓冲区，保存待扫描文件对象
  * 初始化扫描基本数据，读取文件大小以及文件头到缓存区
  * @param pFile 需要扫描的文件
  * @retval NULL，失败，原因是文件过小或过大，不需要扫描
			非NULL 成功
*/
CEngData* AV_CALLTYPE FT_GetScanPool(IN IFileEx* pFile);

// 释放扫描缓冲区
void      AV_CALLTYPE FT_ReleaseScanPool(IN CEngData*  pAVEData);

/**
 * @param result 文件的具体类型信息
 * 
 * @retval 0 成功，其他值表示失败
 */
int32_t AV_CALLTYPE FT_RecongizeFirst(IN OUT CEngData*  pAVEData, OUT FTRESULT* result);

/**
* @param result 文件的具体类型信息
*
* @retval 0 成功，其他值表示失败，不需要继续调用FT_RecongizeNext
*/
int32_t AV_CALLTYPE FT_RecongizeNext(IN OUT CEngData*  pAVEData, OUT FTRESULT* result);


//////////////////////////////////////////////////////////////////////////
// 文件指纹管理模块
class IFinger;

/*
typedef IFinger* (AV_CALLTYPE *Func_CreateFinger) ();

struct FileFinger {
	uint32_t unSize;
	FileMainType mainType;
	uint32_t LastScanVersion;	// 上次扫描的特征库版本
};

class IFinger {
protected:
	virtual ~IFinger() {		
	}
public:
	virtual void Dispose() PURE;
	virtual bool ReadFigner(wchar_t szPath, FileFinger& finger) PURE;
};
*/

//////////////////////////////////////////////////////////////////////////
// 压缩包管理模块（包括自解压包，自解压包根据具体情况判断是否按EXE或压缩包处理）
class IArchiveCallback
{
protected:
	~IArchiveCallback() = default;
public:
	/**
	 *	用于压缩包处理模块获取一个解压用的临时文件，由 AVEng 实现一个临时文件
	 *	对象，避免每个压缩包模块实现一个临时文件接口
	 *	
	 * @param szRelativePath 压缩包内的相对路径，可以为 nullptr 
	 * @param szExt 压缩包内文件的可能扩展名，可以为 nullptr
	 */
	virtual IFileEx* AV_CALLTYPE GetTempFile( IN const wchar_t* szRelativePath, IN const wchar_t* szExt ) PURE;
	enum EAction
	{
		actionKeep,
		actionModify, // 无法打回压缩包的话则删除该项
		actionDelete,
	};
	virtual EAction AV_CALLTYPE ScanItem(IN IFileEx* pItem, OUT wchar_t szVirus[MAX_VIRNAME_LEN]) PURE;
	enum EActionResult
	{
		modifyOk = 0x10,		// 可以更新为清除后的内容
		modifyFailed = 0x11,	// 压缩包不支持更新
		deleteOk = 0x20,		// 可以删除
		deleteFailed = 0x21,	// 压缩包不支持删除
		deleteFull = 0x28,		// 需要删除整个压缩包
	};
	virtual void AV_CALLTYPE ActionResult( IN IFileEx* pFile, IN EActionResult result ) PURE;
	virtual bool AV_CALLTYPE WantAbort() PURE;
};

class IArchive {
protected:
	virtual ~IArchive() {
		
	}
public:
	virtual void AV_CALLTYPE Dispose() PURE;

	/** 
	 是否支持包内更新？
	 */
	virtual bool AV_CALLTYPE SupportUpdate() PURE;
	/**
	 * @retval 压缩包中的文件数量，不确定时返回 -1
	 */
	virtual int32_t AV_CALLTYPE ItemCount() PURE;
	virtual void AV_CALLTYPE ExtractAll(IN IArchiveCallback * pCallback) PURE;
	/**
	 在支持包内更新并且有文件删除或清除时调用
	@param szBackupId 备份的具体标识
	 */
	virtual bool AV_CALLTYPE Update( const wchar_t* szBackupId ) PURE;
};

int32_t AV_CALLTYPE ZIP_Init(IN const char* szDataFolder);
void AV_CALLTYPE ZIP_Clear();
IArchive* AV_CALLTYPE ZIP_Open(IFileEx* pFile, uint16_t usSubType );

//////////////////////////////////////////////////////////////////////////
// 脱壳模块
// 在脱壳后，如果后续处理过程文件有改变，则调用者用处理后的文件直接覆盖原始文件

int32_t AV_CALLTYPE PKEXE_Init(IN const char* szDataFolder);
void    AV_CALLTYPE PKEXE_Clear();
const char* AV_CALLTYPE PKEXE_GetName(int index);	// 调试用，获取 Exep Name


// 脱壳
// 返回：
//		< 0, 脱壳失败，返回 -ID
//		= 0, 错误
//		> 0, 脱壳成功，返回 ID
int  AV_CALLTYPE PKEXE_Unpack(IN OUT CEngData*  pAVEData, OUT IFileEx* pTarget,IN  FTRESULT* ftInfo = nullptr);

//////////////////////////////////////////////////////////////////////////
// 扫描模块
//		包括：AVScanA AVScanEP AVScanT AVScanM AVScanD AVScanS 旧版引擎调用模块 AVCloud
// 所有扫描模块名均以 AVScan 开头
// TODO : 修改成员函数的传参方式

class IScan { 
protected:
	virtual ~IScan() = default;
public:
	enum EHandleMode {
		CanCure  = 0x01, // 能够清除
		NeedRepair = 0x02, // 需要调用清理模块
		NeedReport = 0x10, // 需要上报样本
	};
	// 处理次序：上报 -->清除/删除-->修复
	virtual void AV_CALLTYPE Dispose() PURE;

	/**
	 * 读取某种类型的特征数
	 * 
	 * @param unSubFileType 文件子类型
	 * @retval 特征总数
	 */
	virtual uint32_t AV_CALLTYPE ReadRecTotal( uint16_t unSubFileType ) PURE;
	/** 
	 * 扫描模块需要首先对文件类型进行检查，对于自己不支持的文件类型直接返回 (uint32_t)-1 
	 * 
	 * @param pAVEData 要扫描的数据信息，调用识别模块的 FT_GetScanPool 生成
	 * @param pFileType 由识别模块返回的文件类型
	 * @param unLastRecNo 最近扫描记录号， 
	 * @param pMode 指向具体的处理方式，为 EHandleMode 定制的值
	 * @retval 匹配到的记录号，不匹配返回 (uint32_t)-1
	 */
	virtual uint32_t AV_CALLTYPE Scan(IN OUT CEngData*  pAVEData,
		FTRESULT* pFileType, uint32_t unLastRecNo, OUT uint32_t* pMode) PURE;

	/**
	 * 执行清除操作
	 * @param pAVEData 要扫描的数据信息，调用识别模块的 FT_GetScanPool 生成
	 * @param unSubFileType 文件子类型
	 * @param unRecNo 扫描时匹配的记录号
	 */
	virtual int32_t AV_CALLTYPE Cure(IN OUT CEngData*  pAVEData,
		uint16_t unSubFileType, uint32_t unRecNo) PURE;
};

IScan* AV_CALLTYPE  CreateScan();

int32_t AV_CALLTYPE SCAN_Init(IN const char* szDataFolder);
void    AV_CALLTYPE SCAN_Clear();

// for Debug 扫描文件
//		< 0, 错误
//		= 0, 未匹配
//		> 0, 匹配成功，返回 ID
int  AV_CALLTYPE SCAN_File(CEngData *pAVEData, int blCure = 0);


// 清理模块
// 
int32_t AV_CALLTYPE FIX_Init( const char* szDataFolder );
// pFile指向的文件可能已在杀毒时删除
int32_t AV_CALLTYPE FIX_Repair( IFileEx* pFile, uint32_t unRecNo );
int32_t AV_CALLTYPE FIX_Clear();
