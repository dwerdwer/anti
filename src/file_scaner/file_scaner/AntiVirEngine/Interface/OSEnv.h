#pragma  once

#include "TypeDef.h"

/** 
 * ��ʹ�ù����У���Ҫ�ȵ��� MoveNext �ҷ��� true�����ܵ��� Open �� GetPath 
 */
class IFolder {
protected:
	virtual ~IFolder() {
		
	}
public:
	virtual void Dispose() PURE;
	virtual bool MoveNext() PURE;
	virtual bool IsFolder() PURE;
	virtual const wchar_t* GetPath() PURE;
	virtual int32_t ItemCount() PURE;
};

typedef void* HLOCK;
enum ETraceLevel {
	traceFatal = 50000,
	traceError = 40000,
	traceWarn = 30000,
	traceInfo = 20000,
	traceDebug = 10000,
	traceMessage = 0,
};
/**
 * @brief ϵͳ�����ӿ�
 * 
 * �����ṩϵͳ�ĸ����ڴ������ƣ��ýӿ���Ҫ��Բ�ͬƽ̨�ṩ��ͬ��ʵ��
*/
class IOSBase {
protected:
	virtual ~IOSBase() {		
	}
public:
	virtual void AV_CALLTYPE Dispose() PURE;

	/************************************�ڴ���ش���****************************/
	/**
	 *@brief �����ڴ棬�� C++ �е� alloc ���ƣ�������Ҫ����������չ
	 * @param size Ҫ������ڴ��С
	 */
	virtual void* AV_CALLTYPE MemAlloc(size_t size) PURE;
	virtual void* AV_CALLTYPE MemReAlloc(void* pData, size_t size) PURE;
	virtual void AV_CALLTYPE MemFree(void* pData) PURE;
	virtual size_t AV_CALLTYPE MemSizeOf(void* pData) PURE;
	/**
	 * �ú����� MemFreeCode ��Ҫ���ھɿ⣬�ɿⲻ�ú����ȥ��
	 */
	virtual void* AV_CALLTYPE MemAllocCode(size_t size) PURE;
	virtual void AV_CALLTYPE MemFreeCode(void* p) PURE;

	static const uint32_t codePageCurrent = 0;
	static const uint32_t codePageUTF7 = 65000;       // UTF-7 translation
	static const uint32_t codePageUTF8 = 65001;       // UTF-8 translation

	/***********************************�ַ�����ش���(���ֽ�)********************/
	virtual int32_t AV_CALLTYPE StrMultiByteToWideChar( 
		uint32_t   CodePage,
		_In_      uint32_t  dwFlags,
		_In_      char const* lpMultiByteStr,
		_In_      int    cbMultiByte,
		_Out_opt_ wchar_t* lpWideCharStr,
		_In_      int    cchWideChar
	) PURE;
	virtual int32_t AV_CALLTYPE StrWideCharToMultiByte(
		_In_      uint32_t    CodePage,
		_In_      uint32_t   dwFlags,
		_In_      wchar_t const* lpWideCharStr,
		_In_      int     cchWideChar,
		_Out_opt_ char*   lpMultiByteStr,
		_In_      int     cbMultiByte,
		_In_opt_  char const*  lpDefaultChar,
		_Out_opt_ bool*  lpUsedDefaultChar
	) PURE;
	virtual char* AV_CALLTYPE StrCharNext(uint32_t CodePage, char const* szCurrent) PURE;
	virtual char* AV_CALLTYPE StrCharPrev(uint32_t CodePagte, char const* szStart, char const* szCurrent) PURE;

	virtual int AV_CALLTYPE StrPrintA(uint32_t codePage, char* buffer, const char * format, va_list argptr) PURE;
	virtual int AV_CALLTYPE StrPrintLenA( uint32_t codePage, const char * format, va_list argptr ) PURE;
	virtual int AV_CALLTYPE StrPrintW( wchar_t* buffer, const wchar_t * format, va_list argptr ) PURE;
	virtual int AV_CALLTYPE StrPrintLenW( const wchar_t * format, va_list argptr ) PURE;
	virtual int AV_CALLTYPE StrScanA(uint32_t codePage, const char* buffer, const char* format, va_list argptr) PURE;
	virtual int AV_CALLTYPE StrScanW(const wchar_t* buffer, const wchar_t* format, va_list argptr) PURE;

