#pragma once

#include "../../AntiVirEngine/Include/VL_Type.h"
#include <time.h>

#ifndef FILENAME_MAX
#define FILENAME_MAX 260
#endif // FILENAME_MAX

#ifndef MAX_PATH
#define MAX_PATH 4096
#endif // MAX_PATH

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

#ifndef _Null_terminated_
#define _Null_terminated_
#endif // _Null_terminated_

#define far

#define APIENTRY		_stdcall

#define DLL_PROCESS_ATTACH   1    
#define DLL_THREAD_ATTACH    2    
#define DLL_THREAD_DETACH    3    
#define DLL_PROCESS_DETACH   0    


typedef unsigned char 	BYTE;
typedef unsigned short 	WORD;
typedef unsigned int 	DWORD;
typedef unsigned int 	UINT;

typedef int 			BOOL;
typedef int 			LONG;		// 4 bytes

typedef void far *		LPVOID;

typedef long long		__int64;	// 8 bytes


typedef char			CHAR;

typedef _Null_terminated_ CHAR *NPSTR, *LPSTR, *PSTR;



typedef union _LARGE_INTEGER {
	struct {
		DWORD LowPart;
		LONG HighPart;
	} DUMMYSTRUCTNAME;
	struct {
		DWORD LowPart;
		LONG HighPart;
	} u;
	LONGLONG QuadPart;
} LARGE_INTEGER;


#ifdef _USE_32BIT_TIME_T
// #define  _USE_32BIT_TIME_T
typedef long __time32_t;
typedef __time32_t time_t;
#else
// typedef __int64    __time64_t;
#ifndef _BITS_TYPES_H 
typedef __time64_t time_t;
#endif // _BITS_TYPES_H 

typedef __time_t	time_t;

#endif // _CRT_NO_TIME_T


typedef struct _SYSTEMTIME {
	WORD wYear;
	WORD wMonth;
	WORD wDayOfWeek;
	WORD wDay;
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;


#ifndef _ADDRESSOF

#ifdef  __cplusplus
#define _ADDRESSOF(v)   ( &reinterpret_cast<const char &>(v) )
#else
#define _ADDRESSOF(v)   ( &(v) )
#endif

#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )

#endif // _ADDRESSOF

#if  !defined(va_arg) && !defined(_VA_LIST_DEFINED)

typedef char *  va_list;
#define va_start(ap,v)  ( ap = (va_list)_ADDRESSOF(v) + _INTSIZEOF(v) )
#define va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap)      ( ap = (va_list)0 )

#endif // va_arg


#pragma warning( disable: 4996 4200)
