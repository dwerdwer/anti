

#ifndef _HTTP_
#define _HTTP_

#include "Req.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <stdint.h>
#include <curl/curl.h>

#define MAXHOSTSIZE	1024
#define MAXBUFSIZE 	1024*1024
#define HTTP_GET 		0
#define HTTP_POST 		1
#define KV_TRACE_DLL  "libkvtrace.so"
#define FUN_TRACE   "trace_log"
#define TRACE(uuid, data, datasize, direct) \
    {  \
        void *handle = NULL; \
        typedef void (*pfn)(const char*, const char*, uint32_t, int); \
        handle = dlopen(KV_TRACE_DLL, RTLD_LAZY); \
        if (handle) { \
            pfn p_trace = NULL;  \
            p_trace = (pfn)dlsym(handle, FUN_TRACE);  \
            if(p_trace) {  \
                p_trace(uuid, data, datasize, direct);  \
            } \
            else  \
            { \
                printf("error !!!!! dlsym\n"); \
            } \
            dlclose(handle); \
        } \
    }

struct ReqNode{
    unsigned long mMax;
    unsigned long mLenght;
    char * mPtr;
};

class Http : public Req {
    public:
        Http(){
            printf("!!!!!!!!!\n");
            curl_global_init(CURL_GLOBAL_DEFAULT);
        }
        ~Http(){
            curl_global_cleanup();
        }
        //int reqSync(const char * pBufHost,const char * pDataBuf, int nDataLength, int nReqTarget,ReqDataSync * pData)
        int reqSync(const char * pBufHost,const char * pDataBuf, int nDataLength, int nReqTarget,ReqDataSync * pData,int nReqFlag){
            if (nReqFlag == HTTP_POST){
                return reqUsePostSync(pBufHost,pDataBuf,nDataLength,nReqTarget,pData,0,0);
            }else{
                //return reqUseGet(pBufHost,pDataBuf,nDataLength,nReqTarget,pStub,pUsrData,0,0);
            }
            return -1;
        }

        void doReq(const char * pBufHost, const char* p_uuid, const char * pDataBuf,const int nDataLength, int nReqTarget,Stub pStub,void * pUsrData,int nReqFlag){
            if (nReqFlag == HTTP_POST){
                reqUsePost(pBufHost, p_uuid, pDataBuf,nDataLength,nReqTarget,pStub,pUsrData,0,0);
            }else{
                reqUseGet(pBufHost,pDataBuf,nDataLength,nReqTarget,pStub,pUsrData,0,0);
            }
        }
        void doUpload(const char * pBufHost,const char * pLocalPath, int nReqTarget,Stub pStub,void * pUsrData){
            doUpload(pBufHost,pLocalPath,nReqTarget,pStub,pUsrData,0,0);
        }
    private:
        static size_t ReadProc(char * pBuf,size_t nSize, size_t nLength,void * pData){
            size_t bufSize = nSize * nLength;
            ReqNode * pNode = (ReqNode *)pData;
            if (pNode){
                if ((pNode->mLenght + (unsigned long)bufSize) < pNode->mMax){
                    memcpy(pNode->mPtr,pBuf,bufSize);
                    pNode->mLenght = pNode->mLenght + (unsigned long)bufSize;
                }
            }
            return bufSize;
        }

