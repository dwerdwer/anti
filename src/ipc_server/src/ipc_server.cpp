#include "libwebsockets_server_eventloop.h"
#include "libwebsockets.h"
#include "libwebsockets_server_module_defines.h"

#include "utils/utils_network.h"
#include "utils/utils_library.h"

#include <iostream>
#include <string>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <vector>
#include <queue>
#include "IRpcMessage.h"
#include "RpcMessage.h"
#include "ipc_server.h"

struct ipc_server{
	lws_server_evloop_t *p_evloop;
};

class ipc_connection_t: public IRpcConnection
{
private:
    void *          m_InstParam[MAX_INSTPARAMS];
    long            m_lRef;
    IRpcServer*     m_pIRpcServer;
    uint32_t        m_sync_flag;   // 1 sync  0 async
    bool            m_b_is_closable;
    void*           m_p_token;
    void*           m_p_lws_connection;
    void*           m_p_lws_token;
	std::string 	m_strdata;
	const peer_addr_t* 	m_p_peer_addr;

public:
    ipc_connection_t(IRpcServer*p_rpcserver, void* p_token, IN const peer_addr_t* p_addr)
    {
        m_lRef = 1;
        m_pIRpcServer = p_rpcserver;
        memset(m_InstParam, sizeof(m_InstParam), 0);
        m_b_is_closable = false;
        m_p_token = p_token;
		m_p_peer_addr = p_addr;
    }

private:
    virtual ~ipc_connection_t(){}

public:
    // add on ref count , initlize 1
    virtual void    AddRef()
    {
        __sync_fetch_and_add(&m_lRef, 1);
    }

    virtual void    Release()   // dec 1
    {
        __sync_fetch_and_sub(&m_lRef, 1);
        if (m_lRef == 0)
        {
            m_pIRpcServer->ClearInstParam(this);
            delete this;
        }
    }

    virtual void* GetInstParam(int index )
    {
        return m_InstParam[index];
    }

    virtual void SetInstParam(void* pClient, int index)
    {
        if (pClient && index < sizeof(m_InstParam))
            m_InstParam[index] = pClient;
    }

    virtual int SendResponse(void* evloop, IRpcMessage *msg)
    {
        //TODO: call do_sync_request , or do_async_request
	std::string str_data = msg->Encode();
	uint32_t response_length = (uint32_t)str_data.length();

    do_send(evloop, m_p_token, (const char*)str_data.c_str(), response_length);
/*        if (m_sync_flag)
	        do_sync_send(m_p_lws_connection, m_p_lws_token, (const char*)str_data.c_str(), response_length);
        else
	        do_async_send(m_p_token, str_data.c_str(), response_length);
*/
    }

// do not exposed below func to extern
public:
	virtual void OnConnect()
	{
		m_pIRpcServer->OnConnect(this);
	}

    virtual void OnClose()
    {
        m_pIRpcServer->OnClose(this);
    }

    void        OnReceive(const char* p_input_data, uint32_t input_data_len)
    {
		m_strdata.append(p_input_data, input_data_len);
        int msg_len = 0;
        IRpcMessage *msg = NewInstance_IRpcMessage(m_strdata, &msg_len);
		if (msg_len < 0 && msg_len > -100) //not enough
		{

		}
		else if (msg_len <= -100)
		{
			m_strdata.clear();
			std::string strtmp(p_input_data, input_data_len);
            lwsl_notice("client %s:%d, connection receive data invalid, info: %s\n",
                    m_p_peer_addr->address, m_p_peer_addr->port,strtmp.c_str()) ;
		}
		else if (msg)
		{
        	m_pIRpcServer->OnReceive(this, msg);
			delete msg;
			m_strdata.erase(0, msg_len);
		}
		else  // data valid, but msg pointer is null
		{

		}
    }

    void Close()
    {}

    void SetIsClosed(bool is_close)
    {
        m_b_is_closable = is_close;
    }



};

typedef struct wt_params
{
    // Note: use 'conn_params_t' to store the params of each connection,
    // in order to handle async request synchronously.
    char* p_last_output_buffer;
    pthread_spinlock_t lock;
}wt_params_t;

class callback : public callback_base
{
    public:
        virtual void notify(connect_t p_ipc_conn, server_notify_reason_t reason,
                peer_addr_t *p_peer_addr, const char *p_info, uint32_t info_len);
        virtual void sync_request(void *p_rpcconnection,
                const char *p_input_data, uint32_t input_data_len);
        virtual void async_request(void *p_thread_params,connect_t p_ipc_conn,
                const char *p_data, uint32_t data_len);

        virtual bool is_closable(void *p_conn_params);

        virtual connect_t connection_create(IN const peer_addr_t *p_peer_addr, IN void* p_token);

