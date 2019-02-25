#pragma once
#include "ILinuxScan.h"
#include "../AntiVirEngine/CB_Linux/CBTypeDef.h"


struct CEngData;

//
// 扫描处理类，适用于扫描文件夹等，含扫描计数信息
//
class LinuxScan: public ILinuxScan
{
private:
	IScanNotify* notifyCallback;

	int	 g_fileCount;
	int  g_virusCount;
	int  g_unpackCount;
	int  g_unpackFailCount;
	char g_szCurrentFilePath[MAX_PATH];

public:
	LinuxScan(IScanNotify* notifyCallback);
	virtual ~LinuxScan();

public:
	virtual void Dispose() { delete this;  }
	virtual int  AV_ScanDir(const char* dirname, SCANMODE mode);
	virtual int  AV_ScanFile(LPCSTR path, SCANMODE mode, char* virName, unsigned int* pdwVirRecNo);

private:
	int  AV_ScanFile_Internal(CEngData* pAVE, int deepth);
	int  ScanFile_Loop(CEngData* pAVE, int deepth);
};