        int doUpload(const char * pBufHost,const char * pLocalPath, int nReqTarget,Stub pStub,void * pUsrData,int bCheckPeer,int bCheckHost){
            int bSuccess = 0;
            if (pBufHost  &&  strlen(pBufHost) < MAXHOSTSIZE && pLocalPath &&  strlen(pLocalPath) < MAXHOSTSIZE && pStub){
                //检查文件是否存在
                FILE * pFile= fopen(pLocalPath, "rb");
                if (pFile){
                    struct stat fileInfor;
                    double uploadSpeed;
                    double uploadTime;
                    if(fstat(fileno(pFile), &fileInfor) == 0){
                        ReqNode reqNode;
                        memset(&reqNode,0,sizeof(ReqNode));
                        reqNode.mMax = MAXBUFSIZE;
                        reqNode.mLenght = 0;
                        reqNode.mPtr = new char[reqNode.mMax];
                        if (reqNode.mPtr){
                            memset(reqNode.mPtr,0,reqNode.mMax);
                            CURL * ptrCURL = curl_easy_init();
                            if(ptrCURL) {
                                curl_easy_setopt(ptrCURL, CURLOPT_URL, pBufHost);
                                curl_easy_setopt(ptrCURL, CURLOPT_UPLOAD, 1L);
                                curl_easy_setopt(ptrCURL, CURLOPT_READDATA, pFile);
                                curl_easy_setopt(ptrCURL, CURLOPT_INFILESIZE_LARGE,(curl_off_t)fileInfor.st_size);
                                curl_easy_setopt(ptrCURL, CURLOPT_VERBOSE, 1L);

                                //curl_easy_setopt(ptrCURL, CURLOPT_POSTFIELDS, pDataBuf);
                                //curl_easy_setopt(ptrCURL, CURLOPT_POSTFIELDSIZE, (long)nDataLength);

                                if (!bCheckPeer){
                                    curl_easy_setopt(ptrCURL, CURLOPT_SSL_VERIFYPEER, NULL);
                                }
                                if (!bCheckHost){
                                    curl_easy_setopt(ptrCURL, CURLOPT_SSL_VERIFYHOST, NULL);
                                }



                                curl_easy_setopt(ptrCURL, CURLOPT_WRITEFUNCTION, Http::ReadProc);
                                curl_easy_setopt(ptrCURL, CURLOPT_WRITEDATA, &reqNode);
                                CURLcode tmpCode = curl_easy_perform(ptrCURL);



                                if(tmpCode != CURLE_OK){
                                    memset(reqNode.mPtr,0,reqNode.mMax);
                                    strcpy(reqNode.mPtr,curl_easy_strerror(tmpCode));
                                    reqNode.mLenght = strlen(reqNode.mPtr);
                                    pStub(NET_ERR,reqNode.mPtr,reqNode.mLenght,nReqTarget,pUsrData);
                                }else{
                                    curl_easy_getinfo(ptrCURL, CURLINFO_SPEED_UPLOAD, &uploadSpeed);
                                    curl_easy_getinfo(ptrCURL, CURLINFO_TOTAL_TIME, &uploadTime);

                                    char uploadBuf[64];
                                    memset(&uploadBuf,0,sizeof(uploadBuf));
                                    sprintf(uploadBuf,"\r\n########## Upload information##########\r\nSpeed:%.3f\r\nTime:%.3f\r\n",uploadSpeed,uploadTime);
                                    strcat(reqNode.mPtr,uploadBuf);
                                    reqNode.mLenght = strlen(reqNode.mPtr);

                                    pStub(NET_SUC,reqNode.mPtr,reqNode.mLenght,nReqTarget,pUsrData);
                                }
                                curl_easy_cleanup(ptrCURL);
                                bSuccess = 1;
                            }
                            delete [] reqNode.mPtr;
                            reqNode.mPtr = NULL;
                        }
                    }
                    fclose(pFile);
                }
            }
            return bSuccess;
        }
        int reqUsePostSync(const char * pBufHost,const char * pDataBuf, int nDataLength, int nReqTarget,ReqDataSync * pData,int bCheckPeer,int bCheckHost){
            int bSuccess = -1;
            if (pBufHost && strlen(pBufHost) < MAXHOSTSIZE && pDataBuf && nDataLength < MAXBUFSIZE ){
                ReqNode reqNode;
                memset(&reqNode,0,sizeof(ReqNode));
                reqNode.mMax = MAXBUFSIZE;
                reqNode.mLenght = 0;
                reqNode.mPtr = new char[reqNode.mMax];
                if (reqNode.mPtr){
                    memset(reqNode.mPtr,0,reqNode.mMax);
                    CURL * ptrCURL = curl_easy_init();
                    if(ptrCURL) {
                        curl_easy_setopt(ptrCURL, CURLOPT_URL, pBufHost);
                        curl_easy_setopt(ptrCURL, CURLOPT_POSTFIELDS, pDataBuf);
                        curl_easy_setopt(ptrCURL, CURLOPT_POSTFIELDSIZE, (long)nDataLength);
                        if (!bCheckPeer){
                            curl_easy_setopt(ptrCURL, CURLOPT_SSL_VERIFYPEER, NULL);
                        }
                        if (!bCheckHost){
                            curl_easy_setopt(ptrCURL, CURLOPT_SSL_VERIFYHOST, NULL);
                        }
                        curl_easy_setopt(ptrCURL, CURLOPT_WRITEFUNCTION, Http::ReadProc);
                        curl_easy_setopt(ptrCURL, CURLOPT_WRITEDATA, &reqNode);
                        CURLcode tmpCode = curl_easy_perform(ptrCURL);
                        long errCode=0;
                        curl_easy_getinfo(ptrCURL,CURLINFO_RESPONSE_CODE,&errCode);
                        if(tmpCode == CURLE_OK){
                            bSuccess = (int)(errCode);
                            if (pData){
                                //pData->nFlag = nReqTarget;
                                pData->nLength = reqNode.mLenght;
                                pData->nTarget = nReqTarget;
                                if (pData->nLength  < 4096){
                                    memcpy(pData->pBuf,reqNode.mPtr,pData->nLength );
                                }
                            }
                        }
                        curl_easy_cleanup(ptrCURL);
                    }
                    delete [] reqNode.mPtr;
                    reqNode.mPtr = NULL;
                }
            }
            return bSuccess;
        }

