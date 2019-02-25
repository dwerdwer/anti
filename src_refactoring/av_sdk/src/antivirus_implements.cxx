#include "antivirus_interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "IRpcMessage.h"
#include "RpcMessage.h"
#include "IRpcClient.h"
#include "RpcMessage.pb.h"

#include "json.hpp"
using json = nlohmann::json;

struct module
{
    void *ptr_data;
    //ScanNotify ptr_notify;
    //ListNotify ptr_callback;
    DataNotify ptr_dataNotify;
    void *ptr_param;
};

class ConnNotify : public IRpcNotify
{
  public:
    ConnNotify()
    {
        ptr_handler = NULL;
        ptr_Connection = NULL;
    }
    //通知消息
    int NotifyInfo(IRpcConnection *hConnection, IRpcMessage *ptr_msg)
    {
        //ptr_msg->Dump();
        ptr_Connection = hConnection;
        std::string data = ptr_msg->Get_StringValue(4);
        DataProc(hConnection, data);
        return 0;
    }

    int DataProc(IRpcConnection *hConnection, std::string data)
    {
        if (data.empty() == true)
            return 0;
        if (ptr_handler == NULL)
            return 0;
        printf("___xxxxxx####### %s \n",data.c_str());
        ptr_handler->ptr_dataNotify(data.c_str(),data.length(),0,ptr_handler->ptr_param);
        return 1;
    }

    //连接断开通知：
    int OnDisconnected(IRpcConnection *hConnection, int error = 0)
    {
        printf("Disconnect by server\n");
        ptr_Connection = NULL;
        return -1;
    }

    void SetHandler(module *p_module)
    {
        ptr_handler = p_module;
    }
    IRpcConnection * GetConnection(){
        return ptr_Connection;
    }

  private:
    module *ptr_handler;
    IRpcConnection *ptr_Connection;
};

ConnNotify gConnNotify;

uint32_t dataPost(void *ptr, const char *buffer, int lenght)
{
    if (ptr == nullptr)
        return 0;
    if (buffer == nullptr)
        return 0;
    ConnNotify * pConnnNotify = (ConnNotify *)ptr;
    IRpcConnection *hConnection = pConnnNotify->GetConnection();
    if (hConnection){
        IRpcMessage *ptr_data = New_IRpcMsg(2, 3);
        ptr_data->Add_StringValue(4, buffer);
        hConnection->SendResponse(ptr_data);
    }else{
        printf("post data [%s]\n", buffer);
        IRpcMessage *ptr_data = New_IRpcMsg(2, 3);
        ptr_data->Add_StringValue(4, buffer);
        QueryNotify("127.0.0.1", 7681, ptr_data, (IRpcNotify *)ptr);
    }
    return 1;
}

void *av_sdk_init()
{
    module *ptr_module = new module;
    if (nullptr != ptr_module)
    {
        ptr_module->ptr_data = &gConnNotify;
        gConnNotify.SetHandler(ptr_module);
    }
    return ptr_module;
}
void av_sdk_uninit(void *ptr_sdk)
{
    if (ptr_sdk)
    {
        delete (module *)ptr_sdk;
    }
}
uint32_t av_sdk_stop(void *ptr_sdk)
{
    return 0;
}
uint32_t av_sdk_pause(void *ptr_sdk)
{
    return 0;
}
uint32_t av_sdk_resume(void *ptr_sdk)
{
    return 0;
}
uint32_t av_list_isolation(void *ptr_sdk, DataNotify ptr_notify, void *ptr_data)
{
    if (nullptr == ptr_sdk)
        return 0;
    if (nullptr == ptr_notify)
        return 0;
    module *ptr_module = (module *)ptr_sdk;
    ptr_module->ptr_dataNotify = ptr_notify;
    //implements
    return 1;
}
uint32_t av_restore_isolation(void *ptr_sdk, const char *ptr_name)
{
    return 0;
}
uint32_t av_scan_target(void *ptr_sdk, uint32_t uint32_option, const char *ptr_path, DataNotify ptr_notify, void *ptr_data)
{
    if (nullptr == ptr_sdk)
        return 0;
    if (ptr_notify == nullptr)
        return 0;
    if (ptr_path == nullptr)
        return 0;
    module *ptr_module = (module *)ptr_sdk;
    ptr_module->ptr_dataNotify = ptr_notify;
    char buffer[1024] = {0};
    //{"cmd":"add_target","data":{"option":8000,"path":"/home/zhangrui"}}
    sprintf(buffer, "{\"cmd\":\"add_target\",\"data\":{\"option\":%u,\"path\":\"%s\"}}", uint32_option, ptr_path);
    printf("sdk:%s\n",buffer);
    return dataPost(ptr_module->ptr_data, buffer, strlen(buffer));
}

uint32_t av_get_path(void *ptr_sdk, DataNotify ptr_notify, void *ptr_data)
{
    if (!ptr_sdk)
        return 0;
    if (!ptr_notify)
        return 0;
    module *ptr_module = (module *)ptr_sdk;
    ptr_module->ptr_dataNotify = ptr_notify;
    ptr_module->ptr_param = ptr_data;
    const char *buffer = "{\"cmd\":\"get_path\"}";
    return dataPost(ptr_module->ptr_data, buffer, strlen(buffer));
}
