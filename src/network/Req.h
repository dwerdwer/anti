#ifndef _REQ_
#define _REQ_

#define NET_ERR 0
#define NET_SUC 1

typedef void (* Stub)(int nFlag,char * pBuf,int nLength,int nTarget,void * pUsrData);

struct ReqDataSync{
        int nFlag;
        char pBuf[4096];
        int nLength;
        int nTarget;
};

class Req{
public:
        virtual int reqSync(const char * pBufHost,const char * pDataBuf, int nDataLength, int nReqTarget,ReqDataSync * pData,int nReqFlag) = 0;
        virtual void doReq(const char * pBufHost, const char * p_uuid, const char * pDataBuf,const int nDataLength, int nReqTarget,Stub pStub,void * pUsrData,int nReqFlag) = 0;
		virtual void doUpload(const char * pBufHost,const char * pLocalPath, int nReqTarget,Stub pStub,void * pUsrData) = 0;
};

#endif