        int reqUsePost(const char * pBufHost, const char* p_uuid, const char * pDataBuf,const int nDataLength, int nReqTarget,Stub pStub,void * pUsrData,int bCheckPeer,int bCheckHost){
            TRACE(p_uuid, pDataBuf, nDataLength, 1);  // add by tpfei 2018/1/30 for add trace log
            int bSuccess = 0;
            if (pBufHost && strlen(pBufHost) < MAXHOSTSIZE && pDataBuf && nDataLength < MAXBUFSIZE && pStub){
                ReqNode reqNode;
                memset(&reqNode,0,sizeof(ReqNode));
                reqNode.mMax = MAXBUFSIZE;
                reqNode.mLenght = 0;
                reqNode.mPtr = new char[reqNode.mMax];
                if (reqNode.mPtr){
                    memset(reqNode.mPtr,0,reqNode.mMax);
                    CURL * ptrCURL = curl_easy_init();
                    if(ptrCURL) {
                        curl_easy_setopt(ptrCURL, CURLOPT_URL, pBufHost);
                        curl_easy_setopt(ptrCURL, CURLOPT_POSTFIELDS, pDataBuf);
                        curl_easy_setopt(ptrCURL, CURLOPT_POSTFIELDSIZE, (long)nDataLength);
                        if (!bCheckPeer){
                            curl_easy_setopt(ptrCURL, CURLOPT_SSL_VERIFYPEER, NULL);
                        }
                        if (!bCheckHost){
                            curl_easy_setopt(ptrCURL, CURLOPT_SSL_VERIFYHOST, NULL);
                        }
                        curl_easy_setopt(ptrCURL, CURLOPT_WRITEFUNCTION, Http::ReadProc);
                        curl_easy_setopt(ptrCURL, CURLOPT_WRITEDATA, &reqNode);
                        CURLcode tmpCode = curl_easy_perform(ptrCURL);
                        long errCode=0;
                        curl_easy_getinfo(ptrCURL,CURLINFO_RESPONSE_CODE,&errCode);
                        if(tmpCode != CURLE_OK){
                            memset(reqNode.mPtr,0,reqNode.mMax);
                            strcpy(reqNode.mPtr,curl_easy_strerror(tmpCode));
                            reqNode.mLenght = strlen(reqNode.mPtr);
                            if (errCode < 1){
                                errCode = -1;
                            }
                            pStub((int)(errCode),reqNode.mPtr,reqNode.mLenght,nReqTarget,pUsrData);
                        }else{
                            pStub((int)(errCode),reqNode.mPtr,reqNode.mLenght,nReqTarget,pUsrData);
                            TRACE(p_uuid, reqNode.mPtr, reqNode.mLenght, 2);
                        }
                        curl_easy_cleanup(ptrCURL);
                        bSuccess = 1;
                    }
                    delete [] reqNode.mPtr;
                    reqNode.mPtr = NULL;
                }
            }
            return bSuccess;
        }
        int reqUseGet(const char * pBufHost,const char * pDataBuf,const int nDataLength, int nReqTarget,Stub pStub,void * pUsrData,int bCheckPeer,int bCheckHost){
            int bSuccess = 0;
            if (pBufHost && strlen(pBufHost) < MAXHOSTSIZE && pDataBuf && nDataLength < MAXBUFSIZE && pStub){
                ReqNode reqNode;
                memset(&reqNode,0,sizeof(ReqNode));
                reqNode.mMax = MAXBUFSIZE;
                reqNode.mLenght = 0;
                reqNode.mPtr = new char[reqNode.mMax];
                if (reqNode.mPtr){
                    memset(reqNode.mPtr,0,reqNode.mMax);
                    CURL * ptrCURL = curl_easy_init();
                    if(ptrCURL) {
                        curl_easy_setopt(ptrCURL, CURLOPT_URL, pBufHost);
                        if (!bCheckPeer){
                            curl_easy_setopt(ptrCURL, CURLOPT_SSL_VERIFYPEER, NULL);
                        }
                        if (!bCheckHost){
                            curl_easy_setopt(ptrCURL, CURLOPT_SSL_VERIFYHOST, NULL);
                        }
                        curl_easy_setopt(ptrCURL, CURLOPT_WRITEFUNCTION, Http::ReadProc);
                        curl_easy_setopt(ptrCURL, CURLOPT_WRITEDATA, &reqNode);
                        CURLcode tmpCode = curl_easy_perform(ptrCURL);
                        if(tmpCode != CURLE_OK){
                            memset(reqNode.mPtr,0,reqNode.mMax);
                            strcpy(reqNode.mPtr,curl_easy_strerror(tmpCode));
                            reqNode.mLenght = strlen(reqNode.mPtr);
                            pStub(NET_ERR,reqNode.mPtr,reqNode.mLenght,nReqTarget,pUsrData);
                        }else{
                            pStub(NET_SUC,reqNode.mPtr,reqNode.mLenght,nReqTarget,pUsrData);
                        }
                        curl_easy_cleanup(ptrCURL);
                        bSuccess = 1;
                    }
                    delete [] reqNode.mPtr;
                    reqNode.mPtr = NULL;
                }
            }
            return bSuccess;
        }
};

#endif



