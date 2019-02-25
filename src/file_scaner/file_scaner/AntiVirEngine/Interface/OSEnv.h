#pragma  once

#include "TypeDef.h"

/** 
 * 在使用过程中，需要先调用 MoveNext 且返回 true，才能调用 Open 或 GetPath 
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
 * @brief 系统基本接口
 * 
 * 用于提供系统的辅助内存管理机制，该接口需要针对不同平台提供不同的实现
*/
class IOSBase {
protected:
	virtual ~IOSBase() {		
	}
public:
	virtual void AV_CALLTYPE Dispose() PURE;

	/************************************内存相关处理****************************/
	/**
	 *@brief 分配内存，与 C++ 中的 alloc 类似，根据需要可做其他扩展
	 * @param size 要分配的内存大小
	 */
	virtual void* AV_CALLTYPE MemAlloc(size_t size) PURE;
	virtual void* AV_CALLTYPE MemReAlloc(void* pData, size_t size) PURE;
	virtual void AV_CALLTYPE MemFree(void* pData) PURE;
	virtual size_t AV_CALLTYPE MemSizeOf(void* pData) PURE;
	/**
	 * 该函数与 MemFreeCode 主要用于旧库，旧库不用后可以去除
	 */
	virtual void* AV_CALLTYPE MemAllocCode(size_t size) PURE;
	virtual void AV_CALLTYPE MemFreeCode(void* p) PURE;

	static const uint32_t codePageCurrent = 0;
	static const uint32_t codePageUTF7 = 65000;       // UTF-7 translation
	static const uint32_t codePageUTF8 = 65001;       // UTF-8 translation

	/***********************************字符串相关处理(多字节)********************/
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

	/*************************************文本相关处理******************************/
	/**
	@brief 打开指定文件路径对应的文件

	@param szPath 指向要打开的文件路径
	@param bReadOnly 是否以只读方式打开文件

	@retval 如果成功返回 打开的文件对象指针，否则返回 NULL
	*/
	virtual IFileEx * AV_CALLTYPE FileOpen( const wchar_t* szPath, bool bReadOnly) PURE;
	/**
	@brief 按指定文件路径创建文件

	@param szPath 指向要创建的文件路径

	@retval 如果成功返回 创建的文件对象指针，否则返回 NULL
	*/
	virtual IFileEx * AV_CALLTYPE FileCreate( const wchar_t* szPath) PURE;

	/**
	@brief 创建一个临时文件
	临时文件是一种用于扫描过程临时创建的文件，在文件关闭(调用Dispose)后应该从文件系统
	删除。 

	@retval 如果成功返回 创建的临时文件，否则返回 NULL
	*/
	virtual IFileEx * AV_CALLTYPE FileCreateTemp() PURE;
	/**
	@brief 判断指定文件是否存在
	*/
	virtual bool AV_CALLTYPE FileIsExist(const wchar_t* szPath) PURE;

	/**
	 * 遍历扫描目标中的内容，除文件系统遍历外，还支持特定系统自定义的目标，
	 * 特定系统自定义目标 szFolder 首字母为 ? 
	 */
	virtual IFolder* AV_CALLTYPE FolderEnum( const wchar_t* szFolder, const wchar_t* szFilter) PURE;

	/***********************************模块相关处理****************************/
	/**
	 * @brief 加载一个模块
	 * 
	 *		@param szName 要加载的模块名称（只是模块文件的文件名，不包含所在文件夹、扩展名等
	 */
	virtual void* AV_CALLTYPE ModuleLoad( const wchar_t* szName ) PURE;
	/**
	 * @brief 读取模块中的一个函数
	 */
	virtual void* AV_CALLTYPE ModuleGetFunc(void* hModule, const wchar_t* szName) PURE;
	/**
	 * @brief 卸载一个模块
	 */
	virtual void AV_CALLTYPE ModuleUnload(void* hModule) PURE;

	/*******************************运行处理支持****************************/
	virtual bool AV_CALLTYPE RunTry( void (AV_CALLTYPE *func)(void* pArgs), void* pArgs ) PURE;
	virtual void AV_CALLTYPE RunThrow(uint32_t code, wchar_t const * szInfo) PURE;

	virtual void AV_CALLTYPE RunTrace(const wchar_t* const szModule, ETraceLevel level, const wchar_t* const szInfo) PURE;
	virtual void AV_CALLTYPE RunSleep() PURE;

	virtual HLOCK AV_CALLTYPE RunCreateLock() PURE;
	virtual void AV_CALLTYPE RunLock(HLOCK hLock) PURE;
	virtual void AV_CALLTYPE RunUnlock(HLOCK hLock) PURE;
	virtual void AV_CALLTYPE RunDisposeLock(HLOCK hLock) PURE;

	/******************************线程存储函数****************************/

	static const uint32_t TLS_FAILED = (uint32_t)-1;
	virtual uint32_t AV_CALLTYPE TlsAlloc() PURE;
	virtual bool AV_CALLTYPE TlsFree(uint32_t dwTlsIndex) PURE;
	virtual void* AV_CALLTYPE TlsGetValue(uint32_t dwTlsIndex) PURE;
	virtual bool AV_CALLTYPE TlsSetValue(uint32_t dwTlsIndex, void* pTlsValue) PURE;
};


extern "C" IOSBase* AV_CALLTYPE OS_Create();