	/*************************************�ı���ش���******************************/
	/**
	@brief ��ָ���ļ�·����Ӧ���ļ�

	@param szPath ָ��Ҫ�򿪵��ļ�·��
	@param bReadOnly �Ƿ���ֻ����ʽ���ļ�

	@retval ����ɹ����� �򿪵��ļ�����ָ�룬���򷵻� NULL
	*/
	virtual IFileEx * AV_CALLTYPE FileOpen( const wchar_t* szPath, bool bReadOnly) PURE;
	/**
	@brief ��ָ���ļ�·�������ļ�

	@param szPath ָ��Ҫ�������ļ�·��

	@retval ����ɹ����� �������ļ�����ָ�룬���򷵻� NULL
	*/
	virtual IFileEx * AV_CALLTYPE FileCreate( const wchar_t* szPath) PURE;

	/**
	@brief ����һ����ʱ�ļ�
	��ʱ�ļ���һ������ɨ�������ʱ�������ļ������ļ��ر�(����Dispose)��Ӧ�ô��ļ�ϵͳ
	ɾ���� 

	@retval ����ɹ����� ��������ʱ�ļ������򷵻� NULL
	*/
	virtual IFileEx * AV_CALLTYPE FileCreateTemp() PURE;
	/**
	@brief �ж�ָ���ļ��Ƿ����
	*/
	virtual bool AV_CALLTYPE FileIsExist(const wchar_t* szPath) PURE;

	/**
	 * ����ɨ��Ŀ���е����ݣ����ļ�ϵͳ�����⣬��֧���ض�ϵͳ�Զ����Ŀ�꣬
	 * �ض�ϵͳ�Զ���Ŀ�� szFolder ����ĸΪ ? 
	 */
	virtual IFolder* AV_CALLTYPE FolderEnum( const wchar_t* szFolder, const wchar_t* szFilter) PURE;

	/***********************************ģ����ش���****************************/
	/**
	 * @brief ����һ��ģ��
	 * 
	 *		@param szName Ҫ���ص�ģ�����ƣ�ֻ��ģ���ļ����ļ����������������ļ��С���չ����
	 */
	virtual void* AV_CALLTYPE ModuleLoad( const wchar_t* szName ) PURE;
	/**
	 * @brief ��ȡģ���е�һ������
	 */
	virtual void* AV_CALLTYPE ModuleGetFunc(void* hModule, const wchar_t* szName) PURE;
	/**
	 * @brief ж��һ��ģ��
	 */
	virtual void AV_CALLTYPE ModuleUnload(void* hModule) PURE;

	/*******************************���д���֧��****************************/
	virtual bool AV_CALLTYPE RunTry( void (AV_CALLTYPE *func)(void* pArgs), void* pArgs ) PURE;
	virtual void AV_CALLTYPE RunThrow(uint32_t code, wchar_t const * szInfo) PURE;

	virtual void AV_CALLTYPE RunTrace(const wchar_t* const szModule, ETraceLevel level, const wchar_t* const szInfo) PURE;
	virtual void AV_CALLTYPE RunSleep() PURE;

	virtual HLOCK AV_CALLTYPE RunCreateLock() PURE;
	virtual void AV_CALLTYPE RunLock(HLOCK hLock) PURE;
	virtual void AV_CALLTYPE RunUnlock(HLOCK hLock) PURE;
	virtual void AV_CALLTYPE RunDisposeLock(HLOCK hLock) PURE;

	/******************************�̴߳洢����****************************/

	static const uint32_t TLS_FAILED = (uint32_t)-1;
	virtual uint32_t AV_CALLTYPE TlsAlloc() PURE;
	virtual bool AV_CALLTYPE TlsFree(uint32_t dwTlsIndex) PURE;
	virtual void* AV_CALLTYPE TlsGetValue(uint32_t dwTlsIndex) PURE;
	virtual bool AV_CALLTYPE TlsSetValue(uint32_t dwTlsIndex, void* pTlsValue) PURE;
};


extern "C" IOSBase* AV_CALLTYPE OS_Create();
