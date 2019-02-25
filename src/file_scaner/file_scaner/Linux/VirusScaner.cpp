
#include "VirusScaner.h"
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "./Include/ILinuxScan.h"

#define EVENT_SCAN_START 0
#define EVENT_FIND_EXEP 1
#define EVENT_FIND_ARCHIVE 2
#define EVENT_FIND_VIRUS 3
#define EVENT_SCAN_END 4

class ScanNotify : public IScanNotify
{
  public:
    ScanNotify()
    {
        mPtrNotify = NULL;
        mPtrData = NULL;
    }
    virtual ~ScanNotify()
    {
    }

    // 文件扫描开始
    virtual void OnScanStart(const char *filePath)
    {
        //printf("OnScanStart: %s\n", filePath);
        if (mPtrNotify)
        {
            mPtrNotify(EVENT_SCAN_START, filePath, NULL, 0, mPtrData);
        }
    }

    // 脱壳通知
    virtual void OnFindExep(const char *filePath,
                            const char *exepName,
                            unsigned int dwExepRecNo,
                            SCANRESULT OpResult)
    {
        //printf("OnFindExep : %s, %s, %08X, %d\n", filePath, exepName, dwExepRecNo, OpResult);
        if (mPtrNotify)
        {
            mPtrNotify(EVENT_FIND_EXEP, filePath, NULL, 0, mPtrData);
        }
    }

    // 解压通知
    virtual void OnFindArchive(const char *filePath,
                               const char *zipName,
                               unsigned int dwZipRecNo,
                               SCANRESULT OpResult)
    {
        if (mPtrNotify)
        {
            mPtrNotify(EVENT_FIND_ARCHIVE, filePath, zipName, 0, mPtrData);
        }
    }

    // 病毒通知
    virtual void OnFindVirus(const char *filePath,
                             const char *virName,
                             unsigned int dwVirRecNo,
                             SCANRESULT OpResult)
    {
        printf("OnFindVirus: %s, %s, %08X\n", filePath, virName, dwVirRecNo);
        if (mPtrNotify)
        {
            mPtrNotify(EVENT_FIND_VIRUS, filePath, virName, (int)dwVirRecNo, mPtrData);
        }
    }

    // 扫描结束
    virtual void OnScanEnd(const char *filePath)
    {
        //printf("OnScanEnd  : %s\n", filePath);
        if (mPtrNotify)
        {
            mPtrNotify(EVENT_SCAN_END, filePath, NULL, 0, mPtrData);
        }
    }

    void SetStub(Notify pNotify, void *pData)
    {
        mPtrNotify = pNotify;
        mPtrData = pData;
    }

  private:
    Notify mPtrNotify;
    void *mPtrData;
};

VirusScaner::VirusScaner()
{
}
VirusScaner::~VirusScaner()
{
}

WaitNode *VirusScaner::get()
{
    WaitNode *pNode = NULL;
    mMutex.enter();
    if (!mQueSet.empty())
    {
        pNode = mQueSet.front();
        mQueSet.pop();
    }
    mMutex.leave();
    return pNode;
}

int VirusScaner::run()
{

    if (strlen(mConfig) > 0)
    {

        ScanNotify tmpNofify;
        int nError = AV_InitEng(mConfig);
        if (nError >= 0)
        {
    
            WaitNode *pNode = NULL;
            struct stat pathStat;
            char nameBuf[64];
            unsigned int recIndex;
            ILinuxScan *pILinuxScan = NULL;
            time_t timeValue = 0;
            mFlag = 1;
            int nBusy = 0;
            while (mFlag)
            {
                pNode = get();
                if (pNode)
                {
                
                    if (!nBusy){
                        nBusy = 1;
                        mMutex.enter();
                        mBusy = 1;
                        mMutex.leave();
                    }
                    
                    if (lstat(pNode->mPath, &pathStat) >= 0)
                    {
                        if (!S_ISDIR(pathStat.st_mode))
                        {
                            AV_ScanFile(pNode->mPath, MODE_SCAN, nameBuf, &recIndex);
                    
                        }
                        else
                        {
                            tmpNofify.SetStub(pNode->mPtrNotify, pNode->mPtrUsrData);
                            timeValue = time(NULL);
                            const char *tmpDirs[] = {
                                pNode->mPath,
                            };
                            pILinuxScan = NewInstance_ILinuxScan(&tmpNofify);
                            AV_ScanObject(tmpDirs, 1, MODE_SCAN, pILinuxScan);
                            pILinuxScan->Dispose();
                            pILinuxScan = NULL;
                            timeValue = time(NULL) - timeValue;
              
                        }
                    }
                    else
                    {
                
                    }

                    delete pNode;
                    pNode = NULL;
                }
                else
                {
                    if (nBusy){
                        nBusy = 0;
                        mMutex.enter();
                        mBusy = 0;
                        mMutex.leave();
                    }

                    sleep(1);
                }
            }
            while ((pNode = get()) != NULL)
            {
                delete pNode;
                pNode = NULL;
            }
            AV_ClearEng();
        }
    }
    else
    {
        printf("\n> Error:There is not library!\n");
    }
    return 0;
}
int VirusScaner::add(const char *pBuf, Notify pNotify, void *pData, int nFlag)
{
    int nSuc = 0;
    if (pBuf && pNotify)
    {
        WaitNode *pNode = new WaitNode();
        if (pNode != NULL)
        {
            memset(pNode, 0, sizeof(WaitNode));
            strcpy(pNode->mPath, pBuf);
            pNode->mPtrNotify = pNotify;
            pNode->mFlag = nFlag;
            pNode->mPtrUsrData = pData;
            mMutex.enter();
            mQueSet.push(pNode);
            nSuc = 1;
            mMutex.leave();
        }
    }
    return nSuc;
}

int VirusScaner::isBusyNow(){
    int nBusy = -1;
    mMutex.enter();
    nBusy = mBusy;
    mMutex.leave();
    return nBusy;
}   
int VirusScaner::shut()
{
    mFlag = 0;
}

int VirusScaner::config(const char *pPath)
{
    int nSuc = 0;
    if (pPath && (strlen(pPath) < MaxPathLength) && (access(pPath, 0) != -1))
    {
        memset(mConfig, 0, sizeof(mConfig));
        strcpy(mConfig, pPath);
        nSuc = 1;
    }
    return nSuc;
}