        virtual void connection_destroy(void *p_conn_params);
    public:
        inline void set_rpcserver(IRpcServer* p_rpcserver)
        {
            m_p_rpcserver = p_rpcserver;
        }
        inline IRpcServer* get_rpcserver()
        {
            return m_p_rpcserver;
        }
    public:
        callback(){m_p_rpcserver = NULL;}

    private:
        IRpcServer*     m_p_rpcserver;
};

typedef callback callback_t;

bool callback::is_closable(void *p_conn_params)
{
    return false;
    /*
       conn_params_t *p_params = (conn_params_t*)p_conn_params;
       return p_params->b_isclosable;
       */
}

connect_t callback::connection_create(IN const peer_addr_t *p_peer_addr, IN void* p_token)
{
    ipc_connection_t* p_conn = new ipc_connection_t(m_p_rpcserver, p_token, p_peer_addr);
    return (void*)p_conn;
}

void callback::connection_destroy(void *p_ipc_conn)
{
    printf("__func__: %s\n", __func__);
    ipc_connection_t* p_conn = reinterpret_cast<ipc_connection_t*>(p_ipc_conn);
    p_conn->Release();
}

void callback::notify(void *p_ipc_conn, server_notify_reason_t reason,
        peer_addr_t *p_peer_addr, const char *p_info, uint32_t info_len)
{
    switch(reason)
    {
        case SERVER_NOTIFY_CONNECTION_ESTABLISHED:
			{
            	lwsl_notice("client %s:%d, connection established, info: %s\n",
                	    p_peer_addr->address, p_peer_addr->port, p_info);
				ipc_connection_t* p_conn = reinterpret_cast<ipc_connection_t*>(p_ipc_conn);
				if (p_conn)
					p_conn->OnConnect();
			}
            break;
        case SERVER_NOTIFY_GET_CLIENT_ADDR_FAILED:
            lwsl_notice("get client addr failed, info: %s\n", p_info);
            break;
        case SERVER_NOTIFY_CONNECTION_CLOSED:
            {
				lwsl_notice("client %s:%d, connection closed\n",
						p_peer_addr->address, p_peer_addr->port);
				ipc_connection_t* p_conn = reinterpret_cast<ipc_connection_t*>(p_ipc_conn);
				if (p_conn)
					p_conn->OnClose();
            }
            break;
        case SERVER_NOTIFY_RECEIVE_DATA:
            lwsl_notice("clietn %s:%d, receive data, size: %u\n",
                    p_peer_addr->address, p_peer_addr->port, info_len);
            break;
        case SERVER_NOTIFY_RECEIVE_DATA_IGNORED:
            lwsl_notice("client %s:%d, receive data ignored,  size: %u\n",
                    p_peer_addr->address, p_peer_addr->port, info_len);
            break;
        case SERVER_NOTIFY_RECEIVE_SELF_BUFFER_FULL:
            lwsl_notice("client %s:%d, receive self buffer full \n",
                    p_peer_addr->address, p_peer_addr->port);
            break;
        case SERVER_NOTIFY_RECEIVE_SELF_REQUEST_ERROR:
            lwsl_notice("client %s:%d, receive self request error \n",
                    p_peer_addr->address, p_peer_addr->port);
            break;
        case SERVER_NOTIFY_RECEIVE_WORKER_ERROR:
            lwsl_notice("client %s:%d, receive worker error \n",
                    p_peer_addr->address, p_peer_addr->port);
            break;
        case SERVER_NOTIFY_WORKER_ASYNC_REQUEST_ERROR:
            lwsl_notice("client %s:%d, worker async request error\n",
                    p_peer_addr->address, p_peer_addr->port);
            break;
        case SERVER_NOTIFY_WORKER_ASYNC_RESULT_ERROR:
            lwsl_notice("client %s:%d, worker async result error\n",
                    p_peer_addr->address, p_peer_addr->port);
            break;
        case SERVER_NOTIFY_SEND_DATA:
            lwsl_notice("client %s:%d, send datas, size: %u\n",
                    p_peer_addr->address, p_peer_addr->port, info_len);
            break;
        case SERVER_NOTIFY_SEND_DATA_ERROR:
            lwsl_notice("client %s:%d, send data error, size: %u\n",
                    p_peer_addr->address, p_peer_addr->port, info_len);
            break;
        default:
            break;

    }
}


void callback::sync_request(void* p_conn, const char *p_input_data, uint32_t input_data_len)
{

    ipc_connection_t *p_ipc_conn = reinterpret_cast<ipc_connection_t*>(p_conn);
    p_ipc_conn->OnReceive(p_input_data, input_data_len);
}

void callback::async_request(void *p_thread_params, connect_t p_ipc_conn, const char *p_data, uint32_t data_len)
{
    ipc_connection_t *p_conn = reinterpret_cast<ipc_connection_t*>(p_ipc_conn);
    p_conn->OnReceive(p_data, data_len);
}

typedef struct
{
    lws_server_evloop_t *p_event_loop;
    int32_t blocking_timeout;
}thread_arg_t;

