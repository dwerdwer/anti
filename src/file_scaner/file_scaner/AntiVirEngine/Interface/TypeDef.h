#pragma once

/**
* �����궨�壺
* ƽ̨�꣬�������ֵ�ǰ�����Ŀ��ƽ̨���������к���������Ŀ�����ϵͳΪ��Ӧ�Ĳ���ϵͳ
*	PLT_WIN32 32λ Windows ƽ̨
*   PLT_WIN64 64λ Windows ƽ̨
*   PLT_LINUX32 32 λ Linux ƽ̨
*   PLT_LINUX64 64 λ Linux ƽ̨
* ���η�ʽ:
*   AV_CALLTYPE
*/

#include <stdint.h>

#ifndef PURE
#define PURE =0
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif


#ifdef _MSC_VER
#ifdef _WIN64
#define  PLT_WIN64
#define  PLT_WIN
#else
#define  PLT_WIN32
#define  PLT_WIN
#endif 
#endif // VC

#ifdef __GNUC__
#ifdef _X86_
#define  PLT_LINUX32
#define  PLT_LINUX
#else
#define  PLT_LINUX64
#define  PLT_LINUX
#endif // _X86_
#endif // GCC


#ifdef PLT_WIN

#define CRUN_CALLTYPE __cdecl // C���п�Ĵ��η�ʽ
#define  AV_CALLTYPE __stdcall 
#define PATH_SPLIT_CHAR  '\\'
#define PATH_SPLIT_STRING L"\\"

#ifdef AVUTIL_EXPORTS
#define AVUTIL_API __declspec(dllexport)
#else
#define AVUTIL_API __declspec(dllimport)
#endif

#endif // PLT_WIN

#ifdef PLT_LINUX

#define CRUN_CALLTYPE __attribute__((__cdecl__)) // C���п�Ĵ��η�ʽ
#define AV_CALLTYPE __attribute__((__stdcall__))
#define PATH_SPLIT_CHAR  '/'
#define PATH_SPLIT_STRING L"/"

#define AVUTIL_API 

#define __cdecl 	 __attribute__((__cdecl__))


#include <stdio.h>
#include <stdlib.h>


#endif // PLT_LINUX


#ifdef __cplusplus
#define  EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#define EXTERN_C  extern "C"

#ifdef PLT_WIN

#ifdef _WIN64
    typedef unsigned __int64 size_t;	// 8 Bytes
#else
    typedef unsigned int     size_t;	// 4 Bytes
#endif // WIN64

__if_not_exists(FILETIME) {
#include  "PshPack4.h"
	typedef struct _FILETIME {
		uint32_t dwLowDateTime;
		uint32_t dwHighDateTime;
	} FILETIME, *PFILETIME;
#include "PopPack.h"
}

#endif // WIN32

#ifdef PLT_LINUX

/*
#ifdef PLT_LINUX64
    typedef unsigned long long  size_t;		// 8 bytes
#else
    typedef unsigned int     size_t;		// 4 bytes
#endif // PLT_LINUX64
*/

typedef long long __int64;		// 8 bytes


#define _IN_
#define _OUT_
#define IN
#define OUT

#define _In_
#define _Out_

#define _In_z_
#define _Inout_z_
#define _Out_z_

#define _In_opt_
#define _Out_opt_

#define _Ret_maybenull_
#define _Post_writable_byte_size_(n)
#define _Inout_updates_(n)
#define _Inout_updates_to_(n,n2)
#define _Out_writes_all_(n)
#define _In_reads_(n)
#define _Out_writes_all_(n)
#define _Null_terminated_

#define nullptr 0

#define FALSE 0

#define _ATL_PACKING	8
#define AtlThrow(x)

#include  "PshPack4.h"
	typedef struct _FILETIME {
		uint32_t dwLowDateTime;
		uint32_t dwHighDateTime;
	} FILETIME, *PFILETIME;
#include "PopPack.h"
#endif // PLT_LINUX

/*
*
*/
class IFileEx {
protected:
	virtual ~IFileEx() {
	}
public:
	/**
	@brief �ļ���λ��ʽ
	*/
	enum ESeekFrom
	{
		seekFromBegin = 0, ///< ���ļ���ʼ
		seekFromCurrent,   ///< �ӵ�ǰλ��
		seekFromEnd,       ///< ���ļ���β
	};

	/**
	* @brief �ļ�׼������Ľ��
	*/
	enum EPrepareForCureResult
	{
		forCureUnknown = 0,
		forCureSuccess = 0x1,
		forCureReboot = 0x2,
		forCureFailed = 0x3,
		forCureIsSafe = 0x4, // ���ļ��ǰ�ȫ�ģ���Ӧ��������
		forCureSubstitute = 0x5,
		forCureCanDelete = 0x6, // �޷���д��ʽ���������������ɾ��������û��������ʧ��ɾ���Ļ������Ե���ɾ������ɾ��
		forCureDelete = 0x7, // Ӧ��ֱ�ӽ���ɾ��
	};

