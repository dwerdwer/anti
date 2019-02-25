//
// Created by jiangmin on 2017/12/5.
//

#include "interface.h"
#include "Monitor.h"
#include "module_data.h"
#include "json/json.h"
#include "utils/utils_library.h"
#include "USBMonitorDef.h"
#include "module_interfaces.h"
#include "module_message_defines.h"

#define USB_MODULE_TAG "USBSTORAGE_EVENT"

struct usb_monitor_config_data
{
    int auto_upload;
    int max_size;
    int file_filter[64];
};

struct module
{
    uint32_t uCategory;
    Monitor *pMonitor;
    notify_scheduler_t pNotify;
    void *pParams;
    usb_monitor_config_data usbConfig;
    char * pTag[1];
};

void ActionFileProc(int nFlag, int nType, char *pBuf, int nSize, void *pData)
{
    printf("Action:flag=%d type=%d,data=%s,size=%d", nFlag, nType, pBuf, nSize);
    module *pModule = (module *)pData;
    if (pModule)
    {
		module_data_t  * pData =  create_module_data();
        set_module_data_property(pData,g_p_message_id, "UStorgae_Event", strlen("UStorgae_Event"));
        int nLength = 1;
        // TODO: debug
        set_module_data_property(pData, "UStorgae_Event_Num", (const char *)&nLength, sizeof(int));
        if (nFlag == FLAG_PROC)
        {
            //进程
            int nEventFlag = nType;
            set_module_data_property(pData, "UStorage_Flag", (const char *)&nEventFlag, sizeof(int));
            set_module_data_property(pData, "UStorage_Data", (const char *)pBuf, strlen(pBuf));
        }
        else if (nFlag == FLAG_FILE)
        {
            //文件
            int nEventFlag = nType;
            set_module_data_property(pData,"UStorage_Flag", (const char *)&nEventFlag, sizeof(int));
            set_module_data_property(pData, "UStorage_Data", (const char *)pBuf, strlen(pBuf));
        }


		module_message_t module_msg;
		module_msg.p_data = pData;
		module_msg.category = (module_category_t)pModule->uCategory;


        mdh_sync_params_t syncData;
        syncData.is_sync = false;
        pModule->pNotify(&module_msg, pModule->pParams, &syncData);
    }
}

LIB_PUBLIC module_t *create(uint32_t category, notify_scheduler_t notifier, void *p_params, uint32_t arg_count, const char **p_args)
{
    module_t *pModule = new module_t;
    if (pModule)
    {
        pModule->pMonitor = new Monitor;
        pModule->pNotify = notifier;
        pModule->uCategory = category;
        pModule->pParams = p_params;

		pModule->pTag[0] = new char[32];
		memset(pModule->pTag[0],0,32);
		strcpy(pModule->pTag[0],USB_MODULE_TAG);

        return pModule;
    }
    return NULL;
}

LIB_PUBLIC void destroy(module_t *p_module)
{
    if (p_module)
    {
        module *pModule = (module *)p_module;

        delete [] pModule->pTag[0];
		pModule->pTag[0] = NULL;

        delete pModule->pMonitor;
        pModule->pMonitor = NULL;
        delete pModule;
        pModule = NULL;
    }
}

LIB_PUBLIC void get_inputted_message_type(module_t *p_module, const char ***const ppp_inputted_message_types, uint32_t *p_message_type_count)
{
	if (p_module){
		module * pModule = (module *)p_module;
		*ppp_inputted_message_types = (const char **)pModule->pTag;
		*p_message_type_count  = 1;
	}
}
LIB_PUBLIC module_state_t run(module_t *p_module)
{
    module_state_t tmpState = MODULE_FATAL;
    if (p_module)
    {
        module *pModule = (module *)p_module;
        pModule->pMonitor->begin(ActionFileProc, (void *)pModule);
        tmpState = MODULE_OK;
    }
    return tmpState;
}

LIB_PUBLIC module_state_t stop(module_t *p_module)
{
    module_state_t tmpState = MODULE_FATAL;
    if (p_module)
    {
        module *pModule = (module *)p_module;

        pModule->pMonitor->end();
        tmpState = MODULE_OK;
    }
    return tmpState;
}

LIB_PUBLIC module_data_t *assign(module_t *p_module, const module_data_t *p_data, bool is_sync)
{
   // printf("\n  =========================== assign_usb_monitor ============================= \n");

    if (p_module)
    {
        module *pModule = (module *)p_module;
        module_data_t *module_data_handle = copy_module_data(p_data);
        if (module_data_handle)
        {
            const char *buffer;
            uint32_t buffer_size;
            int32_t ret = get_module_data_property(module_data_handle, "MESSAGE_CATEGORY", &buffer, &buffer_size);
            if (ret == 0)
            {
                if ((int)(*buffer) == CATEGORY_TASK)
                {
                    ret = get_module_data_property(module_data_handle, "CENTER_MESSAGE_TASK_CMD", (const char **)&buffer, &buffer_size);
                    if (0 == ret)
                    {
                        char *cmd = strdup(buffer);
                        int length = buffer_size;
                        printf(">>> Cmd = %s Length = %d\n", cmd, length);
                        free(cmd);
                        ret = get_module_data_property(module_data_handle, "CENTER_MESSAGE_TASK_PARAMS", (const char **)&buffer, &buffer_size);
                        if (0 == ret)
                        {
                            char *data = strdup(buffer);
                            int size = buffer_size;
                            memset(&(pModule->usbConfig), 0, sizeof(usb_monitor_config_data));
                            
                            Json::CharReaderBuilder reader_b;
                            Json::CharReader *reader(reader_b.newCharReader());

                            Json::Value root;
                            JSONCPP_STRING errs;
                            bool ok = reader->parse(data, data + strlen(data), &root, &errs);
                            if (ok && errs.size() == 0)
                            {
                                std::vector<std::string> vec;
                                if (root["auto_upload"].isInt())
                                {
                                    pModule->usbConfig.auto_upload = root["auto_upload"].asInt();
                                }
                                if (root["max_size"].isInt())
                                {
                                    pModule->usbConfig.max_size = root["max_size"].asInt();
                                }
                                Json::Value array = root["file_type"];
                                for (int index = 0; index < (int)array.size(); index++)
                                {
                                    pModule->usbConfig.file_filter[index] = array[index].asInt();
                                }
                            }
                            printf(">>> data = %s size = %d\n", data, size);
                            free(data);
                        }
                        else
                        {
                            printf(" get CENTER_MESSAGE_TASK_PARAMS == 0\n");
                        }
                    }
                    else
                    {
                        printf(" get CENTER_MESSAGE_TASK_CMD == 0\n");
                    }
                }
            }
            else
            {
                printf(" get MESSAGE_CATEGORY == 0\n");
            }
        }
    }
    if (is_sync)
    {
        module_data_t *data = create_module_data();
        int status = 0;
        char tmp_id[] = "MSG_TYPE_EDR_SYSINFO";
        set_module_data_property(data, g_p_message_id, tmp_id, strlen(tmp_id));
        set_module_data_property(data, "CENTER_MESSAGE_TASK_RESULT", (const char *)&status, sizeof(status));
        return data;
    }
    else
    {
        return NULL;
    }
}
