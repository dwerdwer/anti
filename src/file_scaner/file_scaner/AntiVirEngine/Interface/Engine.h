#pragma once

#include "TypeDef.h"

//////////////////////////////////////////////////////////////////////////
// 引擎模块 AVEng

/**
* 引擎创建函数，函数名为 CreateEngine
*/
enum EHandleMode {
	modeIgnore,
	modeFind,
	modeCure,
	modeDelete,
	modeStop // 仅用于备份时返回值
};

class IScanEngine {
protected:
	virtual ~IScanEngine() {
	}
public:

	virtual void AV_CALLTYPE Dispose() PURE;

	virtual void AV_CALLTYPE SetMaxUnzip(uint32_t unMax) PURE;
	virtual void AV_CALLTYPE SetMaxUnPkexe(uint32_t unMax) PURE;
	virtual void AV_CALLTYPE ScanFile( IN IFileEx* pFile) PURE;
	virtual void AV_CALLTYPE ScanPath(IN const wchar_t* szPath) PURE;
	virtual void AV_CALLTYPE ScanMemory(IN const wchar_t* szExt, IN void* pData, IN uint32_t& unSize) PURE;
	virtual void AV_CALLTYPE ScanDisk(IN const wchar_t* szDisk) PURE;

	virtual bool AV_CALLTYPE CanAbort() PURE;
	virtual void AV_CALLTYPE Abort() PURE;
	virtual void AV_CALLTYPE PostAbortCleanup() PURE;
};

	enum EScanResult {
		scanNormal	= 0x0,
		scanFind	= 0x01,
		scanCured = 0x10,
		scanCureReboot = 0x11,
		scanCureFailed = 0x13,
		scanDelete = 0x28,
		scanDeleteReboot= 0x21,
		scanDeleteFailed = 0x23,
		scanMaskFind = 0x01,
		scanMaskFailed = 0x02,
		scanMaskNotExisted = 0x08
	};

	class IFileEx;
/**
* 由调用者实现
*/
class IScanPrompt
{
protected:
	virtual ~IScanPrompt() {

	}
public:
	virtual EHandleMode AV_CALLTYPE PreScan(IN const wchar_t* szPath, IN int64_t llFileSize ) PURE;
	virtual EHandleMode AV_CALLTYPE PreCure(IN const wchar_t* szPath, IN const wchar_t* szVirusName) PURE;
	virtual void AV_CALLTYPE NotifyResult(IN const wchar_t* szPath, IN uint32_t unVirusNo,
		IN const wchar_t* szVirusName, IN EScanResult result) PURE;

	virtual void AV_CALLTYPE RunStep() PURE;

	/**
	 * 在清除或删除前进行文件备份
	 * @param szPath 要备份的文件路径
	 * @retval 返回备份文件完整路径，如果不需要备份返回 null
	 */
	virtual const wchar_t* AV_CALLTYPE BackupStart(IN const wchar_t* szPath, IN int64_t llFileSize ) PURE;
	virtual void AV_CALLTYPE BackupFinish(IN const wchar_t* szPath, IN int64_t llFileSize ) PURE;
	virtual EHandleMode AV_CALLTYPE BackupFailed(IN const wchar_t* szPath) PURE;

	virtual void AV_CALLTYPE ReportFile( IFileEx* pFile, uint32_t unVirusId, uint8_t md5[16], int32_t reason ) PURE;
};

/**
* 接口创建函数，函数名为 CreateEngine
*/
extern "C" int32_t AV_CALLTYPE EngineInit(wchar_t const* szFolder, const wchar_t* szBackupFolder );
typedef IScanEngine* (AV_CALLTYPE* Func_EngineCreate)( IScanPrompt* pPrompt );
extern "C" IScanEngine* AV_CALLTYPE EngineCreate( IScanPrompt* pPrompt );

extern "C" int32_t AV_CALLTYPE FileRestore(const wchar_t* szBackPath, const wchar_t* szRestorePath );
