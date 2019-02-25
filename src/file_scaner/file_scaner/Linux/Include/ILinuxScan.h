#pragma once

// AVEMgr 扫描调度模块提供的扫描接口


// 初始化引擎
// 
// pszVLibPath 病毒库根目录，以'\'结尾
//
// 返回:
//		0 = 成功
//
int AV_InitEng(const char* pszVLibPath);

// 清理引擎
int AV_ClearEng();

typedef enum
{
	MODE_SCAN = 0,
	MODE_CURE = 1,
	MODE_DELETE = 2,
} SCANMODE;

typedef enum
{
	ERESULT_NONE = 0,
	ERESULT_FIND,
	ERESULT_KILL,
	ERESULT_DELETE,

	ERESULT_EXEP_OK = 10,
	ERESULT_EXEP_FAIL,
} SCANRESULT;

//
// 批量扫描的回调接口
//
class IScanNotify
{
public:
	virtual ~IScanNotify() { }

	// 文件扫描开始
	virtual void OnScanStart(const char* filePath)
	{
	}

	// 脱壳通知
	virtual void OnFindExep(const char*		filePath,
		const char*		exepName,
		unsigned int	dwExepRecNo,
		SCANRESULT		OpResult)
	{
	}

	// 解压通知
	virtual void OnFindArchive(const char*	filePath,
		const char*		zipName,
		unsigned int	dwZipRecNo,
		SCANRESULT		OpResult)
	{
	}

	// 病毒通知
	virtual void OnFindVirus(const char*		filePath,
		const char*		virName,
		unsigned int	dwVirRecNo,
		SCANRESULT		OpResult)
	{
	}

	// 扫描结束
	virtual void OnScanEnd(const char* filePath)
	{
	}
};

// 扫描接口
class ILinuxScan
{
public:
	virtual void Dispose() = 0;
	virtual int  AV_ScanDir(const char* dirname, SCANMODE mode) = 0;
	virtual int  AV_ScanFile(const char* path, SCANMODE mode, char* virName, unsigned int* pdwVirRecNo) = 0;
};

ILinuxScan* NewInstance_ILinuxScan(IScanNotify* notifyCallback);

//
// 对文件查毒
//
// filePath			查毒文件完整路径，非文件夹
// virName,			返回发现的病毒名
// pdwVirRecNo		返回病毒记录号
//
// 返回：
// 
//
int AV_ScanFile(const char* filePath, SCANMODE mode, char* virName, unsigned int* pdwVirRecNo);

//
// 扫描一个或一组目标
//
int AV_ScanObject(const char** scanObj, int iObjCount, SCANMODE mode, ILinuxScan* pILinuxScan);

