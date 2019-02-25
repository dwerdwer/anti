#ifndef UPLOAD_UTILS_H
#define UPLOAD_UTILS_H

#include <stdint.h>
#include <time.h>
#include "zip.h"

#define MAXBUFSIZE 4096

class UploadUtils 
{
public:
	static int UploadFile(const char *pAddr, const char *pPath,int nTimeout);	

    static int UploadBuffer(const char *pAddr, const char *pData, int nLength,const char * pName,int nTimeout);
    
    static int ZipBuffer(const char *pData, const int nLength,const char* p_filename, char *pBuf, int &nSize);

    static int UploadBufferAsZip(const char *pAddr, const char *pData, int nLength,const char * pName, const char* pZipName,int nTimeout);

    static int UploadSysLog(const char* pServerUrl, const char* pLogData, uint64_t uDataSize, uint32_t iTimeOut);
};

#endif
