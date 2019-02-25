#pragma once

#include "TypeDef.h"

// ���»�����ؽӿ� ������Ҫ�����������������
// int32_t AV_CALLTYPE OnUpdated(  );

//////////////////////////////////////////////////////////////////////////
// �ļ�����ʶ��ģ��
//		
enum EFTRESULT
{
	EFT_NEEDNOT_CHECK	  = 65535,	// ���ı���ý�塢�����ļ�������Ҫ�����鶾
	EFT_NEED_UNPACK_ZIP	  = 1,	// ��֪Arch�ǣ���Ҫ�ѿǡ�
								// wSubType �ǿ����
	EFT_NEED_UNEXEPACK	  = 2,	// ��֪Exep�ǣ���Ҫ�ѿǡ�
								// wSubType �ǿ����
	EFT_NEED_CHECK_UNPACK = 3,	// ��Ҫ����ʶ��ӿ�
	EFT_NEED_CHECK_FILE   = 4,	// ��ʶ�������/���죬��Ҫ�鶾��
								// wFileType/ wSubType �Ǻ����鶾���������

	EFT_UNKNOWN_PROG	  = 10,	// ����ʶ��ĳ��򣬲�������֪����
								// �����Ǳ��β�����δ֪�ǡ�����Ⱦ
								// �鶾ʱ������ʹ��Ӧ���鶾�����⴦��
								// ������ȡʱ������Ҫ�˹�����
								// wFileType �ǳ������࣬��PE/NE��
};

#define ST2_ZIP         0x92    // ѹ����

// �ڴ�֮����ļ����ͣ��������Ƿ����Exep���
#define ST2_CHECK_EXEP_BEGIN	0x30
#define ST2_CHECK_EXEP_END		0x90

