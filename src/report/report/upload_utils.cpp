
#include <stdio.h>
#include <string.h>

#include "curl/curl.h"
#include "upload_utils.h"
#include "debug_print.h"

int UploadUtils::UploadFile(const char *pAddr, const char *pPath,int nTimeout)
{
    int result = -1;
    if (NULL == pAddr && NULL == pPath)
        return result;

    static const char headBuffer[] = "Expect:";
    //curl_global_init(CURL_GLOBAL_ALL);
    CURL *pCurl = curl_easy_init();
    if (NULL == pCurl) {
        return result;
    }
    struct curl_slist *headerlist = NULL;
    struct curl_httppost *formpost = NULL;
    struct curl_httppost *lastptr = NULL;
    headerlist = curl_slist_append(headerlist, headBuffer);
    
    curl_formadd(&formpost, &lastptr,
                 CURLFORM_COPYNAME, "file", CURLFORM_FILE, pPath,
                 CURLFORM_END);
    curl_formadd(&formpost, &lastptr,
                 CURLFORM_COPYNAME, "submit", CURLFORM_COPYCONTENTS, "Submit",
                 CURLFORM_END);
    curl_easy_setopt(pCurl, CURLOPT_URL, pAddr);
    
    curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(pCurl, CURLOPT_HTTPPOST, formpost);
    curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, nTimeout );
    
    CURLcode curl_res = curl_easy_perform(pCurl);
    
    curl_easy_cleanup(pCurl);
    curl_formfree(formpost);
    curl_slist_free_all(headerlist);
    //curl_global_cleanup();

    result = (int)curl_res;
    return result;
}

int UploadUtils::UploadBuffer(const char *pAddr, const char *pData, int nLength,const char * pName,int nTimeout)
{
    int result = -1;
    if (NULL == pAddr && NULL == pData)
        return result;
 
    CURL *curl = curl_easy_init();
    
    if(NULL == curl)
        return result;

    struct curl_httppost *formpost = NULL;
    struct curl_httppost *lastptr = NULL;

    curl_slist *headerlist = NULL;
    headerlist = curl_slist_append(headerlist, "Expect:");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "file",
                 CURLFORM_BUFFER, pName, CURLFORM_BUFFERPTR, pData,
                    // !!! for compatible must convert to long 
                 CURLFORM_BUFFERLENGTH, (long)nLength, CURLFORM_END);

    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "submit",
                 CURLFORM_COPYCONTENTS, "Submit", CURLFORM_END);
    
    curl_easy_setopt(curl, CURLOPT_URL, pAddr);

    curl_easy_setopt(curl, CURLOPT_TIMEOUT, nTimeout);
    
    //formpost->bufferlength = nLength; // for compatible explicit assignment
   
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost); 

    CURLcode curl_res = curl_easy_perform(curl);

    debug_print("\n%s: curl_easy_perform ret: %d\n", __func__, curl_res); 
    //printf("%s:%d \ncurl_easy_perform error: %s\n---------------------\n", 
    //__FILE__, __LINE__, curl_easy_strerror(curl_res));
    
    result = (int)curl_res;
    
    curl_easy_cleanup(curl);
    curl_formfree(formpost);
    curl_slist_free_all(headerlist);

    return result;
}

int UploadUtils::ZipBuffer(const char *pData, const int nLength,const char* p_filename, char *pBuf, int &nSize)
{
    int result = 0;
    
    if (pBuf && pData) {
        int nBufSize = nLength + 4096;
        char *pArr = new char[nBufSize]();

        HZIP hZip = CreateZip(pArr, (unsigned int) nBufSize, NULL);
        if (hZip) 
        {
            ZRESULT hr = ZipAdd(hZip, p_filename, (void *)pData, nLength);
            if (hr == ZR_OK) 
            {
                char *pAddr = NULL;
                unsigned long nZipSize = 0;
                hr = ZipGetMemory(hZip, (void **) &pAddr, &nZipSize);
                if (hr == ZR_OK) 
                {
                    if ((unsigned int)nSize >= nZipSize) 
                        memcpy(pBuf, pAddr, nZipSize);
                    
                    nSize = nZipSize;
                    result = 1;
                }
            }
            CloseZip(hZip);
        }
        // zip file end	
        delete[] pArr;
        pArr = NULL;
    }
    return result;
}

int UploadUtils::UploadBufferAsZip(const char *pAddr, const char *pData, 
                                   int nLength,const char * pName, const char* pZipName,int nTimeout)
{
    int nSucccess = -1;
    if (pAddr && pData && pName)
    {
        int nBufSize = nLength + 4096;
        //if (ZipBuffer(pData,nLength,NULL,nBufSize))
        {
            char * pBuf = new char[nBufSize];
            if (pBuf){
                if (ZipBuffer(pData,nLength, pName, pBuf,nBufSize))
                    nSucccess = UploadBuffer(pAddr,pBuf,nBufSize,pZipName,nTimeout);
                
                delete [] pBuf;
                pBuf = NULL;
            }
        }
    }
    return nSucccess;
}

int UploadUtils::UploadSysLog(const char* pServerUrl, const char* pLogData, uint64_t uDataSize, uint32_t iTimeOut)
{
    int result = -1;
    if (pServerUrl && pLogData)
    {
        char headBuffer[] = "content-type: text/plain";
        //curl_global_init(CURL_GLOBAL_ALL);
        CURL *pCurl = curl_easy_init();
        if (pCurl)
        {
            struct curl_slist *headerlist = NULL;
            struct curl_httppost *formpost = NULL;
            headerlist = curl_slist_append(headerlist, headBuffer);

            curl_easy_setopt(pCurl, CURLOPT_URL, pServerUrl);
            curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, headerlist);

            curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, pLogData);
            curl_easy_setopt(pCurl, CURLOPT_POSTFIELDSIZE, uDataSize);

            curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, iTimeOut );
            CURLcode curl_res = curl_easy_perform(pCurl);

            result = (int)curl_res;
            curl_easy_cleanup(pCurl);
            curl_formfree(formpost);
            curl_slist_free_all(headerlist);
        }
    }
    return result;
}

