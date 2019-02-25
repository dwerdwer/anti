#include "interface_scaner.h"
#include "VirusScaner.h"
#include <sys/time.h>
#include <string.h>
#include "utils/utils_library.h"
#include "module_data.h"
#include "module_defines.h"
#include "module_message_defines.h"
#include "KVMessage.h"
#include "Json.h"

#define REPORTER_TAG "MSG_TYPE_REPORTER"

struct module {
    uint32_t uCategory;
    VirusScaner *pWorker;
    notify_scheduler_t pNotify;
    void *pParams;
    int nToken;
    char *pTag[1];
};


long getCurrentTime() {
    struct timeval timeValue;
    gettimeofday(&timeValue, NULL);
    return ((long) (timeValue.tv_sec * 1000 + timeValue.tv_usec / 1000));
}

void NotifyProc(const int nFlag, const char *pPath, const char *pName, int nId, void *pUsrData) 
{
    char *filter_str = (char*)pUsrData;

    if(0 == strcmp(filter_str, "ScanAll"))
    {
        // TODO: scan what ?
    }
    printf("\n > %d %s\n", nFlag, pPath);

    if (nFlag >= 0 && nFlag < 5) {

        //发送protobuf数据给上报模块
        if (pUsrData) {

            //病毒信息生成protobuf
            char msgString[4096] = { 0 };
            
            int currentTime = (int) (getCurrentTime() / 1000);
            std::string fmtString = "";
            fmtString.append("{\"node_file_virus\":{\"date\":%ld,\"find_time\":%ld,\"filepath\":\"%s\",");
            fmtString.append("\"virus_name\":\"%s\",\"vlib_version\":%ld,\"virus_findby\":%d,\"virus_op\":%d},");
            fmtString.append("\"samps\":{\"virus_name\":\"%s\",\"vlib_version\":%ld,\"samp_md5\":\"this is md5_%d\",");
            fmtString.append("\"samp_sha1\":\"this is sha1_%d\",\"samp_filetime\":%ld,\"samp_size\":%d,");
            fmtString.append("\"store_path\":\"%s\"},\"virus_type\":\"病毒分类\",\"virus_features\":\"病毒特征\"}");
            sprintf(msgString, fmtString.c_str(), currentTime, currentTime, pPath, "xxx.xx",
                    currentTime, 1, 10, "xxx.xx", currentTime, 0, 0, currentTime, 952700, "./tmp");
           
            IRequestMessage *msg = NewInstance_IRequestMessage(CMD_COMMON_REPORT, SUBCMD_COMMON_REPORT);
            msg->Add_StringValue(KEY_REPORT_TYPE, "VirusLog");
            msg->Add_StringValue(KEY_REPORT_ACTION, "findvirus");
            msg->Add_StringValue(KEY_REPORT_INFO, msgString);
            std::string encodeData = msg->Encode();

            module *pModule = (module *) pUsrData;
            module_data_t *pData = create_module_data();

            set_module_data_property(pData, g_p_message_id, REPORTER_TAG, strlen(REPORTER_TAG));
            set_module_data_property(pData, "REPORTER_MESSAGE_DATA", (const char *) encodeData.c_str(), encodeData.size());
            
            module_message_t module_msg;
            module_msg.p_data = pData;
            module_msg.category = (module_category_t)pModule->uCategory;

            mdh_sync_params_t syncData;
            syncData.is_sync = false;

            pModule->pNotify(&module_msg, pModule->pParams, &syncData);

            destroy_module_data(pData);
        }
    }
}


LIB_PUBLIC module_t *create(uint32_t category, notify_scheduler_t notifier, void *p_params, uint32_t arg_count, const char **p_args) {
    module *pModule = new module;
    if (pModule) {

        printf("$ data: %d   %s\n", arg_count, p_args[0]);
        //Set_Token(1072);
        pModule->uCategory = category;
        pModule->pWorker = new VirusScaner();
        pModule->pNotify = notifier;
        pModule->pParams = p_params;

        pModule->pTag[0] = new char[32];
        memset(pModule->pTag[0], 0, 32);
        strcpy(pModule->pTag[0], REPORTER_TAG);

        if (pModule->pWorker) {
            if (arg_count > 0) {
                pModule->pWorker->config(p_args[0]);
            }
        }
        return pModule;
    }
    return NULL;
}

LIB_PUBLIC void destroy(module_t *p_module) {
    if (p_module) {
        module *pModule = (module *) p_module;
        pModule->pWorker->shut();
        delete pModule->pWorker;
        pModule->pWorker = NULL;
        delete pModule;
        pModule = NULL;
    }
}

LIB_PUBLIC module_state_t run(module_t *p_module) {
    module_state_t tmpState = MODULE_FATAL;
    if (p_module) {
        module *pModule = (module *) p_module;
        pModule->pWorker->run();
        tmpState = MODULE_OK;
    }
    return tmpState;
}

LIB_PUBLIC module_state_t stop(module_t *p_module) {
    module_state_t tmpState = MODULE_FATAL;
    if (p_module) {
        tmpState = MODULE_OK;
    }
    return tmpState;
}

LIB_PUBLIC module_data_t *assign(module_t *p_module, const module_data_t *p_data, bool is_sync) 
{
    printf("$ begin scan......\n");

    //int msgType = -1;
    if (p_module) 
    {
        module *pModule = (module *) p_module;

        const char *message_id;
        uint32_t message_id_len;

        if(0 != get_module_data_property(p_data, g_p_message_id, &message_id, &message_id_len)) 
            return NULL;

        if(0 == strcmp(message_id, "ScanVirus"))
        {
            const char *params_str;
            uint32_t params_str_len;

            if(0 != get_module_data_property(p_data, "TASK_PARAMS", &params_str, &params_str_len))
                return NULL;
            
            Json *p_root = Json_Parse(params_str);
            if (p_root) 
            {
                Json *p_target = Json_GetObjectItem(p_root, "Target");
                if (p_target) 
                {
                    Json *filter = Json_GetObjectItem(p_target, "FileFilter");
                    //添加扫描任务
                    pModule->pWorker->add(filter->valuestring, NotifyProc, pModule, 0);
        //            printf("------filter->valuestring = %s\n", filter->valuestring);
                }
                Json_Delete(p_root);
            }
        }

        if (is_sync)
        {
            module_data_t * data = create_module_data();
            int nBusyFlag = pModule->pWorker->isBusyNow();
            int status_code = 0;
            if (nBusyFlag){
                status_code = 11;
            }else{
                status_code = 0;
            }
            set_module_data_property(data, "CENTER_MESSAGE_TASK_RESULT", (const char*)&status_code, sizeof(status_code));
            return data;
        }
    }
    return NULL;
}

LIB_PUBLIC void get_inputted_message_type(module_t *p_module, const char ***const ppp_inputted_message_types,
                                          uint32_t *p_message_type_count) {
    if (p_module) {
        module *pModule = (module *) p_module;
        *ppp_inputted_message_types = (const char **) pModule->pTag;
        *p_message_type_count = 1;
    }
}