/*
// �ļ����ඨ�壬�鶾�����ݴ�ֵʹ�ö�Ӧ��������
enum FileMainType {
	FTYPE_ZIP		= 1,		// ѹ����
	FTYPE_PKEXE		= 2,		// ��ִ�г���ӿ�
	FTYPE_APK		= 3,		// Android ������������
	FTYPE_IOSAPP	= 4,		// MacOS/IOS app ������������
	FTYPE_DISK		= 5,		// ���̡������������ļ�
	FTYPE_MACRODOC  = 6,		// Office �ĵ���docx��xlsx������겡��
	FTYPE_SCRIPT	= 7,		// �ű��ļ�������shell�ű���Bat�����vbs��java_script�ȣ�����鶾
	FTYPE_KNOWNIMAGE= 8,		// ���ܰ���������Media�ļ������鶾
	FTYPE_KNOWNDAT  = 9,		// ���ܰ��������������ļ������鶾
	FTYPE_DOSCOM	= 10,		// .COM, DOS ����
	FTYPE_DOSMZ		= 11,		// DOS EXE
	FTYPE_NE		= 12,		// Win16 NE, ���� LX
	FTYPE_PE		= 13,		// ��׼Windows���򣬰�����֪�Ͳ���ʶ�������
	FTYPE_ELF		= 14,		// ��׼Linux����  ������֪�Ͳ���ʶ��ĳ�������

	FTYPE_LIB		= 20,		// �������ӵĶ�̬lib��̬�⣬�����鶾
	FTYPE_TXT		= 21,		// �ı��ļ��������鶾
	FTYPE_BIN		= 22,		// ��֪�������ļ��������鶾
	FTYPE_MEDIA		= 23,		// ��֪Media�������鶾
	FTYPE_UNKNOWN   = 24,		// δ֪�ļ�����������com�������鶾
};

// �ļ����ඨ�壬ÿ�����������һ�����࣬ÿ������������1��ʼ����, 0 ��ʾδ֪
// ����Ҫ�鶾���ȳ����ע������ɲ����壬�ڲ�ʹ��
enum FileSubType {
	// ��� FTYPE_ZIP,		// ѹ����
	FSUB_7Z			= 1,		// 7Z �����ʶ�������
	FSUB_CAB		= 2,
	FSUB_SIS		= 3,		// Symbin �����
	FSUB_SISX		= 4,		// Symbin ����� 
	// ...


	// ��� FTYPE_PKEXE,		// ��ִ�г���ӿ�
	// ...

	// ��� FTYPE_APK,		// Android ������������

	// ��� FTYPE_IOSAPP,		// MacOS/IOS app ������������
	FSUB_IPA		= 1,		// IOS ipa
	FSUB_MACAPP		= 2,		// MacOS app

	// ��� FTYPE_DISK,		// ���̡������������ļ�
	FSUB_MBRFILE	= 1,
	FSUB_BOOTFILE	= 2,
	FSUB_MBRDISK	= 3,		// �������������
	FSUB_BOOTDISK   = 4,		// �߼�����������

	// ��� FTYPE_MACRODOC,		// Office �ĵ���docx��xlsx������겡��

	// ��� FTYPE_SCRIPT,		// �ű��ļ�������shell�ű���Bat�����vbs��java_script�ȣ�����鶾
	FSUB_BAT		= 1,		// ;.BAT;.CMD;
	FSUB_PY			= 2,		// ;.PY;.PYW
	FSUB_VBS		= 3,		// ;.VBS;.VBE;WSF;.WSH;.MSC
	FSUB_JS			= 4,		// ;.JS;.JSE;.WSF;.WSH;.MSC
	FSUB_SHELL		= 5,		// Linux shell

	// ��� FTYPE_NE,			//
	FSUB_NE			= 1,
	FSUB_LX			= 2,

	// ��� FTYPE_PE,			// ������
	FSUB_MSVC		= 1,
	FSUB_MSVB		= 2,
	FSUB_DOTNET		= 3,
	FSUB_CSHARP		= 4,

	FSUB_BORLANDC	= 4,

	FSUB_YI			= 10,		// ������
	FSUB_PY2EXE		= 11,		// PY 
	FSUB_PYEASY		= 12,		// PY 


	// ��� FTYPE_ELF,		// ��׼Linux����  ������֪�Ͳ���ʶ��ĳ�������
	// ��� FTYPE_KNOWNIMAGE,		// ���ܰ���������Media�ļ������鶾
	// ��� FTYPE_KNOWNDAT,		// ���ܰ���������Media�ļ������鶾

	// ��� FTYPE_TXT,		// �ı��ļ��������鶾
	// ��� FTYPE_BIN,		// ��֪�������ļ��������鶾
	// ��� FTYPE_MEDIA,		// ��֪Media�������鶾
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

#define CPU_MASK			0x00FF		// CPU �ܹ�����

#define CPU_LITTLE_ENDIAN	0x0000
#define CPU_BIG_ENDIAN		0x8000

#define CPU_ENDIAN_MASK		0x8000		// CPU ��С��ģʽ����
*/

struct CEngData;

#include "PshPack2.h"
typedef struct _FTRESULT
{
	uint16_t wFileType;		// ���ļ����Ͷ��壬�鶾�����ݴ�ֵʹ�ö�Ӧ��������
	uint16_t wSubType;		// ������
	uint16_t wArchFlag;		// CPU �ܹ�������ģʽ
	uint16_t wResult;		// ʶ�������ݴ˽��к�������
	uint32_t dwOffs;		// ���������ļ�����ƫ��
	uint32_t dwLen;			// �������򳤶�
	uint32_t dwRecNo;		// ƥ���¼�ţ����ڷ�������
	uint32_t dwRev;			// ����
} FTRESULT;
#include "PopPack.h"

#define MIN_FILE_SIZE		6	// �ļ���С��С

/** 
 * @param szDataFolder ʶ�����������ļ������ļ��е�·�����ļ����� PATH_SPLIT_CHAR ��β 
 * @retval 0 �ɹ�������ֵ��ʾʧ��
 */
int32_t AV_CALLTYPE FT_Init( IN const char* szDataFolder);
void    AV_CALLTYPE FT_Clear();

