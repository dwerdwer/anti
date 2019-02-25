
#ifndef _VIRUSSCANER_
#define _VIRUSSCANER_
#include <iostream>
#include <queue>
#include "Mutex.h"
using namespace std;

typedef void (* Notify)(const int nFlag, const char * pPath,const char * pName,int nId,void * pData);

#define MaxPathLength 512
struct WaitNode{
    char mPath[MaxPathLength];
    Notify mPtrNotify;
    int mFlag;
    void * mPtrUsrData;
    };

class VirusScaner{
public:
    VirusScaner();
    
    ~VirusScaner();
    int run();
    int add(const char * pBuf,Notify pNotify,void * pData,int nFlag);
    int shut();
    int config(const char * pPath);
    int isBusyNow();
private:
    WaitNode * get();
    queue<WaitNode*> mQueSet;
    int mFlag;
    int mBusy;
    Mutex mMutex;
    char mConfig[MaxPathLength];
    };
    
#endif
