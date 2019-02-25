#pragma once

#include "TypeDef.h"
#include "Engine.h"

#ifndef MAX_PATH
#define MAX_PATH          260
#endif

#include <pshpack1.h>
struct ScanOptions {
	uint32_t m_dwSize; // �ṹ��С
	enum EFlags {
		scanUnzip = 0x01,		// ɨ��ѹ����
		scanUnpack = 0x02,		// �ѿ�
		scanStopOnOne = 0x10,   // ����һ��������ֹͣ

		scanProgramOnly		= 0x100,	// ֻɨ�����

		scanOriginalMd5		= 0x1000,	// ��Ҫ����ԭʼ�ļ��� MD5ֵ
		scanUseFigner		= 0x2000,	// �Ƿ�ʹ��ָ�ƣ������ļ�ɨ�衢�ļ����ʹ�á���ҳ��ء��ʼ���ز���Ҫʹ��
		scanUseCloud		= 0x4000,	// �Ƿ�ʹ��������
		scanBackup			= 0x8000	// �ļ���ҪǰӦ���б���
	};

	uint32_t m_dwFlags;		  // ɨ��ѡ���־���
	EHandleMode m_handeMode;  // ���崦��ʽ
	uint32_t m_dwMaxFileSize; // ���ɨ���ļ���С, 0 Ϊ������ 
	uint32_t m_dwMaxUnzipFileSize; // ����ѹ�ļ���С�� 0Ϊ������
};

/**
 * ����ѹ�����а���������������ص�һ����������Id
 * ��Ⱦ�������Ҳ�ǵ�һ����������Id
 * ֻ�Զ����ļ����ѿ�Ҳ��ԭʼ�ļ�������MD5��ѹ����������
 */
struct ScanResult {
	uint32_t m_dwSize; // �ṹ��С
	enum  EFlags {
		withMd5		= 0x10,
	};
	
	char m_szVirusName[MAX_VIRNAME_LEN]; // ��������
	uint8_t m_arrMd5[16];			// ����ԭʼ�ļ��� MD5 ֵ��δɱ��ǰ�ģ�
	char m_szBackupId[MAX_PATH]; // ���ݵ��ļ���
	uint32_t m_dwVirusId; // ������¼Id
	EScanResult m_result; // ���崦����
};

#include <poppack.h>
/**
 * @brief ��һ�ļ���ɨ��ӿ�
 *		��س����Լ�����ѡ����й��ˣ�����ʱ�������ɨ��
 * ��Ҫ���ڼ�����ģ�����ɨ��
 */
struct IScanSimple {
protected:
	~IScanSimple() = default;
public:
	virtual void AV_CALLTYPE Dispose() PURE;
	virtual void AV_CALLTYPE SetOtpions(const ScanOptions* pOptions) PURE;

	virtual bool AV_CALLTYPE ScanFile(const wchar_t* szPath, ScanResult* pResult) PURE;
	/**
	 * ֱ�Ӷ��ڴ���ļ�����ɨ�账��
	 *	1. �������������������ֻ���� unDataLen �ṩ���ڴ��С������ᱻ�ض�
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
 * @brief ����ɨ��ӿ�
 * 
 */
struct IScanWorker {
protected:
	~IScanWorker() = default;
public:
	virtual void AV_CALLTYPE Dispose() PURE;

	/**
	 * @brief ���ý��սӿ�
	 */
	virtual void AV_CALLTYPE SetNotify(IScanNotify* pNotify) PURE;
	/**
	 * @brief ��ɨ��ѡ��ָ���Ĭ��ֵ
	 *		�����������ü�ɨ��Ŀ��
	 * @retval �������ɨ�����ͣ���� false ���������������������ò����� true
	 */
	virtual bool AV_CALLTYPE Reset() PURE;
	virtual void AV_CALLTYPE SetOptions(const ScanOptions* pOptions) PURE;
	virtual void AV_CALLTYPE SetClientProcessId(uint32_t unProcessId) PURE;

