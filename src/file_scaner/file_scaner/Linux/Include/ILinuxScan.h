#pragma once

// AVEMgr ɨ�����ģ���ṩ��ɨ��ӿ�


// ��ʼ������
// 
// pszVLibPath �������Ŀ¼����'\'��β
//
// ����:
//		0 = �ɹ�
//
int AV_InitEng(const char* pszVLibPath);

// ��������
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
// ����ɨ��Ļص��ӿ�
//
class IScanNotify
{
public:
	virtual ~IScanNotify() { }

	// �ļ�ɨ�迪ʼ
	virtual void OnScanStart(const char* filePath)
	{
	}

	// �ѿ�֪ͨ
	virtual void OnFindExep(const char*		filePath,
		const char*		exepName,
		unsigned int	dwExepRecNo,
		SCANRESULT		OpResult)
	{
	}

	// ��ѹ֪ͨ
	virtual void OnFindArchive(const char*	filePath,
		const char*		zipName,
		unsigned int	dwZipRecNo,
		SCANRESULT		OpResult)
	{
	}

	// ����֪ͨ
	virtual void OnFindVirus(const char*		filePath,
		const char*		virName,
		unsigned int	dwVirRecNo,
		SCANRESULT		OpResult)
	{
	}

	// ɨ�����
	virtual void OnScanEnd(const char* filePath)
	{
	}
};

// ɨ��ӿ�
class ILinuxScan
{
public:
	virtual void Dispose() = 0;
	virtual int  AV_ScanDir(const char* dirname, SCANMODE mode) = 0;
	virtual int  AV_ScanFile(const char* path, SCANMODE mode, char* virName, unsigned int* pdwVirRecNo) = 0;
};

ILinuxScan* NewInstance_ILinuxScan(IScanNotify* notifyCallback);

//
// ���ļ��鶾
//
// filePath			�鶾�ļ�����·�������ļ���
// virName,			���ط��ֵĲ�����
// pdwVirRecNo		���ز�����¼��
//
// ���أ�
// 
//
int AV_ScanFile(const char* filePath, SCANMODE mode, char* virName, unsigned int* pdwVirRecNo);

//
// ɨ��һ����һ��Ŀ��
//
int AV_ScanObject(const char** scanObj, int iObjCount, SCANMODE mode, ILinuxScan* pILinuxScan);

