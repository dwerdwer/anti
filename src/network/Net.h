#ifndef _NET_
#define _NET_

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "Req.h"
#include "Http.h"
using namespace std;

#define USE_HTTP 	0
#define USE_TCP 		1
#define USE_CUR 		USE_HTTP
#define USE_REQ 		HTTP_POST

class Net{
public:
    static Net * getInstance(){
        if (!mPtrNet){
            mPtrNet = new Net();
        }
        return mPtrNet;
    }
    int reqSync(const char * pBufHost,const char * pDataBuf, int nDataLength, int nReqTarget,ReqDataSync * pData,int nReqFlag){
        if (mPtrReq){
			if (nReqFlag == USE_HTTP)
			{
				return mPtrReq->reqSync(pBufHost,pDataBuf,nDataLength,nReqTarget,pData,USE_REQ);
			}else{

			}
		}
        return -1;
    }
    void doReq(const char * pBufHost, const char* p_uuid, const char * pDataBuf,const int nDataLength, int nReqTarget,Stub pStub,void * pUsrData,int nReqFlag){
		if (mPtrReq){
			if (nReqFlag == USE_HTTP)
			{
				mPtrReq->doReq(pBufHost,p_uuid, pDataBuf,nDataLength,nReqTarget,pStub,pUsrData,USE_REQ);
			}else{

			}
		}
    }
	void doUpload(const char * pBufHost,const char * pLocalPath, int nReqTarget,Stub pStub,void * pUsrData,int nReqFlag){
		if (mPtrReq){
			if (nReqFlag == USE_HTTP)
			{
				mPtrReq->doUpload(pBufHost,pLocalPath,nReqTarget,pStub,pUsrData);
			}else{

			}
		}
	}
private:
    Net(){
		mReqFlag = USE_CUR;
		if (mReqFlag == USE_HTTP){
			mPtrReq = (Req *)new Http();
		}else{

		}
    }
    ~Net(){
        if (mPtrNet){
            cout<<"Delete pointer"<<endl;
            delete mPtrNet;
            mPtrNet = 0;
        }
    }
    static Net * mPtrNet;
	Req * mPtrReq;
	int mReqFlag;
};
Net * Net::mPtrNet = NULL;

#endif
