
#ifdef USING_AVUTIL
#include <Utils.h>
#else 
#define  _CRT_SECURE_NO_WARNINGS

#ifdef WINDOWS
#else
#endif

#include <stdio.h>

// #include "../../KVPAV2017/AVE/!KVE/AVScanEP/stdafx.h"

#endif

#include "VirusFileAPI.h"


NameDatReader::NameDatReader()
{
	this->rfp = NULL;
	this->secBuf = NULL;

	this->headerInfo = NULL;
	this->secInfos = NULL;

	this->lastSecIndex = -1;
#ifdef USING_AVUTIL
	m_hLock = NULL;
#else

#endif
}

NameDatReader::~NameDatReader()
{
	if (this->rfp) {
	    fclose(this->rfp); this->rfp = NULL;
    }
	if (this->headerInfo) {
		free(this->headerInfo); this->headerInfo = NULL;
    }
	if (this->secInfos) {
		free(this->secInfos); this->secInfos = NULL;
    }
	if (this->secBuf) {
		free(this->secBuf); this->secBuf = NULL;
    }

#ifdef USING_AVUTIL
	if (m_hLock != NULL)
		AVDisposeLock(m_hLock);
#else
#ifdef linux
	pthread_mutex_destroy(&this->mutex_lock);
#endif

#ifdef _WIN32
	DeleteCriticalSection(&this->mutex_lock);
#endif
#endif
}


/* 分配缓冲区，读文件信息头  传入文件路径 */
int NameDatReader::Init(const char* pszDatFilePath)
{
	// 文件头部信息
	if (pszDatFilePath == NULL)
		return -1;

	if( NULL == (this->rfp = fopen(pszDatFilePath, "rb")) )
        return -1;

#ifdef USING_AVUTIL
	m_hLock = AVCreateLock();
#else
#ifdef linux
	pthread_mutex_init(&mutex_lock, NULL);
#endif

#ifdef _WIN32
	InitializeCriticalSection(&mutex_lock);
#endif
#endif

	this->headerInfo = (HeaderInfo*)malloc(sizeof(HeaderInfo));

	if (fread(this->headerInfo, 1, sizeof(HeaderInfo), rfp) == 0)
		return -1;

	if (this->headerInfo->magic != VNAME_FILE_MAGIC)
		return -1;
	// 节偏移
	this->secInfos = (SectionInfo*)malloc(headerInfo->firstSecOffset - sizeof(HeaderInfo));
	if (NULL == secInfos) return -1;

	fseek(rfp, sizeof(HeaderInfo), SEEK_SET);

	if (fread(this->secInfos, 1, headerInfo->firstSecOffset - sizeof(HeaderInfo), rfp) == 0)
		return -1;

	// 最大节缓存区
	this->secBuf = (char*)malloc(headerInfo->maxSecSize);

	memset(secBuf, 0, headerInfo->maxSecSize);

	return 0;
}

int_t NameDatReader::GetBaseNo()
{
	if (headerInfo == NULL)
		return -1;
	return headerInfo->baseNo;
}

int_t NameDatReader::GetEndNo()
{
	if (headerInfo == NULL)
		return -1;
	return headerInfo->maxEle;
}

/* 传出 key 为 no 的病毒名      传入长度为 MAX_VIR_NAME 的 strBuf */
int NameDatReader::GetName(long no, char* strBuf)
{
    int result = 0;

    int strLen = 0;
    int nameLen = 0;

    int secIndex = 0; // 节索引
    int eleIndex = 0; // 元素索引 0 ~ 255

    int_t beginOffset = 0; // 所在区的起始偏移

    int secSize = 0;
    int offset = 0;

    int eleSum = SECTION_HEAD_SIZE; 
    SectionElementLen* secEleLens = NULL;

#ifdef USING_AVUTIL
    CLockLock __lock(m_hLock);
#else

#ifdef linux
    pthread_mutex_lock(&mutex_lock);
#endif

#ifdef _WIN32
    EnterCriticalSection(&mutex_lock);
#endif
#endif

    no -= (long)headerInfo->baseNo;

    if (no >= (long)headerInfo->eleSum || no < 0 || NULL == strBuf)
    {
        if (strBuf) *strBuf = 0;
        result = -1;
        goto FinalEnd;
    }
    // 确定元素范围
    secIndex = no / SECTION_HEAD_SIZE;

    eleIndex = no % SECTION_HEAD_SIZE;

    if (secIndex == 0)
        beginOffset = headerInfo->firstSecOffset;
    else
        beginOffset = secInfos[secIndex - 1].secEndOffset;

    // 尾节可能不足 SECTION_HEAD_SIZE
    if (secIndex == (int)headerInfo->maxSec && headerInfo->eleSum % SECTION_HEAD_SIZE != 0)
        eleSum = headerInfo->eleSum % SECTION_HEAD_SIZE;

    secSize = (int)(secInfos[secIndex].secEndOffset - beginOffset);

    if (this->lastSecIndex != secIndex)
    {
        fseek(rfp, beginOffset, SEEK_SET);

        if (fread(this->secBuf, 1, secSize, rfp) == 0)
        {
            result = -1;
            goto FinalEnd;
        }
        this->lastSecIndex = secIndex;
    }

    // read 
    secEleLens = (SectionElementLen*)secBuf;

    offset = sizeof(SectionElementLen) * eleSum;

    for (int i = 0; i <= eleIndex; i++)
    {
        if (secEleLens[i].currentLen != OUTLINE_MAX_LEN)
        {
            memmove(strBuf + secEleLens[i].repeatLen, secBuf + offset, secEleLens[i].currentLen);

            offset += secEleLens[i].currentLen;
            strLen = secEleLens[i].repeatLen + secEleLens[i].currentLen;
        }
        else // 超长
        {
            nameLen = secBuf[offset];
            memmove(strBuf + secEleLens[i].repeatLen, secBuf + offset + 1, nameLen);

            offset += nameLen + 1;
            strLen = secEleLens[i].repeatLen + nameLen;
        }
    }
    strBuf[strLen] = 0;

FinalEnd:
    // 退出临界区
#ifndef USING_AVUTIL
#ifdef linux
    pthread_mutex_unlock(&this->mutex_lock);
#endif

#ifdef _WIN32
    LeaveCriticalSection(&this->mutex_lock);
#endif
#endif

    return result;
}



#ifdef USE_MSVC_RT

/* 导出 */
int Decode_CompressedFile(NameDatReader* nameDatReader, const char* newTxtPath)
{
	if (!nameDatReader)
		return -1;
	int_t baseNo = nameDatReader->GetBaseNo();

	FILE* wfp = fopen(newTxtPath, "wb");

	if (NULL == wfp)
	{
		perror("fopen error");
		return -1;
	}
	fprintf(wfp, "baseno=0x%X\n\n", baseNo);

	char strBuf[MAX_VIR_NAME] = { 0 };

	int_t i = baseNo;

	while (nameDatReader->GetName((long)i, strBuf) == 0)
	{
		if (strlen(strBuf) != 0)
			fprintf(wfp, "0x%X %s\n", i, strBuf);
		++i;
	}
	fclose(wfp);

	return 0;
}

#endif // USE_MSVC_RT
