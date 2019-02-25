#define _CRT_SECURE_NO_WARNINGS
#pragma once

#ifndef USING_AVUTIL
#define USE_MSVC_RT	1
#endif

// #include "../../KVPAV2017/AVE/!KVE/AVScanEP/stdafx.h"

#ifdef USE_MSVC_RT
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <map>
#include <vector>
#include <algorithm>
#endif 

#ifndef USING_AVUTIL
#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef linux
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef nullptr
#define nullptr		NULL
#endif

typedef unsigned int	DWORD;

#endif
#endif

#define IN
#define OUT

#define SECTION_HEAD_SIZE       256 // 节区元素个数
#define MAX_VIR_NAME            64  // 最大病毒名长度

#define VNAME_FILE_MAGIC        0x4b4e5550 // PUNK

typedef unsigned char	BYTE;
typedef unsigned int	int_t;

#define BLOCK_SIZE      32 * 10240 // 内存块大小


/* 文件头 */
struct HeaderInfo
{
	int_t magic;           // VNAME_FILE_MAGIC
	int_t totalSize;       // 文件大小
	int_t eleSum;          // 元素总数
	int_t maxEle;          // 最大元素编号
	int_t maxSec;          // 最大节区索引  从 0 开始
	int_t firstSecOffset;  // 首节的绝对偏移
	int_t baseNo;          // 起始位置
	int_t maxSecSize;      // 最大节的大小  包括节头
};


/* 文件头部的节信息 */
struct SectionInfo
{
	int_t secEndOffset; // 节尾绝对偏移

};

#define MAX_REPEAT_COUNT 31     // 最多记录31字节重复
#define OUTLINE_MAX_LEN  7      // currentLen 为此值时表示病毒名称的首字节为长度字节

/* 节区元素长度信息 */
struct SectionElementLen
{
	BYTE repeatLen : 5;  // 重复数据长度
	BYTE currentLen : 3; // 排除重复数据后的长度

};


/* 实际存储的节区元素 */
struct SectionElementName
{
	BYTE lenByte;     // 长度字节，当 currentLen 为 OUTLINE_MAX_LEN 时有意义
	const char* virusName;  // 病毒名称

};

#ifdef USE_MSVC_RT

/* 生成病毒名索引文件 */
class NameDatBuilder
{
public:
	/* 传入基本索引 */
	NameDatBuilder(long baseNo);

	~NameDatBuilder();

	/* 传入 key 与病毒名 */
	int PutName(long no, const char* pszName);

	/* 保存索引文件   传入文件路径*/
	int SaveToFile(const char* pszDatFilePath);

private:
	HeaderInfo* headerInfo;      // 文件头
	SectionInfo* secInfos;       // 节偏移数组

	SectionElementLen* secEleLens;   // 元素长度数组
	SectionElementName* secEleNames; // 元素名数组

    std::map<long, const char*>VirusMap;  // 病毒名容器
	
	int blockNo;              // 内存块索引
	std::vector<char*> blockArray; // 内存块数组

	int currentSize;   // 当前已经用大小
};

#endif // USE_MSVC_RT


/* 查询病毒名 */
class NameDatReader
{
public:
	NameDatReader();

	~NameDatReader();

	/* 分配缓冲区，读文件信息头 传入文件路径 */
	int Init(const char* pszDatFilePath);

	/* 传出 key 为 no 的病毒名      传入长度为 MAX_VIR_NAME 的 strBuf */
	int GetName(long no, char* strBuf);

	int_t GetBaseNo();
	int_t GetEndNo();

	HeaderInfo* headerInfo;     // 文件头
	SectionInfo* secInfos;      // 节偏移数组

private:

#ifdef USING_AVUTIL
	HLOCK m_hLock;
#else
#ifdef linux
	pthread_mutex_t   mutex_lock;
#endif
#ifdef _WIN32
	CRITICAL_SECTION  mutex_lock;
#endif

#endif
	int lastSecIndex;  // 上一次节索引
	FILE* rfp;         // 读取文件指针
	char* secBuf;      // 节缓存区指针
};


/* 读取所有内容，完整格式存于 newTxtPath */
int Decode_CompressedFile(NameDatReader* nameDatReader, const char* newTxtPath);


/* 读取新txt文件，生成新索引文件 */
int UpdateNameData(OUT const char* newdatPath, const char* datPath, const char* addNameTxtPath);
