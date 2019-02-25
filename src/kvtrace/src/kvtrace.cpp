#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include "trace_log.h"
#include "IKVMessage.h"
#include "KVMessage.h"

#ifdef linux
#define TRACE_API __attribute ((visibility("default")))
#endif

std::string get_app_dir()
{
    char dir[PATH_MAX] = {0};
    int n =  readlink("/proc/self/exe", dir, PATH_MAX);
    if (n == -1)
        abort();
    else
        dir[n] = '\0';
    char* last_slash = NULL;
    last_slash  = strrchr(dir, '/');
    if (last_slash == NULL || last_slash == dir)
        abort();

    char path[PATH_MAX] = {0};
    int result_len = last_slash - dir;
    strncpy(path, dir, result_len);
    path[result_len] = '\0';
    std::string str(path);
    return str;
}

extern "C" TRACE_API void trace_log(const char* uuid, const char* data, uint32_t data_len, int dirct)
{
    if (!uuid)
        return ;
    else if(0 == strlen(uuid))
        return ;
    std::string log_file = get_app_dir();
    log_file.append("/log/");
    int status;
    status = mkdir(log_file.c_str(), S_IRWXU|S_IRWXG|S_IXOTH);
 /*   if (status == -1)
        return;
 */
    log_file.append(uuid);
    log_file.append(".log");
    init_log(log_file.c_str());

    //dbg_log("%s, %d", "fdkfdkfskdfsdkf", 10);

    char buf[MAX_LOG_LEN] = {0};
    if (dirct == 1)
    {
        //Deocde
        int size = 0;
        IRequestMessage* msg = NewInstance_IRequestMessage((const unsigned char*)data, data_len, &size);
        if (msg)
        {
            int cmd = msg->Get_Cmd();
            int subcmd = msg->Get_SubCmd();
            sprintf(buf, "Client Send\t");
            switch(cmd)
            {
                case 0:
                    {
                        sprintf(buf + strlen(buf), "ECHO CMD");
                    }
                    break;
                case 1:
                    {
                        sprintf(buf + strlen(buf), "CMD HEART_BEAT");
                    }
                    break;
                case 2:
                    {
                        if (subcmd == SUBCMD_INSTALL)
                            sprintf(buf + strlen(buf), "CMD INSTALL");
                        else if (subcmd == SUBCMD_UNINSTALL)
                            sprintf(buf + strlen(buf), "CMD UNINSTALL");
                    }
                    break;
                case 3:
                    {
                        if (subcmd == SUBCMD_LOGIN)
                            sprintf(buf + strlen(buf), "CMD LOGIN");
                        else
                            sprintf(buf + strlen(buf), "CMD LOGOUT");
                    }
                    break;
                case 4:
                    {
                        sprintf(buf + strlen(buf), "CMD_REPORT");
                    }
                    break;
                case 5:
                    {
                        if (subcmd == SUBCMD_COMMON_REPORT)
                            sprintf(buf + strlen(buf), "CMD COMMON_REPORT");
                        else
                            sprintf(buf + strlen(buf), "COMMON_REPORT Unknown subcmd");
                    }
                    break;
                case 10:
                    {
                        sprintf(buf + strlen(buf), "CMD CENTER");
                    }
                    break;
                default:
                    sprintf(buf + strlen(buf), "UNKNOWN CMD");
                    break;
            }
            dbg_log(buf);
            dbg_log(msg->to_String().c_str());
        }
    }
    else
    {
        int size = 0;
        IResponseMessage *msg = NewInstance_IResponseMessage((const unsigned char*)data, data_len, &size);
        if (msg)
        {
            sprintf(buf, "Client Receive\t");
            dbg_log(buf);
            dbg_log(msg->to_String().c_str());
        }
    }
}
