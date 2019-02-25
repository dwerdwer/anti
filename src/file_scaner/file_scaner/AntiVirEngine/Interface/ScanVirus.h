#pragma once

#include "TypeDef.h"
#include "Engine.h"

#ifndef MAX_PATH
#define MAX_PATH          260
#endif

#include <pshpack1.h>
struct ScanOptions {
	uint32_t m_dwSize; // 结构大小
	enum EFlags {
		scanUnzip = 0x01,		// 扫描压缩包
		scanUnpack = 0x02,		// 脱壳
		scanStopOnOne = 0x10,   // 处理一个病毒后即停止

		scanProgramOnly		= 0x100,	// 只扫描程序

		scanOriginalMd5		= 0x1000,	// 需要返回原始文件的 MD5值
		scanUseFigner		= 0x2000,	// 是否使用指纹，建议文件扫描、文件监控使用。网页监控、邮件监控不需要使用
		scanUseCloud		= 0x4000,	// 是否使用云引擎
		scanBackup			= 0x8000	// 文件需要前应进行备份
	};

	uint32_t m_dwFlags;		  // 扫描选项标志组合
	EHandleMode m_handeMode;  // 具体处理方式
	uint32_t m_dwMaxFileSize; // 最大扫描文件大小, 0 为不限制 
	uint32_t m_dwMaxUnzipFileSize; // 最大解压文件大小， 0为不限制
};

/**
 * 对于压缩包中包含多个病毒，返回第一个病毒名及Id
 * 感染多个病毒也是第一个病毒名及Id
 * 只对独立文件（脱壳也是原始文件）计算MD5，压缩包不计算
 */
struct ScanResult {
	uint32_t m_dwSize; // 结构大小
	enum  EFlags {
		withMd5		= 0x10,
	};
	
	char m_szVirusName[MAX_VIRNAME_LEN]; // 病毒名称
	uint8_t m_arrMd5[16];			// 病毒原始文件的 MD5 值（未杀毒前的）
	char m_szBackupId[MAX_PATH]; // 备份的文件名
	uint32_t m_dwVirusId; // 病毒记录Id
	EScanResult m_result; // 具体处理结果
};

#include <poppack.h>
/**
 * @brief 单一文件的扫描接口
 *		监控程序自己根据选项进行过滤，调用时都会进行扫描
 * 主要用于监控类的模块调用扫描
 */
struct IScanSimple {
protected:
	~IScanSimple() = default;
public:
	virtual void AV_CALLTYPE Dispose() PURE;
	virtual void AV_CALLTYPE SetOtpions(const ScanOptions* pOptions) PURE;

	virtual bool AV_CALLTYPE ScanFile(const wchar_t* szPath, ScanResult* pResult) PURE;
	/**
	 * 直接对内存的文件进行扫描处理
	 *	1. 对于清除后变大的情况，将只返回 unDataLen 提供的内存大小，后面会被截断
	 */
	virtual bool AV_CALLTYPE ScanMemoryFile(void* pData, uint32_t& unDataLen, const wchar_t* szExt, ScanResult* pResult) PURE;
};

enum EWorkerState {
	stateIdle,
	stateScanning,
	statePausing,
};

struct IScanNotify;
/** 
 * @brief 遍历扫描接口
 * 
 */
struct IScanWorker {
protected:
	~IScanWorker() = default;
public:
	virtual void AV_CALLTYPE Dispose() PURE;

	/**
	 * @brief 设置接收接口
	 */
	virtual void AV_CALLTYPE SetNotify(IScanNotify* pNotify) PURE;
	/**
	 * @brief 将扫描选项恢复到默认值
	 *		重置所有设置及扫描目标
	 * @retval 如果正在扫描或暂停返回 false ，否则可以重置则进行重置并返回 true
	 */
	virtual bool AV_CALLTYPE Reset() PURE;
	virtual void AV_CALLTYPE SetOptions(const ScanOptions* pOptions) PURE;
	virtual void AV_CALLTYPE SetClientProcessId(uint32_t unProcessId) PURE;

	virtual void AV_CALLTYPE SetHandleMode(const wchar_t* szVirusPrefix, EHandleMode handleMode) PURE;
	// 一次添加一个，带 . 前缀
	virtual void AV_CALLTYPE AddTargetExt(const wchar_t* szExt) PURE;
	// 大于当前Level值的扩展名忽略
	virtual void AV_CALLTYPE AddIgnoreExt(const wchar_t* szExt, uint32_t unLevel) PURE;