	enum EDeleteResult
	{
		deleteSuccess = 0x1,
		deleteReboot = 0x2,
		deleteFailed = 0x3,
		deleteIsSafe = 0x4,
	};
	static const uint64_t INVALID_SIZE = UINT64_MAX;

	virtual void AV_CALLTYPE Dispose() PURE;

	/**
	@brief ��ȡ�ļ�

	@param buffer ��ȡ�����ڴ滺����
	@param ulByteCount �������Ĵ�С

	@retval ���� ��ȡ���ֽ��������ʧ�ܷ��� 0
	*/
	virtual uint32_t AV_CALLTYPE Read(OUT void* buffer, IN uint32_t ulByteCount) PURE;


	/**
	@brief д�ļ�

	@param buffer Ҫд���ڴ滺����
	@param ulByteCount ��������С

	@retval ����д����ֽ���
	*/
	virtual uint32_t AV_CALLTYPE Write(IN void const* buffer, IN uint32_t ulByteCount) PURE;


	/**
	@brief �����ļ��÷���ƫ��

	����ļ��Ѿ�ɾ�����߸��������ø÷���ʱ����Ҫ���κδ���ֱ�ӷ��ء�INVALID_SIZE ����

	@param qwOffset �������ļ�ƫ����
	@param nFrom ��������ʼλ��

	@retval ���� ��ǰ���ļ�ƫ�ƣ����ʧ�ܷ��� INVALID_SIZE

	@see ESeekFrom
	*/
	virtual uint64_t AV_CALLTYPE Seek(IN int64_t qwOffset, IN ESeekFrom nFrom) PURE;


	/**
	@brief ȡ��ǰ�ļ�ƫ��λ��

	����ļ��Ѿ�ɾ�����߸��������ø÷�����ֱ�ӷ��� INVALID_SIZE

	@retval ���� ��ǰ�ļ�ƫ�ƣ����ʧ�ܷ��� INVALID_SIZE
	*/
	virtual uint64_t AV_CALLTYPE GetPosition() PURE;


	/**
	@brief �����ļ��Ĵ�С

	@param qwNewSize �����ļ���С����Ϊ�Ĵ�С

	@retval ����ɹ����� TRUE�����򷵻� FALSE
	*/
	virtual bool AV_CALLTYPE SetSize(IN uint64_t qwNewSize) PURE;


	/**
	@brief ��ȡ�ļ��Ĵ�С

	@retval �����ļ���С�����ʧ�ܷ��� UINT64_MAX
	*/
	virtual uint64_t AV_CALLTYPE GetSize() PURE;

	/**
	* @brief ��ȡ�ļ�������·��
	*
	*/
	virtual const wchar_t* AV_CALLTYPE GetPath() PURE;

	/**
	@brief ��ȡ�ļ���������

	�÷������ص������ƣ�����������·�������ڷ��ص������Ƶ�ָ�룬�ļ�������
	��֤���ļ�������Ч�ԣ���ָ��ָ���������Ч

	@retval ���ؾ�������
	*/
	virtual const wchar_t* AV_CALLTYPE GetName() PURE;


	/**
	@brief ��ȡ�ļ�����չ����������ͷ�� .

	*/
	virtual const char* AV_CALLTYPE GetExtA() PURE;

	/**
	@brief ��֤���ļ������ǿ�д�ģ��Ա����������

	��ɨ������У�PrepareForCure ����ö�Σ����ʵ��ʱӦ�ý�����Ӧ�Ĵ���
	֧�ֶԸú������ظ�����
	�ú�������ʱ��Ӧ�ñ�֤�ļ�ָ������ú���ʱ��ֵһ�¡�

	@retval ���ؾ�����������ֵ�� EPrepareForCureResult
	*/
	virtual EPrepareForCureResult AV_CALLTYPE PrepareForCure() PURE;

	/**
	@brief ������֪ͨ��Ӧ���ļ�������Ҫɾ��

	����ʵ��ʱ�������ļ��ر�ʱ����ɾ����ͬʱӦ��ע����øú�����Ӧ���ٵ���������ص��ļ�
	������������ܵ���δ֪�����
	����ɾ������ļ�����Ҫ֧�֡�Seek �� GetPosition ��������ʱִ�пղ������ɣ�������
	INVALID_SIZE ��

	@retval ����ö�� EDeleteResult �е�ֵ
	*/
	virtual EDeleteResult AV_CALLTYPE Delete() PURE;

	/**
	@breif ȡ�޸�ʱ��

	��Ҫ����ָ���е���ؼ���
	*/
	virtual bool AV_CALLTYPE GetModifyTime(FILETIME* tm) PURE;
};

#else
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#define EXTERN_C 

#endif

#define MAX_VIRNAME_LEN		64
