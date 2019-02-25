//
// Created by jiangmin on 2017/12/4.
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "UStorageMonitor.h"
#include "UStorageFileMonitor.h"
#include "ExecutableMonitor.h"

typedef void (*ActionProc)(int nFlag, int nType, char *pBuf, int nSize,void * pData);
#define FLAG_PROC 0
#define FLAG_FILE 1

class Monitor{
private:
    pthread_t mThreadUStorageMonitor;
    UStorageMonitor mUStorageMonitor;
    pthread_t mThreadUStorageFileMonitor;
    UStorageFileMonitor mUStorageFileMonitor;
    //pthread_t mThreadExecutableMonitor;
    //ExecutableMonitor mExecutableMonitor;
    ActionProc mProc;
    void * mUsrData;
public:

    static void UStorageMonitorProc(int nFlag,int nType,char * pBuf,int nSize,void * pData){
        //pBuf，挂载后Ｕ盘的路径
        Monitor * pMonitor = (Monitor*)pData;
        if (pMonitor){
            if (nFlag == FLAG_INSERT){
                pMonitor->mUStorageFileMonitor.addStorage(pBuf,nSize);
                //pMonitor->mExecutableMonitor.addWatchNode(pBuf);
            }else if (nFlag == FLAG_REMOVE){
                pMonitor->mUStorageFileMonitor.removeStorage(pBuf,nSize);
                //pMonitor->mExecutableMonitor.removeWatchNode(pBuf);
            }
        }
        printf("UStorageMonitorProc::(%d,%d,%s,%d)\n",nFlag,nType,pBuf,nSize);
    }

    static void ExecutableMonitorProc(int nFlag,const char * pPID,const char * pPath,const char * pCmd,void * pData){
        //判断pPath是否在Ｕ盘，就可以决定是否要报告
        printf("ExecutableMonitorProc::(%d,%s,%s,%s)\n",nFlag,pPID,pPath,pCmd);
        Monitor * pMonitor = (Monitor *)pData;
        if (pMonitor){
            pMonitor->mProc(FLAG_PROC,nFlag,(char *)pPID,strlen(pPID),pMonitor->mUsrData);
        }
    }

    static void UStorageFileMonitorProc(EventType eventType,const char * pNewPath,const char * pOldPath,void * pUsrData){
        //判断pNewPath，就可以决定是否要报告
        printf("UStorageFileMonitorProc::(%d,%s,%s)\n",eventType,pNewPath,pOldPath);
        Monitor * pMonitor = (Monitor*)pUsrData;
        if (pMonitor){
            pMonitor->mProc(FLAG_FILE,eventType,(char *)pNewPath,strlen(pNewPath),pMonitor->mUsrData);
        }
    }

    int begin(ActionProc pProc,void * pUsrdata){

        //printf("\n#################### + USB_MONITOR ####################\n");

        mProc = pProc;
        mUsrData = pUsrdata;
        mThreadUStorageMonitor = mUStorageMonitor.beginMonitor(UStorageMonitorProc,this);
        //mThreadExecutableMonitor = mExecutableMonitor.BeginMonitor(ExecutableMonitorProc,this);

        //printf("\n#################### - USB_MONITOR ####################\n");
        return 0;
    }
    int end(){
        mUStorageMonitor.endMonitor();
        //mExecutableMonitor.EndMonitor();
        mUStorageFileMonitor.EndMonitor();
        if (mThreadUStorageMonitor != 0){
            pthread_join(mThreadUStorageMonitor,NULL);
        }
        /*
        if (mThreadExecutableMonitor != -1){
            pthread_join(mThreadExecutableMonitor,NULL);
        }
        */
        if (mThreadUStorageFileMonitor != 0){
            pthread_join(mThreadUStorageFileMonitor,NULL);
        }
        return 0;
    }
};



