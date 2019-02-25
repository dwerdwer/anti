//
// Created by jiangmin on 2017/12/3.
//

#ifndef MONITOR_USB_EXECUTABLEMONITOR_H
#define MONITOR_USB_EXECUTABLEMONITOR_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <time.h>
#include "Mutex.h"


#define MaxNameLength 32
#define MaxItemLength 32768
#define MonitorPath "/proc"

#define STATE_CREATE 1
#define STATE_SHUT 0

struct AppInfo {
    std::string localPath;
    std::string cmdLine;
};

struct Cache {
    int length;
    char data[MaxItemLength][MaxNameLength];
};

typedef void (*FileNotify)(int nFlag, const char *pPID, const char *pPath, const char *pCmd, void *pData);

class ExecutableMonitor {
public:
    ExecutableMonitor() {
        mFlag = 0;
        mPtrNofify = NULL;
        mPtrUsrData = NULL;
        mWatchVec.clear();
    }

    ~ExecutableMonitor() {
    }

    pthread_t BeginMonitor(FileNotify pNotify, void *pUsrData) {
        mPtrNofify = pNotify;
        mPtrUsrData = pUsrData;
        pthread_t monitorThread;
        if (pthread_create(&monitorThread, NULL, ExecutableMonitor::RunMonitor, this) == 0) {
            return monitorThread;
        }
        return -1;
    }

    int EndMonitor() {
        mFlag = 0;
        return 0;
    }

    static int GetAppInformation(std::string pID, AppInfo &infoRef) {
        infoRef.cmdLine = "";
        infoRef.localPath = "";
        std::string pidString = "";
        pidString.append(MonitorPath);
        pidString.append("/");
        pidString.append(pID);
        if (access(pidString.c_str(), 0) != -1) {
            std::string tmpString = pidString + "/exe";
            char pathBuffer[1024] = { 0 };
            
            //local path
            size_t length = readlink(tmpString.c_str(), pathBuffer, sizeof(pathBuffer) - 1);
            if (length > 0 && length < sizeof(pathBuffer)) {
                infoRef.localPath = pathBuffer;
            }
            //command line
            tmpString = pidString + "/cmdline";
            std::ifstream inCmdLine(tmpString.c_str(), std::ios::in);
            if (inCmdLine) {
                if (getline(inCmdLine, tmpString)) {
                    if (tmpString.length() > 0) {
                        infoRef.cmdLine = tmpString;
                    }
                }
                inCmdLine.close();
            }
        }
        if (infoRef.cmdLine.length() > 0 || infoRef.localPath.length() > 0) {
            return 1;
        } else {
            return 0;
        }
    }

