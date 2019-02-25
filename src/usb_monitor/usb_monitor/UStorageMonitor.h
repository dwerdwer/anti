//
// Created by jiangmin on 2017/12/3.
//

#ifndef MONITOR_USB_USTORAGEMONITOR_H
#define MONITOR_USB_USTORAGEMONITOR_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <dirent.h>
#include <sys/stat.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/inotify.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <queue>

#define UEVENT_BUFFER_SIZE 4096
#define FLAG_REMOVE 0
#define FLAG_INSERT 1
#define TYPE_DISK 0
#define TYPE_PARTITION 1

typedef void (*UDevProc)(int nFlag, int nType, char *pBuf, int nSize,void * pData);

typedef enum {
    Event_NewDirectory = 0,
    Event_RemoveDirectory = 1,
    Event_NewFile = 2,
    Event_RemoveFile = 3,
    Event_ModifyFile = 4,
    Event_MoveFileOrDirectofy = 5
} EventType;

struct MonitorData {
    int mFlag;
    char mBuf[UEVENT_BUFFER_SIZE];
    UDevProc mProc;
    void *mData;
};

struct UDevNode {
    char mTarget[512];
    char mValue[512];
    int mMount;
};

class UStorageMonitor{
public:
    UStorageMonitor(){
        mFlag = 0;
        mProc = NULL;
        mPtr = NULL;
    }
    ~UStorageMonitor(){

    }
    static void * udevMonitor(void *pData) 
    {
        void *result = NULL;
        if (NULL == pData) {
            return result;
        }
        UStorageMonitor *pMonitor = (UStorageMonitor *) pData;
        //printf(">>> UStorageMonitor::Start thread udevMonitor......\n");
        struct sockaddr_nl monitorAddr;
        memset(&monitorAddr, 0, sizeof(monitorAddr));

        monitorAddr.nl_family = AF_NETLINK;
        monitorAddr.nl_groups = NETLINK_KOBJECT_UEVENT;
        monitorAddr.nl_pid = 0;
        int monitorSocket = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
        if (-1 == monitorSocket) {
#ifdef _DEBUG
            perror("udevMonitor: socket error");
#endif
            return NULL;
        }
        if (-1 == bind(monitorSocket, (struct sockaddr *) &monitorAddr, sizeof(monitorAddr))) {
#ifdef _DEBUG
            perror("udevMonitor: bind error");
#endif
        }
        std::vector <UDevNode *> vecDev;
        pMonitor->mFlag = 1;
        
        while (pMonitor->mFlag) 
        {
            usleep(500000); // 0.5s
            
            char mbsBuf[4096] = { 0 };
            
            struct msghdr tmpMsgHdr;
            memset(&tmpMsgHdr, 0, sizeof(struct msghdr));
            
            struct iovec tmpIovec;
            memset(&tmpIovec, 0, sizeof(tmpIovec));
            
            tmpIovec.iov_base = (void *) mbsBuf;
            tmpIovec.iov_len = sizeof(mbsBuf);
            
            tmpMsgHdr.msg_name = (void *) &monitorAddr;
            tmpMsgHdr.msg_namelen = sizeof(monitorAddr);
            tmpMsgHdr.msg_iov = &tmpIovec;
            tmpMsgHdr.msg_iovlen = 1;
            
            size_t length = (size_t)recvmsg(monitorSocket, &tmpMsgHdr, MSG_DONTWAIT);
            
            if (length > 0 && length < sizeof(mbsBuf)) {
                for (size_t index = 0; index < length; index++) {
                    if (mbsBuf[index] == '\0') {
                        mbsBuf[index] = '\n';
                    }
                }
                
                if (NULL != strstr(mbsBuf, "TAGS=:systemd:")) 
                {
                    if (strstr(mbsBuf, "ACTION=add")) {
                        if (strstr(mbsBuf, "DEVTYPE=partition")) {
                            char *pDevName = strstr(mbsBuf, "DEVNAME=");
                            if (pDevName) {
                                pDevName = pDevName + strlen("DEVNAME=");
                                char *pDevType = strstr(pDevName, "DEVTYPE=partition");
                                if (pDevType) {
                                    int nIndex = strlen(pDevName) - strlen(pDevType);
                                    if (nIndex > 0 && nIndex < 1024) {
                                        char mbsDevBuf[1024] = { 0 };
                                        strncpy(mbsDevBuf, pDevName, nIndex);
                                        mbsDevBuf[strlen(mbsDevBuf) - 1] = '\0';
                                        //debug_print("\nINSERT PARTITION %s \n", mbsDevBuf);
                                        //pMonitor->mProc(FLAG_INSERT,TYPE_PARTITION,mbsDevBuf,strlen(mbsDevBuf),pMonitor->mPtr);
                                        UDevNode *node = new UDevNode;
                                        if (node) {
                                            memset(node, 0, sizeof(UDevNode));
                                            if (strlen(mbsDevBuf) < sizeof(node->mTarget)) {
                                                strcpy(node->mTarget, mbsDevBuf);
                                                node->mMount = 0;
                                                vecDev.push_back(node);
                                            }
                                        }
                                    }
                                }
                            }
                        } else if (strstr(mbsBuf, "DEVTYPE=disk")) {
                            char *pDevName = strstr(mbsBuf, "DEVNAME=");
                            if (pDevName) {
                                pDevName = pDevName + strlen("DEVNAME=");
                                char *pDevType = strstr(pDevName, "DEVTYPE=disk");
                                if (pDevType) {
                                    int nIndex = strlen(pDevName) - strlen(pDevType);
                                    if (nIndex > 0 && nIndex < 1024) {
                                        char mbsDevBuf[1024] = { 0 };
                                        strncpy(mbsDevBuf, pDevName, nIndex);
                                        mbsDevBuf[strlen(mbsDevBuf) - 1] = '\0';
                                        //printf("\nINSERT DISK %s\n", mbsDevBuf);
                                        //pMonitor->mProc(FLAG_INSERT,TYPE_DISK,mbsDevBuf,strlen(mbsDevBuf),pMonitor->mPtr);
                                    }
                                }
                            }
                        }
                    } else if (strstr(mbsBuf, "ACTION=remove")) {
                        if (strstr(mbsBuf, "DEVTYPE=partition")) {
                            char *pDevName = strstr(mbsBuf, "DEVNAME=");
                            if (pDevName) {
                                pDevName = pDevName + strlen("DEVNAME=");
                                char *pDevType = strstr(pDevName, "DEVTYPE=partition");
                                if (pDevType) {
                                    int nIndex = strlen(pDevName) - strlen(pDevType);
                                    if (nIndex > 0 && nIndex < 1024) {
                                        char mbsDevBuf[1024] = { 0 };
                                        strncpy(mbsDevBuf, pDevName, nIndex);
                                        mbsDevBuf[strlen(mbsDevBuf) - 1] = '\0';
                                        //printf("\nREMOVE PARTITION %s\n", mbsDevBuf);
                                        UDevNode *node;
                                        std::vector<UDevNode *>::iterator it = vecDev.begin();
                                        while (it != vecDev.end()) {
                                            node = *it;
                                            if (strcmp(node->mTarget, mbsDevBuf) == 0) {
                                                if (node->mMount == 1){
                                                    memset(mbsDevBuf, 0, sizeof(mbsDevBuf));
                                                    strcpy(mbsDevBuf,node->mValue);
                                                    pMonitor->mProc(FLAG_REMOVE,TYPE_PARTITION,mbsDevBuf,strlen(mbsDevBuf),pMonitor->mPtr);
                                                }
                                                it = vecDev.erase(it);
                                                delete node;
                                                node = NULL;
                                            } else {
                                                it++;
                                            }
                                        }
                                    }
                                }
                            }
                        } else if (strstr(mbsBuf, "DEVTYPE=disk")) {
                            char *pDevName = strstr(mbsBuf, "DEVNAME=");
                            if (pDevName) {
                                pDevName = pDevName + strlen("DEVNAME=");
                                char *pDevType = strstr(pDevName, "DEVTYPE=disk");
                                if (pDevType) {
                                    int nIndex = strlen(pDevName) - strlen(pDevType);
                                    if (nIndex > 0 && nIndex < 1024) {
                                        char mbsDevBuf[1024] = { 0 };
                                        strncpy(mbsDevBuf, pDevName, nIndex);
                                        mbsDevBuf[strlen(mbsDevBuf) - 1] = '\0';
                                        //pMonitor->mProc(FLAG_REMOVE,TYPE_DISK,mbsDevBuf,strlen(mbsDevBuf),pMonitor->mPtr);
                                        //printf("\nREMOVE DISK %s\n", mbsDevBuf);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            //usleep(10000);
            std::string mountString = "";
            FILE *pFile = fopen("/proc/mounts", "r");
            if (pFile) {
                char mbsBuf[1024] = { 0 };
                int nSize = fread(mbsBuf, sizeof(char), sizeof(mbsBuf), pFile);
                while (nSize > 0) {
                    mountString.append(mbsBuf);
                    memset(mbsBuf, 0, sizeof(mbsBuf));
                    nSize = fread(mbsBuf, sizeof(char), sizeof(mbsBuf), pFile);
                }
                fclose(pFile);
            }
            if (mountString.length() > 0) {
                char mbsTarget[128];
                UDevNode *node;
                std::string directoryString = "";
                char *pMountFlag = NULL;
                std::vector<UDevNode *>::iterator iterator;
                for (iterator = vecDev.begin(); iterator != vecDev.end();) {
                    node = *iterator;
                    if (node->mMount == 0) {
                        memset(mbsTarget, 0, sizeof(mbsTarget));
                        strcpy(mbsTarget, node->mTarget);
                        strcat(mbsTarget, " /");
                        pMountFlag = strstr((char *) mountString.c_str(), mbsTarget);
                        if (pMountFlag) {
                            if (getMountPath(pMountFlag, directoryString)) {
                                strcpy(node->mValue, directoryString.c_str());
                                node->mMount = 1;
                                pMonitor->mProc(FLAG_INSERT,TYPE_PARTITION,node->mValue,strlen(node->mValue),pMonitor->mPtr);
                            }
                        }
                    }
                    iterator++;
                }
            }
        }
        close(monitorSocket);
        return NULL;
    }
    static int getMountPath(char *pBuf, std::string &value) {
        int nSuccess = 0;
        if (pBuf) {
            char *pBegin = strstr(pBuf, " /");
            if (pBegin) {
                pBegin = pBegin + strlen(" ");
                char *pEnd = strstr(pBegin, " ");
                if (pEnd) {
                    int length = strlen(pBegin) - strlen(pEnd);
                    if (length > 0 && length < 512) {
                        char mbsBuf[512] = { 0 };
                        strncpy(mbsBuf, pBegin, length);
                        value = mbsBuf;
                        nSuccess = 1;
                    }
                }
            }
        }
        return nSuccess;
    }
    pthread_t beginMonitor(UDevProc pProc,void * pPtr){
        this->mProc = pProc;
        this->mPtr = pPtr;
        pthread_t monitorThread;
        if (pthread_create(&monitorThread, NULL, UStorageMonitor::udevMonitor, this) == 0) {
            return monitorThread;
        }
        return -1;
    }
    void endMonitor(){
        mFlag = 0;
    }
private:
    int mFlag;
    UDevProc mProc;
    void * mPtr;
};

#endif //MONITOR_USB_USTORAGEMONITOR_H