/*
  * ��ȡɨ�軺�����������ɨ���ļ�����
  * ��ʼ��ɨ��������ݣ���ȡ�ļ���С�Լ��ļ�ͷ��������
  * @param pFile ��Ҫɨ����ļ�
  * @retval NULL��ʧ�ܣ�ԭ�����ļ���С����󣬲���Ҫɨ��
			��NULL �ɹ�
*/
CEngData* AV_CALLTYPE FT_GetScanPool(IN IFileEx* pFile);

// �ͷ�ɨ�軺����
void      AV_CALLTYPE FT_ReleaseScanPool(IN CEngData*  pAVEData);

/**
 * @param result �ļ��ľ���������Ϣ
 * 
 * @retval 0 �ɹ�������ֵ��ʾʧ��
 */
int32_t AV_CALLTYPE FT_RecongizeFirst(IN OUT CEngData*  pAVEData, OUT FTRESULT* result);

/**
* @param result �ļ��ľ���������Ϣ
*
* @retval 0 �ɹ�������ֵ��ʾʧ�ܣ�����Ҫ��������FT_RecongizeNext
*/
int32_t AV_CALLTYPE FT_RecongizeNext(IN OUT CEngData*  pAVEData, OUT FTRESULT* result);


//////////////////////////////////////////////////////////////////////////
// �ļ�ָ�ƹ���ģ��
class IFinger;

/*
typedef IFinger* (AV_CALLTYPE *Func_CreateFinger) ();

struct FileFinger {
	uint32_t unSize;
	FileMainType mainType;
	uint32_t LastScanVersion;	// �ϴ�ɨ���������汾
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
// ѹ��������ģ�飨�����Խ�ѹ�����Խ�ѹ�����ݾ�������ж��Ƿ�EXE��ѹ��������
class IArchiveCallback
{
protected:
	~IArchiveCallback() = default;
public:
	/**
	 *	����ѹ��������ģ���ȡһ����ѹ�õ���ʱ�ļ����� AVEng ʵ��һ����ʱ�ļ�
	 *	���󣬱���ÿ��ѹ����ģ��ʵ��һ����ʱ�ļ��ӿ�
	 *	
	 * @param szRelativePath ѹ�����ڵ����·��������Ϊ nullptr 
	 * @param szExt ѹ�������ļ��Ŀ�����չ��������Ϊ nullptr
	 */
	virtual IFileEx* AV_CALLTYPE GetTempFile( IN const wchar_t* szRelativePath, IN const wchar_t* szExt ) PURE;
	enum EAction
	{
		actionKeep,
		actionModify, // �޷����ѹ�����Ļ���ɾ������
		actionDelete,
	};
	virtual EAction AV_CALLTYPE ScanItem(IN IFileEx* pItem, OUT wchar_t szVirus[MAX_VIRNAME_LEN]) PURE;
	enum EActionResult
	{
		modifyOk = 0x10,		// ���Ը���Ϊ����������
		modifyFailed = 0x11,	// ѹ������֧�ָ���
		deleteOk = 0x20,		// ����ɾ��
		deleteFailed = 0x21,	// ѹ������֧��ɾ��
		deleteFull = 0x28,		// ��Ҫɾ������ѹ����
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
	 �Ƿ�֧�ְ��ڸ��£�
	 */
	virtual bool AV_CALLTYPE SupportUpdate() PURE;
	/**
	 * @retval ѹ�����е��ļ���������ȷ��ʱ���� -1
	 */
	virtual int32_t AV_CALLTYPE ItemCount() PURE;
	virtual void AV_CALLTYPE ExtractAll(IN IArchiveCallback * pCallback) PURE;
	/**
	 ��֧�ְ��ڸ��²������ļ�ɾ�������ʱ����
	@param szBackupId ���ݵľ����ʶ
	 */
	virtual bool AV_CALLTYPE Update( const wchar_t* szBackupId ) PURE;
};

int32_t AV_CALLTYPE ZIP_Init(IN const char* szDataFolder);
void AV_CALLTYPE ZIP_Clear();
IArchive* AV_CALLTYPE ZIP_Open(IFileEx* pFile, uint16_t usSubType );

