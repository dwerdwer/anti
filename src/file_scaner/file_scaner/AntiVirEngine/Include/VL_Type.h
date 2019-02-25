#ifndef VL_TYPE_H
#define VL_TYPE_H

typedef unsigned char 	BYTE;
typedef unsigned short 	WORD;
typedef unsigned char 	BYTE;
typedef unsigned int 	UINT;
typedef int 			BOOL;

#ifdef __GNUC__
typedef unsigned int 	DWORD;
typedef int  			LONG;
#else
typedef unsigned long 	DWORD;
typedef long  			LONG;
#endif // __GNUC__

#ifdef _MSC_VER
#ifdef _WIN64
typedef unsigned __int64	UINT_PTR;
#else
typedef unsigned int		UINT_PTR;
#endif 
#endif // VC

#ifdef __GNUC__
#ifdef _X86_
typedef char			__int8;
typedef short			__int16;
typedef unsigned int	UINT_PTR;
#else
typedef unsigned long long	UINT_PTR;
#endif // _X86_
#endif // GCC

#ifdef _WIN32
typedef unsigned __int64 QWORD;
typedef __int64 QLONG;
#endif

typedef BYTE* PBYTE;
typedef WORD* PWORD;
typedef DWORD* PDWORD;
typedef char* PSTR;
typedef short SHORT;
typedef int   AVSIZE;

typedef const char* LPCSTR;

#ifndef VOID
#define VOID void
typedef char CHAR;
typedef short SHORT;
#ifdef __GNUC__
typedef int   LONG;
#else
typedef long  LONG;
#endif // __GNUC__

#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
#define _W64 __w64
#else
#define _W64
#endif
#endif
//typedef _W64 unsigned long ULONG_PTR, *PULONG_PTR;
//typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;

#ifndef __GNUC__
typedef __int64 LONGLONG;
typedef unsigned __int64 ULONGLONG;
#else
typedef __SIZE_TYPE__ size_t;
typedef long long LONGLONG;
typedef unsigned long long ULONGLONG;
#endif

#pragma pack(push,1)

typedef union
{
	BYTE bLo, bHi;
	WORD w;
} CUWORD,*PUWORD;

typedef union
{
	BYTE b1, b2, b3, b4;
	WORD wLo, wHi;
	DWORD dw;
} CUDWORD, *PUDWORD;

#pragma pack(pop)

#endif // !VL_TYPE_H
