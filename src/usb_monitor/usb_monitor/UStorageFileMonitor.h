//
// Created by jiangmin on 2017/12/3.
//

#ifndef MONITOR_USB_USTORAGEFILEMONITOR_H
#define MONITOR_USB_USTORAGEFILEMONITOR_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <vector>
#include <time.h>
#include "Mutex.h"

typedef void (*EventNotify)(EventType evnetType, const char *pNewPath, const char *pOldPath, void *pUsrData);
struct XNode {
    std::string localPath;
    int watchDiscript;
};

class UStorageFileMonitor {
public:
    UStorageFileMonitor(){
        mFlag = 0;
        mVecWatch.clear();
        mVecDirs.clear();
        mFileDiscript = -1;
        mPtrNofify = NULL;
        mPtrUsrData = NULL;
    }
    ~UStorageFileMonitor(){

    }

    void addStorage(char *pBuf, int nLength) {
        //printf(">>> + UStorageFileMonitor:: add director %s\n",pBuf);
        if (pBuf){
            std::vector<std::string> tmpVec = viewDir(pBuf);
            if (tmpVec.size() > 0){
                mMutex.enter();
                std::string tmpString = "";
                int watchDiscript = -1;
                int monitorFlag = IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO | IN_ISDIR;
                for (size_t index=0;index<tmpVec.size();index++){
                    tmpString =tmpVec.at(index);
                    //printf(">>> UStorageFileMonitor:: add director %s\n",tmpString.c_str());
                    watchDiscript = inotify_add_watch(mFileDiscript,tmpString.c_str(),monitorFlag);
                    if (watchDiscript >= 0){
                        XNode node;
                        node.localPath = tmpString;
                        node.watchDiscript = watchDiscript;
                        mVecWatch.push_back(node);
                    }
                }
                mMutex.leave();
            }
        }
        //printf(">>>-  UStorageFileMonitor:: add director %s\n",pBuf);
    }


    void removeStorage(char *pBuf, int nLenght) {
        if (pBuf){
            //printf(">>> + UStorageFileMonitor:: remove director %s\n",pBuf);
            mMutex.enter();
            XNode node;
            std::vector<XNode>::iterator it = mVecWatch.begin();
            while (it != mVecWatch.end()) {
                node = *it;
                if (startWith((char *)node.localPath.c_str(), pBuf)) {
                    //inotify_rm_watch(mFileDiscript,node.watchDiscript);
                    //close(node.watchDiscript);
                    //printf(">>> UStorageFileMonitor:: remove director %s\n",node.localPath.c_str());
                    it = mVecWatch.erase(it);
                } else {
                    it++;
                }
            }
            mMutex.leave();
            //printf(">>> - UStorageFileMonitor:: remove director %s\n",pBuf);
        }
    }

    pthread_t BeginMonitor(std::vector<std::string> dirs,EventNotify pNofify,void * pUsrData) {
        mPtrNofify = pNofify;
        mPtrUsrData = pUsrData;
        for (size_t index=0;index<dirs.size();index++){
            std::vector<std::string> tmpVec = viewDir(dirs.at(index));
            for (size_t nIndex=0;nIndex<tmpVec.size();nIndex++){
                mVecDirs.push_back(tmpVec.at(nIndex));
            }
        }

        pthread_t monitorThread;
        if (pthread_create(&monitorThread, NULL, UStorageFileMonitor::RunMonitor, this) == 0) {
            return monitorThread;
        }
        return -1;
    }

    void EndMonitor() {
        mFlag = 0;
    }