void *thread_run_evloop(void *param)
{
    thread_arg_t *p_thread_arg = (thread_arg_t*)param;
    launch_server_evloop(p_thread_arg->p_event_loop, p_thread_arg->blocking_timeout);
    pthread_exit(NULL);
}

static lws_server_evloop_t*  run_evloop(lws_server_wt_t *p_wt, callback_t* p_callback,  uint32_t port)
{
    lws_server_evloop_info_t info;
    info.p_callback = p_callback;
    info.protocol_group = PROTOCOL_GROUP_BASED_ON_RAW_SOCKET;
    info.working_thread = *p_wt;
    info.p_iface = NULL;
    info.port = (uint16_t)port;
    info.hint_max_sending_size = 4096;

    lws_server_evloop_t *p_evloop = create_server_evloop(&info);
    thread_arg_t *p_arg_list = (thread_arg_t*)malloc(sizeof(thread_arg_t)); //这段内存在哪释放
    if (NULL == p_arg_list)
    {
        printf("malloc thread_arg_t err\n");
        return NULL;
    }
    p_arg_list->blocking_timeout = 500;
    p_arg_list->p_event_loop = p_evloop;

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, thread_run_evloop, (void*)p_arg_list);

    return p_evloop;
}

static int32_t initialize_wt_params(wt_params_t **p_wt_params_ptrs,
        uint8_t working_thread_count)
{
    int32_t result = 0;
    uint8_t i = 0;
    for(; i < working_thread_count; ++i)
    {
        wt_params_t *p_temp = new wt_params_t;
        //	wt_params_t *p_temp = (wt_params_t*)malloc(sizeof(wt_params_t));
        if (NULL != p_temp)
        {
            p_temp->p_last_output_buffer = NULL;
            pthread_spin_init(&(p_temp->lock), 0);

            p_wt_params_ptrs[i] = p_temp;
        }
        else
        {
            result = -1;
            break;
        }
    }
    return result;
}

static wt_params_t **create_wt_params_ptrs(uint8_t working_thread_count)
{
    wt_params_t **pp_result = NULL;
    wt_params_t **pp_temp =
        (wt_params_t**)malloc(working_thread_count * sizeof(wt_params_t*));
    if(NULL != pp_temp &&
            0 == initialize_wt_params(pp_temp, working_thread_count))
    {
        pp_result = pp_temp;
    }
    return pp_result;
}

static void destroy_wt_params_ptrs(wt_params_t **pp_wt_params_ptrs,
        uint8_t working_thread_count)
{
    uint8_t i = 0;
    for (; i < working_thread_count; ++i)
    {
        //		pthread_spin_destroy(&(pp_wt_params_ptrs[i]->lock));
        delete(pp_wt_params_ptrs[i]);
    }
    free(pp_wt_params_ptrs);
}

static lws_server_evloop_t* run_evloop_with_working_thread(uint8_t working_thread_count, uint32_t port, callback_t* p_callback)
{
    lws_server_evloop_t *p_evloop = NULL;
    wt_params_t **pp_wt_params = create_wt_params_ptrs(working_thread_count);
    if(NULL != pp_wt_params)
    {
        lws_server_wt_t wt;
        wt.pp_params = (void**)pp_wt_params;
        wt.count = (uint8_t)working_thread_count;

        p_evloop = run_evloop(&wt, p_callback, port);
        //	destroy_wt_params_ptrs(pp_wt_params, working_thread_count);
    }
    return p_evloop;
}

static lws_server_evloop_t* run_evloop_without_working_thread(uint32_t port, callback_t* p_callback)
{
    lws_server_evloop_t *p_evloop = NULL;
    lws_server_wt_t wt;
    wt.pp_params = (void**)NULL;
    wt.count = 0;
    p_evloop = run_evloop(&wt, p_callback, port);
    return p_evloop;
}

LIB_PUBLIC ipc_server_t* start_ipc_server(uint32_t wt_count, uint32_t port, IRpcServer* p_rpcserver)
{

    ipc_server_t *p_ipc_server = (ipc_server_t*)malloc(sizeof(ipc_server_t*));
    if (NULL == p_ipc_server)
        return NULL;
    callback_t* p_callback = new callback_t();
    p_callback->set_rpcserver(p_rpcserver);
    lws_server_evloop_t *p_evloop = NULL;
    if (wt_count == 0)
        p_evloop = run_evloop_without_working_thread(port, p_callback);
    else
        p_evloop = run_evloop_with_working_thread((uint8_t)wt_count, port, p_callback);

    p_ipc_server->p_evloop = p_evloop;
    p_rpcserver->SetEvloop(p_evloop);

    return p_ipc_server;

}

LIB_PUBLIC void stop_ipc_server(ipc_server_t *p_ipc_server)
{
    stop_server_evloop(p_ipc_server->p_evloop, true);
    destroy_server_evloop(p_ipc_server->p_evloop);
}