//////////////////////////////////////////////////////////////////////////
// �ѿ�ģ��
// ���ѿǺ����������������ļ��иı䣬��������ô������ļ�ֱ�Ӹ���ԭʼ�ļ�

int32_t AV_CALLTYPE PKEXE_Init(IN const char* szDataFolder);
void    AV_CALLTYPE PKEXE_Clear();
const char* AV_CALLTYPE PKEXE_GetName(int index);	// �����ã���ȡ Exep Name


// �ѿ�
// ���أ�
//		< 0, �ѿ�ʧ�ܣ����� -ID
//		= 0, ����
//		> 0, �ѿǳɹ������� ID
int  AV_CALLTYPE PKEXE_Unpack(IN OUT CEngData*  pAVEData, OUT IFileEx* pTarget,IN  FTRESULT* ftInfo = nullptr);

//////////////////////////////////////////////////////////////////////////
// ɨ��ģ��
//		������AVScanA AVScanEP AVScanT AVScanM AVScanD AVScanS �ɰ��������ģ�� AVCloud
// ����ɨ��ģ�������� AVScan ��ͷ
// TODO : �޸ĳ�Ա�����Ĵ��η�ʽ

class IScan { 
protected:
	virtual ~IScan() = default;
public:
	enum EHandleMode {
		CanCure  = 0x01, // �ܹ����
		NeedRepair = 0x02, // ��Ҫ��������ģ��
		NeedReport = 0x10, // ��Ҫ�ϱ�����
	};
	// ��������ϱ� -->���/ɾ��-->�޸�
	virtual void AV_CALLTYPE Dispose() PURE;

	/**
	 * ��ȡĳ�����͵�������
	 * 
	 * @param unSubFileType �ļ�������
	 * @retval ��������
	 */
	virtual uint32_t AV_CALLTYPE ReadRecTotal( uint16_t unSubFileType ) PURE;
	/** 
	 * ɨ��ģ����Ҫ���ȶ��ļ����ͽ��м�飬�����Լ���֧�ֵ��ļ�����ֱ�ӷ��� (uint32_t)-1 
	 * 
	 * @param pAVEData Ҫɨ���������Ϣ������ʶ��ģ��� FT_GetScanPool ����
	 * @param pFileType ��ʶ��ģ�鷵�ص��ļ�����
	 * @param unLastRecNo ���ɨ���¼�ţ� 
	 * @param pMode ָ�����Ĵ���ʽ��Ϊ EHandleMode ���Ƶ�ֵ
	 * @retval ƥ�䵽�ļ�¼�ţ���ƥ�䷵�� (uint32_t)-1
	 */
	virtual uint32_t AV_CALLTYPE Scan(IN OUT CEngData*  pAVEData,
		FTRESULT* pFileType, uint32_t unLastRecNo, OUT uint32_t* pMode) PURE;

	/**
	 * ִ���������
	 * @param pAVEData Ҫɨ���������Ϣ������ʶ��ģ��� FT_GetScanPool ����
	 * @param unSubFileType �ļ�������
	 * @param unRecNo ɨ��ʱƥ��ļ�¼��
	 */
	virtual int32_t AV_CALLTYPE Cure(IN OUT CEngData*  pAVEData,
		uint16_t unSubFileType, uint32_t unRecNo) PURE;
};

IScan* AV_CALLTYPE  CreateScan();

int32_t AV_CALLTYPE SCAN_Init(IN const char* szDataFolder);
void    AV_CALLTYPE SCAN_Clear();

// for Debug ɨ���ļ�
//		< 0, ����
//		= 0, δƥ��
//		> 0, ƥ��ɹ������� ID
int  AV_CALLTYPE SCAN_File(CEngData *pAVEData, int blCure = 0);


// ����ģ��
// 
int32_t AV_CALLTYPE FIX_Init( const char* szDataFolder );
// pFileָ����ļ���������ɱ��ʱɾ��
int32_t AV_CALLTYPE FIX_Repair( IFileEx* pFile, uint32_t unRecNo );
int32_t AV_CALLTYPE FIX_Clear();
