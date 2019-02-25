
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "IRpcMessage.h"
#include "RpcMessage.h"
#include "IRpcClient.h"
#include "debug_print.h"
#include "vc_client_function.h"

class asyn_notify_t:public IRpcNotify
{
public:
    asyn_notify_t(const char* host, int16_t port);
    
    virtual ~asyn_notify_t(); 
    // receive
    virtual int NotifyInfo(IRpcConnection* hConnection, IRpcMessage* msg); 
    
    virtual int OnDisconnected(IRpcConnection* hConnection, int error);

    int16_t port;
    char *p_host;

    send_params_t send_params;
    IRpcConnection *p_connect;
};

asyn_notify_t::asyn_notify_t(const char* host, int16_t port)
{
    this->p_host = new char[strlen(host) + 1] ();
    strncpy(this->p_host, host, strlen(host) + 1);
    this->port = port;

    this->p_connect = NULL;
}

asyn_notify_t::~asyn_notify_t()
{
    if(NULL != this->p_connect)
    {
        this->p_connect->Stop();
        this->p_connect = NULL;
    }
    if(this->p_host) delete this->p_host;    
    this->p_host = NULL;
}

int asyn_notify_t::NotifyInfo(IRpcConnection* hConnection, IRpcMessage* msg)
{
    if(NULL == this->p_connect)
        this->p_connect = hConnection;

    std::string notify_str = msg->to_String();

    this->send_params.async_params.notifier
        (notify_str.c_str(), notify_str.size(), send_params.async_params.p_params);

    Release_IRpcMsg(msg);

    return 0;
}

int asyn_notify_t::OnDisconnected(IRpcConnection* hConnection, int error)
{
    if(hConnection) hConnection->Stop(); //TODO: do what?
    
    this->p_connect = NULL;
    
    return 0;
}

struct syn_notify_t
{
    syn_notify_t(const char* host, int16_t port)
    {
        this->p_host = new char[strlen(host) + 1] ();
        strncpy(this->p_host, host, strlen(host) + 1);
        this->port = port;
    }
    ~syn_notify_t()
    {
        if(this->p_host) delete this->p_host;    
        this->p_host = NULL;
    }
    int16_t port;
    char *p_host;
};

struct rpc_client
{    
    rpc_client(const char* host, int16_t port)
    {
        this->p_syn_notify = new syn_notify_t(host, port);
    }
    ~rpc_client()
    {
        delete this->p_syn_notify;
    }
    syn_notify_t *p_syn_notify;
};

rpc_client_t *create_rpc_client(const char* host, int16_t port)
{
    if(NULL == host) return NULL;

    rpc_client *p_rpc_client = new rpc_client(host, port);
/*
    if(NULL != p_rpc_client) {   
        IRpcMessage* send_msg = New_IRpcMsg(RPC_ECHO, RPC_ECHO_TEST); // temp test

        send_msg->Add_StringValue(RPCKEY_CHAR, "Temp agreement");

        if(0 != QueryNotify(host, port, send_msg, (IRpcNotify*)p_rpc_client)) {
            delete p_rpc_client;
            p_rpc_client = NULL;
        }
    }*/
    return p_rpc_client;
}

void destroy_rpc_client(rpc_client_t* p_client)
{
    if(p_client)
    {
        delete p_client;
        p_client = NULL;
    }
}

int32_t send_to_server(rpc_client_t* p_client, int32_t cmd_id, 
            int32_t sub_cmd_id, const char *p_data, uint32_t data_length, send_params_t *p_params)
{
    int32_t result = -1;

    if(NULL == p_client || NULL == p_data || NULL == p_params)
        return result;
/*  if(p_client->p_connect) {
        IRpcMessage* req_msg = New_IRpcMsg(cmd_id, sub_cmd_id);

        req_msg->Add_StringValue(RPCKEY_CHAR, p_data);

        result = p_client->p_connect->SendResponse(req_msg);
    }*/
    // sync short connection 
    IRpcMessage* req_msg = New_IRpcMsg(cmd_id, sub_cmd_id);

    std::string req_str(p_data, data_length);

    req_msg->Add_StringValue(RPCKEY_CHAR, req_str.c_str());

    int ret_code = 0;
    IRpcMessage* res_msg = QueryData(p_client->p_syn_notify->p_host, 
                                     p_client->p_syn_notify->port, req_msg, &ret_code);
    if(NULL == res_msg){
        debug_print("query host: %s query port: %d error ret_code = %d\n", 
                    p_client->p_syn_notify->p_host, p_client->p_syn_notify->port, ret_code);
        result = -1;
    }
    else {
        if(p_params->sync_params.p_result_buffer)
            strncpy(p_params->sync_params.p_result_buffer, 
                    res_msg->Get_StringValue(RPCKEY_CHAR), p_params->sync_params.result_buffer_length);

        Release_IRpcMsg(res_msg);
        result = 0;
    }
    return result;
}

