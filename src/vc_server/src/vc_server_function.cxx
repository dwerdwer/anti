
#include <stdio.h>
#include <string.h>

#include "IRpcMessage.h"
#include "RpcMessage.h"
#include "ipc_server.h"
#include "debug_print.h"
#include "module_data.h"
#include "vc_server_defines.h"
#include "vc_server_function.h"

using namespace std;

class rpc_server_cb:public IRpcServer
{
public:
    rpc_server_cb(recdata_queue_t *p_cache_data_queue)
    {
        this->p_cache_data_queue = p_cache_data_queue;
    }
    virtual void OnConnect(IRpcConnection* p_ipc_conn)
    {
        // TODO:  user determine stor this p_ipc_conn or not
        // if store then , OnClose must call p_ipc_conn->Release() to release the buffer where this pointer to point
    }
    virtual void OnReceive(IRpcConnection* p_ipc_conn, IRpcMessage* msg);
    
    virtual void OnClose(IRpcConnection* p_ipc_conn)
    {
        //TODO:  if user already stored this p_ipc_conn,
        // user must call p_ipc_conn->Release() and remove this pointer
        // else do nothing!!!
        // p_ipc_conn->Release();
    }
    virtual void ClearInstParam(IRpcConnection* p_ipc_conn) {  }

    virtual ~rpc_server_cb() {  }
private:
    recdata_queue_t *p_cache_data_queue;
};

void rpc_server_cb::OnReceive(IRpcConnection* p_ipc_conn, IRpcMessage* msg)
{    
    //std::string recv_str = msg->to_String();
   
    debug_print("recv_str = %s\n", msg->Get_StringValue(RPCKEY_CHAR));
    //TODO: scan virus balabalabala
    IRpcMessage *res_msg = NewInstance_IRpcMessage(RPC_ECHO, RPC_ECHO_TEST);
    res_msg->Add_StringValue(RPCKEY_CHAR, "Test responses string");

    p_ipc_conn->SendResponse(res_msg);
    delete res_msg;
}

struct ipc_connect
{
    ipc_server_t* p_server;
};

ipc_connect_t *create_ipc_connect(uint32_t wt_count, uint32_t port, void *p_cache_data_queue)
{
    ipc_connect_t *p_ipc_connect = NULL;
    ipc_server_t* p_server = NULL;
    rpc_server_cb *p_rpc_cb = new rpc_server_cb((recdata_queue_t*)p_cache_data_queue);

    if(NULL == p_rpc_cb)
        return p_ipc_connect;

    p_server = start_ipc_server(wt_count, port, p_rpc_cb);

    if (NULL == p_server)
    {
        debug_print("Start IPC server failed! thread count = %d port = %u\n", wt_count, port);

        if(p_rpc_cb) {
            delete p_rpc_cb; p_rpc_cb = NULL;
        }
        return p_ipc_connect;
    }
    p_ipc_connect = new ipc_connect;

    p_ipc_connect->p_server = p_server;

    return p_ipc_connect;
}

void destroy_ipc_connect(ipc_connect_t *p_ipc_connect)
{
    if(p_ipc_connect) {
        if(p_ipc_connect->p_server) {
            stop_ipc_server(p_ipc_connect->p_server); p_ipc_connect->p_server = NULL;
        }
        delete p_ipc_connect; p_ipc_connect = NULL; 
    }
}

int send_to_other_modules(module_t *p_module, const char *p_data, 
                          uint32_t data_len, const char *p_message_id, const char *p_message_type)
{
    int result = -1;

    if(NULL == p_module || NULL == p_data || NULL == p_message_id || NULL == p_message_type)
        return result;

    module_data_t *p_module_data = create_module_data();
    if(NULL != p_module_data)
    {
        if(0 == set_module_data_property(p_module_data, g_p_message_id, p_message_id, strlen(p_message_id)))
        {
            if(0 == set_module_data_property(p_module_data, p_message_type, p_data, data_len))
            {
                module_message_t module_message;
                memset(&module_message, 0, sizeof(module_message_t));

                module_message.p_data = p_module_data;
                module_message.category = (module_category_t)p_module->category;

                mdh_sync_params_t sync_param;
                memset(&sync_param, 0, sizeof(mdh_sync_params_t));
                sync_param.is_sync = false;

                p_module->notifier(&module_message, p_module->p_params, &sync_param);

                result = 0;
            }
        }
        destroy_module_data(p_module_data);
    }
    return result;
}

int send_to_reporter(module_t *p_module, const char *p_data, uint32_t data_len)
{
    int result = -1;
    result = send_to_other_modules(p_module, 
                    p_data, data_len, "MSG_TYPE_REPORTER", "REPORTER_MESSAGE_DATA");
    return result;
}

#define MAX_LOG_LENGTH      1024
#define LOG_PREFIX          "VC_SERVER: "

void vc_server_log(module_t *p_module, const char* fmt, ...)
{
    char log_str[MAX_LOG_LENGTH] = { 0 };

    strcpy(log_str, LOG_PREFIX);

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(log_str + strlen(log_str), MAX_LOG_LENGTH - strlen(log_str), fmt, ap);
    va_end(ap);
    debug_print("%s\n", log_str);

    send_to_other_modules(p_module, log_str, strlen(log_str), "MSG_TYPE_LOG", "LOG_MESSAGE");
}

