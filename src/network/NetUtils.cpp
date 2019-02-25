//#include "NetUtils.h"
#include "Net.h"

int reqSync(const char * pBufHost,const char * pDataBuf, int nDataLength, int nReqTarget,ReqDataSync * pData){
        if (pBufHost && pDataBuf && pData){
        Net * pNet = Net::getInstance();
        if (pNet){
			 return pNet->reqSync(pBufHost,pDataBuf,nDataLength,nReqTarget,pData,USE_HTTP);
        }
    }
    return -1;

}

extern "C" int doReq(const char * pBufHost, const char* p_uuid, const char * pDataBuf, int nDataLength, int nReqTarget,Stub pStub,void * pUsrData){
    if (pBufHost && pDataBuf && pStub){
        Net * pNet = Net::getInstance();
        if (pNet){
			 pNet->doReq(pBufHost, p_uuid, pDataBuf,nDataLength,nReqTarget,pStub,pUsrData,USE_HTTP);
            return 1;
        }
    }
    return 0;
}

int doUpload(const char * pBufHost,const char * pLocalPath, int nReqTarget,Stub pStub,void * pUsrData){
	    if (pBufHost && pLocalPath && pStub){
        Net * pNet = Net::getInstance();
        if (pNet){
			 pNet->doUpload(pBufHost,pLocalPath,nReqTarget,pStub,pUsrData,USE_HTTP);
            return 1;
        }
    }
    return 0;
}