	/**
	@brief 添加扫描目标

	添加需要扫描的目标，目标可以是文件夹或文件路径，也可以是特定的扫描目标，特定扫描目标以
	\\!!\ 为前缀，后面跟指定的目标名称，已定义的目标有：
	Document
	AllDisk
	Computer
	Autorun
	MailBox
	Memory
	AllCDROM
	AllRemovable
	Cache&Temp
	SystemFolder
	Desktop
	Program
	MBR %d : 第几块硬盘的主引导区
	BOOT %c ：逻辑盘 C 的引导区
	使用时请注意大小写，区分大小写

	@param szUri 扫描目标路径

	@retval 一般返回 S_OK
	*/
	virtual void AV_CALLTYPE AddTarget(const wchar_t* szUri) PURE;

	virtual void AV_CALLTYPE AddIgnoreFolder(const wchar_t* szPath) PURE;

	virtual bool AV_CALLTYPE Start( uint32_t unWorkerCount, uint32_t unFromId, uint32_t unLevel ) PURE;
	virtual bool AV_CALLTYPE Pause() PURE;
	virtual void AV_CALLTYPE Resume() PURE;
	virtual void AV_CALLTYPE Stop() PURE;

	virtual void AV_CALLTYPE AdjustLevel(uint32_t unLevel) PURE;
	virtual EWorkerState AV_CALLTYPE QueryState( uint32_t& unFromId ) PURE;
};

struct IScanNotify {
protected:
	~IScanNotify() = default;
public:
	// 下面三个函数在调度线程中运行
	virtual void AV_CALLTYPE OnPreStart() PURE;

	// 添加的一个扫描目标开始扫描
	virtual void AV_CALLTYPE OnTargetStart( const wchar_t* szUri ) PURE;
	virtual void AV_CALLTYPE OnPostCompleted(bool bCancel) PURE;
	virtual void AV_CALLTYPE OnReportProgress(uint64_t ullFull, uint64_t ullComplete) PURE;

	// 下面的函数在扫描线程中运行
	/**
	 * 开始扫描时的回调
	 * @param szUri 具体的扫描文件路径
	 * @param unFileSize 文件大小
	 */
	virtual EHandleMode AV_CALLTYPE OnPreScanStart(const wchar_t* szUri, uint32_t unFileSize) PURE;
	/**
	 * 大于 10MB 的文件在备份时通知，用于界面上提示
	 */
	virtual void AV_CALLTYPE OnBackupStart(const wchar_t* szPath) PURE;
	virtual void AV_CALLTYPE OnBackupFinish(const wchar_t* szPath) PURE;
	/**
	 * 用于通知界面程序备份
	 */
	virtual EHandleMode AV_CALLTYPE OnBackupFailed(const wchar_t* szPath) PURE;

	virtual void AV_CALLTYPE OnStep( ) PURE;
	virtual void AV_CALLTYPE OnVirus( const wchar_t* szPath, const ScanResult* pResult) PURE;
};

EXTERN_C_BEGIN

	IScanWorker* AV_CALLTYPE ScanWorkerCreate( );
	IScanSimple* AV_CALLTYPE ScanSimpleCreate( );
	// 格式 xx.yy.yyyyMMdd
	const wchar_t* AV_CALLTYPE ScanGetVersion( );
	// 单位为秒，从 January 1, 1970, 0:00 UTC 开始计算
	__time64_t AV_CALLTYPE ScanGetLibDate();

	/**
	 * 升级过程中，扫描过程挂起
	 */
	bool AV_CALLTYPE PrepareDefUpdate();
	bool AV_CALLTYPE PostDefUpdate();
	// 备份文件只考虑还原，其他操作由程序直接通过文件操作完成
	bool AV_CALLTYPE BackupRestore(const wchar_t* szBackPath, const wchar_t* szRestoreTo);
	/**
	 * @parma pFile 要上报的文件，只在函数执行过程中有效
	 * @param unVirusId 对应的病毒Id，未确定是病毒时为 0 
	 * @param md5 病毒文件的 MD5 值
	 * @param reason 上报原因，暂时为 0
	 */
	typedef void (AV_CALLTYPE *FUNC_NotifyFindFile)(IFileEx* pFile, uint32_t unVirusId, uint8_t md5[16], int32_t reason );
	void AV_CALLTYPE SetNotifyFindFile(FUNC_NotifyFindFile func);
EXTERN_C_END