	virtual void AV_CALLTYPE SetHandleMode(const wchar_t* szVirusPrefix, EHandleMode handleMode) PURE;
	// һ�����һ������ . ǰ׺
	virtual void AV_CALLTYPE AddTargetExt(const wchar_t* szExt) PURE;
	// ���ڵ�ǰLevelֵ����չ������
	virtual void AV_CALLTYPE AddIgnoreExt(const wchar_t* szExt, uint32_t unLevel) PURE;

	/**
	@brief ���ɨ��Ŀ��

	�����Ҫɨ���Ŀ�꣬Ŀ��������ļ��л��ļ�·����Ҳ�������ض���ɨ��Ŀ�꣬�ض�ɨ��Ŀ����
	\\!!\ Ϊǰ׺�������ָ����Ŀ�����ƣ��Ѷ����Ŀ���У�
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
	MBR %d : �ڼ���Ӳ�̵���������
	BOOT %c ���߼��� C ��������
	ʹ��ʱ��ע���Сд�����ִ�Сд

	@param szUri ɨ��Ŀ��·��

	@retval һ�㷵�� S_OK
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
	// �������������ڵ����߳�������
	virtual void AV_CALLTYPE OnPreStart() PURE;

	// ��ӵ�һ��ɨ��Ŀ�꿪ʼɨ��
	virtual void AV_CALLTYPE OnTargetStart( const wchar_t* szUri ) PURE;
	virtual void AV_CALLTYPE OnPostCompleted(bool bCancel) PURE;
	virtual void AV_CALLTYPE OnReportProgress(uint64_t ullFull, uint64_t ullComplete) PURE;

	// ����ĺ�����ɨ���߳�������
	/**
	 * ��ʼɨ��ʱ�Ļص�
	 * @param szUri �����ɨ���ļ�·��
	 * @param unFileSize �ļ���С
	 */
	virtual EHandleMode AV_CALLTYPE OnPreScanStart(const wchar_t* szUri, uint32_t unFileSize) PURE;
	/**
	 * ���� 10MB ���ļ��ڱ���ʱ֪ͨ�����ڽ�������ʾ
	 */
	virtual void AV_CALLTYPE OnBackupStart(const wchar_t* szPath) PURE;
	virtual void AV_CALLTYPE OnBackupFinish(const wchar_t* szPath) PURE;
	/**
	 * ����֪ͨ������򱸷�
	 */
	virtual EHandleMode AV_CALLTYPE OnBackupFailed(const wchar_t* szPath) PURE;

	virtual void AV_CALLTYPE OnStep( ) PURE;
	virtual void AV_CALLTYPE OnVirus( const wchar_t* szPath, const ScanResult* pResult) PURE;
};

EXTERN_C_BEGIN

	IScanWorker* AV_CALLTYPE ScanWorkerCreate( );
	IScanSimple* AV_CALLTYPE ScanSimpleCreate( );
	// ��ʽ xx.yy.yyyyMMdd
	const wchar_t* AV_CALLTYPE ScanGetVersion( );
	// ��λΪ�룬�� January 1, 1970, 0:00 UTC ��ʼ����
	__time64_t AV_CALLTYPE ScanGetLibDate();

	/**
	 * ���������У�ɨ����̹���
	 */
	bool AV_CALLTYPE PrepareDefUpdate();
	bool AV_CALLTYPE PostDefUpdate();
	// �����ļ�ֻ���ǻ�ԭ�����������ɳ���ֱ��ͨ���ļ��������
	bool AV_CALLTYPE BackupRestore(const wchar_t* szBackPath, const wchar_t* szRestoreTo);
	/**
	 * @parma pFile Ҫ�ϱ����ļ���ֻ�ں���ִ�й�������Ч
	 * @param unVirusId ��Ӧ�Ĳ���Id��δȷ���ǲ���ʱΪ 0 
	 * @param md5 �����ļ��� MD5 ֵ
	 * @param reason �ϱ�ԭ����ʱΪ 0
	 */
	typedef void (AV_CALLTYPE *FUNC_NotifyFindFile)(IFileEx* pFile, uint32_t unVirusId, uint8_t md5[16], int32_t reason );
	void AV_CALLTYPE SetNotifyFindFile(FUNC_NotifyFindFile func);
EXTERN_C_END
