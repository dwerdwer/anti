#ifndef LIBWEBSOCKETS_SERVER_MODULE_DEFINES_HHH
#define LIBWEBSOCKETS_SERVER_MODULE_DEFINES_HHH
#include "utils/utils_network.h"

#include <stdint.h>
#include <stdbool.h>

typedef enum server_notify_reason
{
    SERVER_NOTIFY_CONNECTION_ESTABLISHED,
    SERVER_NOTIFY_GET_CLIENT_ADDR_FAILED,
    SERVER_NOTIFY_CONNECTION_CLOSED,
    SERVER_NOTIFY_RECEIVE_DATA,
    SERVER_NOTIFY_RECEIVE_DATA_IGNORED,
    SERVER_NOTIFY_RECEIVE_SELF_BUFFER_FULL,
    SERVER_NOTIFY_RECEIVE_SELF_REQUEST_ERROR,
    SERVER_NOTIFY_RECEIVE_WORKER_ERROR,
    SERVER_NOTIFY_WORKER_ASYNC_REQUEST_ERROR,
    SERVER_NOTIFY_WORKER_ASYNC_RESULT_ERROR,
    SERVER_NOTIFY_SEND_DATA,
    SERVER_NOTIFY_SEND_DATA_ERROR,
}server_notify_reason_t;

// 'wt' is short for "working thread".
typedef struct lws_server_wt
{
    void **pp_params;
    uint8_t count;          // maximum working thread count is 31.
}lws_server_wt_t;

typedef void* connect_t;
#define IN
#define OUT

// Important: For sync_request and async_result,
// in event loop, there is no chance to release memory which stores output data.
// So, storing output data in a thread specific memory is strict.

class callback_interface
{
public:
    // Log related callbacks
    virtual void notify(connect_t p_rpc_conn, server_notify_reason_t reason,
            peer_addr_t *p_peer_addr, const char *p_info, uint32_t info_len) = 0;

    // Eventloop related callbacks
    virtual void sync_request(connect_t p_rpc_conn,
            const char *p_input_data, uint32_t input_data_len) = 0;
    virtual void async_request(void *p_thread_params, connect_t p_rpc_conn,
            const char *p_data, uint32_t data_len) = 0;

    // Connection related callbacks
    virtual bool is_closable(connect_t p_rpc_conn) = 0;
    virtual connect_t connection_create(IN const peer_addr_t *p_peer_addr, IN void* p_token) = 0;
    virtual void connection_destroy(connect_t p_rpcconnection) = 0;
};

class callback_base : public callback_interface
{
public:
    virtual void notify(connect_t p_rpc_conn, server_notify_reason_t reason,
            peer_addr_t *p_peer_addr, const char *p_info, uint32_t info_len);

    virtual bool is_closable(void *p_rpc_conn);
    virtual connect_t connection_create(IN const peer_addr_t *p_peer_addr, IN void* p_token);
    virtual void connection_destroy(connect_t p_conn_params);
};

#endif