    static void * RunMonitor(void * pPtr){
        UStorageFileMonitor * pMonitor = (UStorageFileMonitor *)pPtr;
        if (pMonitor){
            //printf(">>> UStorageFileMonitor::Start thread RunMonitor......\n");
            if (pMonitor->mVecDirs.size() > 0){
                pMonitor->mFileDiscript = inotify_init();
                if (pMonitor->mFileDiscript >= 0){
                    std::string tmpString = "";
                    int watchDiscript = -1;
                    int monitorFlag = IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO | IN_ISDIR;

                    pMonitor->mMutex.enter();
                    for (size_t index=0;index<pMonitor->mVecDirs.size();index++){
                        tmpString = pMonitor->mVecDirs.at(index);
                        watchDiscript = inotify_add_watch(pMonitor->mFileDiscript,tmpString.c_str(),monitorFlag);
                        if (watchDiscript >= 0){
                            XNode node;
                            node.localPath = tmpString;
                            node.watchDiscript = watchDiscript;
                            pMonitor->mVecWatch.push_back(node);
                        }
                    }
                    pMonitor->mMutex.leave();

                    fd_set monitorTmp;
                    fd_set monitorSet;
                    FD_ZERO(&monitorSet);
                    FD_SET(pMonitor->mFileDiscript, &monitorSet);
                    struct timeval tmpTime;
                    tmpTime.tv_sec = 1;
                    tmpTime.tv_usec = 0;
                    std::string dirPath = "";
                    int length = 0;
                    int readNumber = 0;
                    struct inotify_event *ptrEvent = NULL;
                    char arrBuffer[PATH_MAX] = { 0 };
                    int nSelectFlag = 0;
                    pMonitor->mFlag = 1;

                    //for debug
                    //static time_t debugFlag = 0;

                    while(pMonitor->mFlag)
                    {
                       /* if(time(NULL) >= debugFlag + 5) {
                            //for debug
                            if (pMonitor->mPtrNofify){
                                pMonitor->mPtrNofify(Event_ModifyFile,"/home/application","",pMonitor->mPtrUsrData);
                            }
                            debugFlag = time(NULL);
                        }
                        printf(">>> Select \n");*/
                
                        // sleep 1s
                        usleep(1000000);

                        monitorTmp = monitorSet;
                        nSelectFlag = select(pMonitor->mFileDiscript+1, &monitorTmp, NULL, NULL, &tmpTime);
                        if (nSelectFlag > 0){
                            readNumber = 0;
                            std::string moveFrom = "";
                            memset(arrBuffer,0,PATH_MAX);
                            length = read(pMonitor->mFileDiscript, arrBuffer, sizeof(arrBuffer) - 1);
                            while( length > 0 )
                            {
                                ptrEvent = (struct inotify_event *)&arrBuffer[readNumber];
                                dirPath = pMonitor->getDir(ptrEvent->wd);
                                dirPath.append("/");
                                if (ptrEvent->len > 0){
                                    dirPath.append(ptrEvent->name);
                                }
                                if (ptrEvent->mask == (IN_CREATE | IN_ISDIR)){
                                    watchDiscript = inotify_add_watch(pMonitor->mFileDiscript,dirPath.c_str(),monitorFlag);
                                    if (watchDiscript >= 0){
                                        XNode node;
                                        node.localPath = dirPath;
                                        node.watchDiscript = watchDiscript;
                                        pMonitor->mMutex.enter();
                                        pMonitor->mVecWatch.push_back(node);
                                        pMonitor->mMutex.leave();
                                        if (pMonitor->mPtrNofify){
                                            pMonitor->mPtrNofify(Event_NewDirectory,dirPath.c_str(),"",pMonitor->mPtrUsrData);
                                        }
                                    }
                                }else if (ptrEvent->mask == (IN_DELETE | IN_ISDIR)){
                                    watchDiscript = pMonitor->getDir(dirPath);
                                    if (watchDiscript != -1){
                                        inotify_rm_watch(pMonitor->mFileDiscript,watchDiscript);
                                        pMonitor->removeDir(watchDiscript);
                                    }
                                    if (pMonitor->mPtrNofify){
                                        pMonitor->mPtrNofify(Event_RemoveDirectory,dirPath.c_str(),"",pMonitor->mPtrUsrData);
                                    }
                                }else if (ptrEvent->mask == IN_MODIFY){
                                    if (pMonitor->mPtrNofify){
                                        pMonitor->mPtrNofify(Event_ModifyFile,dirPath.c_str(),"",pMonitor->mPtrUsrData);
                                    }
                                }else if (ptrEvent->mask == IN_DELETE){
                                    if (pMonitor->mPtrNofify){
                                        pMonitor->mPtrNofify(Event_RemoveFile,dirPath.c_str(),"",pMonitor->mPtrUsrData);
                                    }
                                }else if (ptrEvent->mask == IN_MOVED_FROM){
                                    moveFrom = dirPath;
                                }else if (ptrEvent->mask == IN_MOVED_TO){
                                    if (pMonitor->mPtrNofify){
                                        pMonitor->mPtrNofify(Event_MoveFileOrDirectofy,dirPath.c_str(),moveFrom.c_str(),pMonitor->mPtrUsrData);
                                    }
                                }else if (ptrEvent->mask == IN_CREATE){
                                    if (pMonitor->mPtrNofify){
                                        pMonitor->mPtrNofify(Event_NewFile,dirPath.c_str(),"",pMonitor->mPtrUsrData);
                                    }
                                }
                                readNumber = readNumber + sizeof(struct inotify_event) + ptrEvent->len;
                                length = length - sizeof(struct inotify_event) - ptrEvent->len;
                            }
                        }
                    }
                    if (pMonitor->mFileDiscript != -1){
                        pMonitor->mMutex.enter();
                        for (size_t index=0;index<pMonitor->mVecWatch.size();index++){
                            inotify_rm_watch(pMonitor->mFileDiscript,pMonitor->mVecWatch.at(index).watchDiscript);
                            close(pMonitor->mVecWatch.at(index).watchDiscript);
                        }
                        pMonitor->mVecWatch.clear();
                        close(pMonitor->mFileDiscript);
                        pMonitor->mMutex.leave();
                    }
                }
            }
        }
        return NULL;
    }

private:

    int mFlag;
    std::vector<XNode> mVecWatch;
    std::vector<std::string> mVecDirs;
    int mFileDiscript;
    EventNotify mPtrNofify;
    void * mPtrUsrData;
    Mutex mMutex;

    int startWith(char * pBuf, char * pTag) {
        if (pBuf && pTag && strlen(pBuf) > 0 && strlen(pTag) > 0) {
            char * ptr = strstr(pBuf, pTag);
            if (ptr == pBuf) {
                return 1;
            }
        }
        return 0;
    }

    int endWith(const std::string strSrc, const std::string strEnd) {
        if (strSrc.empty() || strEnd.empty()) {
            return 0;
        }
        return strSrc.compare(strSrc.size() - strEnd.size(), strEnd.size(), strEnd) == 0 ? 1 : 0;
    }

    std::string getDir(int watchDiscript) {
        std::string tmpString = "";
        mMutex.enter();
        XNode node;
        for (size_t index = 0; index < mVecWatch.size(); index++) {
            node = mVecWatch.at(index);
            if (node.watchDiscript == watchDiscript) {
                tmpString =  node.localPath;
                break;
            }
        }
        mMutex.leave();
        return tmpString;
    }

    int getDir(std::string localPath) {
        int tmpID = -1;
        mMutex.enter();
        XNode node;
        for (size_t index = 0; index < mVecWatch.size(); index++) {
            node = mVecWatch.at(index);
            if (node.localPath.compare(localPath) == 0) {
                tmpID = node.watchDiscript;
                break;
            }
        }
        mMutex.leave();
        return tmpID;
    }

    int removeDir(int watchDiscript) {
        int nSuc = 0;
        mMutex.enter();
        XNode node;
        for (size_t index = 0; index < mVecWatch.size() && nSuc == 0; index++) {
            node = mVecWatch.at(index);
            if (node.watchDiscript == watchDiscript) {
                std::vector<XNode>::iterator it = mVecWatch.begin() + index;
                mVecWatch.erase(it);
                nSuc = 1;
            }
        }
        mMutex.leave();
        return nSuc;
    }

    std::vector <std::string> viewDir(std::string rootPath) {
        std::vector <std::string> dirVector;
        if (rootPath.length() > 0 && access(rootPath.c_str(), 0) != -1) {
            dirVector.push_back(rootPath);
            if (!endWith(rootPath, "/")) {
                std::queue <std::string> dirQueue;
                struct stat tmpStat;
                dirent *tmpPath = NULL;
                DIR *ptrDir = NULL;
                std::string tmpString = "";
                std::string tmpDirPath = "";
                dirQueue.push(rootPath);
                while (!dirQueue.empty()) {
                    tmpString = dirQueue.front();
                    lstat(tmpString.c_str(), &tmpStat);
                    if (S_ISDIR(tmpStat.st_mode)) {
                        ptrDir = opendir(tmpString.c_str());
                        if (ptrDir) {
                            while ((tmpPath = readdir(ptrDir))) {
                                if (!(strcmp(tmpPath->d_name, ".") == 0 || strcmp(tmpPath->d_name, "..") == 0)) {
                                    if (tmpPath->d_type == DT_DIR) {
                                        tmpDirPath.append(tmpString + "/" + tmpPath->d_name);
                                        dirVector.push_back(tmpDirPath);
                                        dirQueue.push(tmpDirPath);
                                        tmpDirPath = "";
                                    }
                                }
                            }
                            closedir(ptrDir);
                            ptrDir = NULL;
                        }
                    }
                    dirQueue.pop();
                }
            }
        }
        return dirVector;
    }
};

#endif //MONITOR_USB_USTORAGEFILEMONITOR_H
