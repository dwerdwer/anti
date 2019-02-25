
#ifndef _NETUTILS_
#define _NETUTILS_

#include <stdlib.h>
#include <stdio.h>

typedef void (* Stub)(int nFlag,char * pBuf,int nLength,int nTarget,void * pUsrData);

struct ReqDataSync{
        int nFlag;
        char pBuf[4096];
        int nLength;
        int nTarget;
};

extern "C"
{
    int reqSync(const char * pBufHost,const char * pDataBuf, int nDataLength, int nReqTarget,ReqDataSync * pData);
    int doReq(const char * pBufHost, const char* p_uuid, const char * pDataBuf, int nDataLength, int nReqTarget,Stub pStub,void * pUsrData);
	int doUpload(const char * pBufHost,const char * pLocalPath, int nReqTarget,Stub pStub,void * pUsrData);
}

#endif