    static void *RunMonitor(void *pPtr) {

        ExecutableMonitor *pExecutableMonitor = (ExecutableMonitor *) pPtr;
        if (pExecutableMonitor) {
            printf(">>> ExecutableMonitor::Start thread RunMonitor......\n");
            std::string tmpString = MonitorPath;
            struct stat tmpStat;
            lstat(tmpString.c_str(), &tmpStat);
            if (S_ISDIR(tmpStat.st_mode)) {
                Cache *startCache = new Cache();
                Cache *shutCache = new Cache();
                Cache *tmpCache = new Cache();
                Cache *mainCache = new Cache();
                memset(tmpCache, 0, sizeof(Cache));
                DIR *ptrDir = NULL;
                dirent *tmpPath = NULL;
                AppInfo tmpAppInfo;
                int tmpFlag = 0;
                pExecutableMonitor->mFlag = 1;

                //for debug
                //static time_t debugFlag = 0;

                while (pExecutableMonitor->mFlag) {

                    usleep(500000);

                    ptrDir = opendir(tmpString.c_str());
                    if (ptrDir) {
                        tmpPath = NULL;
                        memset(mainCache, 0, sizeof(Cache));
                        while ((tmpPath = readdir(ptrDir))) {
                            if (tmpPath->d_type == DT_DIR) {
                                if (mainCache->length < MaxItemLength && strlen(tmpPath->d_name) < MaxNameLength) {
                                    strcpy(mainCache->data[mainCache->length], tmpPath->d_name);
                                    mainCache->length = mainCache->length + 1;
                                }
                            }
                        }
                        closedir(ptrDir);
                        ptrDir = NULL;
                        if (tmpCache->length > 0 && mainCache->length > 0) {
                            memset(startCache, 0, sizeof(Cache));
                            memset(shutCache, 0, sizeof(Cache));
                            for (int newIndex = 0; newIndex < mainCache->length; newIndex++) {
                                tmpFlag = 0;
                                for (int oldIndex = 0; (oldIndex < tmpCache->length && tmpFlag == 0); oldIndex++) {
                                    if (strcmp(mainCache->data[newIndex], tmpCache->data[oldIndex]) == 0) {
                                        tmpFlag = 1;
                                    }
                                }
                                if (tmpFlag == 0) {
                                    strcpy(startCache->data[startCache->length], mainCache->data[newIndex]);
                                    startCache->length = startCache->length + 1;
                                }
                            }
                            for (int oldIndex = 0; oldIndex < tmpCache->length; oldIndex++) {
                                tmpFlag = 0;
                                for (int newIndex = 0; (newIndex < mainCache->length && tmpFlag == 0); newIndex++) {
                                    if (strcmp(mainCache->data[newIndex], tmpCache->data[oldIndex]) == 0) {
                                        tmpFlag = 1;
                                    }
                                }
                                if (tmpFlag == 0) {
                                    strcpy(shutCache->data[shutCache->length], tmpCache->data[oldIndex]);
                                    shutCache->length = shutCache->length + 1;
                                }
                            }
                            for (int index = 0; index < startCache->length; index++) {
                                if (pExecutableMonitor->isWatchObject(shutCache->data[index])) {
                                    if (pExecutableMonitor->mPtrNofify) {
                                        GetAppInformation(startCache->data[index], tmpAppInfo);
                                        pExecutableMonitor->mPtrNofify(STATE_CREATE, startCache->data[index],
                                                                       tmpAppInfo.localPath.c_str(),
                                                                       tmpAppInfo.cmdLine.c_str(),
                                                                       pExecutableMonitor->mPtrUsrData);
                                    }
                                }
                            }
                            for (int index = 0; index < shutCache->length; index++) {
                                if (pExecutableMonitor->isWatchObject(shutCache->data[index])) {
                                    if (pExecutableMonitor->mPtrNofify) {
                                        pExecutableMonitor->mPtrNofify(STATE_SHUT, shutCache->data[index], "", "",
                                                                       pExecutableMonitor->mPtrUsrData);
                                    }
                                }
                            }
                        }
                        memcpy(tmpCache, mainCache, sizeof(Cache));
                    }

                    //usleep(500000);

                    //for debug
                    /*
                    if (time(NULL) >= debugFlag + 5){

                        if (pExecutableMonitor->mPtrNofify) {
                            pExecutableMonitor->mPtrNofify(STATE_CREATE, "9527",
                                                           "/home/monitor",
                                                           "monitor",
                                                           pExecutableMonitor->mPtrUsrData);
                        }
                        debugFlag = time(NULL);
                    }*/

                }
                delete startCache;
                startCache = NULL;
                delete shutCache;
                shutCache = NULL;
                delete tmpCache;
                tmpCache = NULL;
                delete mainCache;
                mainCache = NULL;
            }
        }
        return NULL;
    }

    void addWatchNode(std::string dirString) {
        if (dirString.length() > 0) {
            mMutex.enter();
            int nIn = 0;
            std::string tmpString = "";
            for (size_t index = 0; index < mWatchVec.size(); index++) {
                tmpString = mWatchVec.at(index);
                if (dirString.compare(tmpString) == 0) {
                    nIn = 1;
                    break;
                }
            }
            if (!nIn) {
                mWatchVec.push_back(dirString);
            }
            mMutex.leave();
        }
    }

    int isWatchObject(std::string pathString) {
        int nFlag = 0;
        if (pathString.length() > 0) {
            std::string tmpString = "";
            mMutex.enter();
            for (size_t index = 0; index < mWatchVec.size(); index++) {
                tmpString = mWatchVec.at(index);
                if (startWith((char *) pathString.c_str(), (char *) tmpString.c_str())) {
                    nFlag = 1;
                    break;
                }
            }
            mMutex.leave();
        }
        return nFlag;
    }

    void removeWatchNode(std::string dirString) {
        if (dirString.length() > 0) {
            mMutex.enter();
            std::vector<std::string>::iterator it = mWatchVec.begin();
            while (it != mWatchVec.end()) {
                if (dirString.compare(*it) == 0) {
                    it = mWatchVec.erase(it);
                } else {
                    it++;
                }
            }
            mMutex.leave();
        }
    }

    int startWith(char *pBuf, char *pTag) {
        if (pBuf && pTag && strlen(pBuf) > 0 && strlen(pTag) > 0) {
            char *ptr = strstr(pBuf, pTag);
            if (ptr == pBuf) {
                return 1;
            }
        }
        return 0;
    }

private:
    int mFlag;
    FileNotify mPtrNofify;
    void *mPtrUsrData;
    Mutex mMutex;
    std::vector <std::string> mWatchVec;
};

#endif //MONITOR_USB_EXECUTABLEMONITOR_H
