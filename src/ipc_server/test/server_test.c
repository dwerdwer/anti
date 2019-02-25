
#include "libwebsockets_server_eventloop.h"
#include "libwebsockets.h"

#include "utils/utils_network.h"

#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

const char g_p_format_str [] = "%s received";

typedef struct conn_params
{
    char *p_output_buffer;
    uint32_t buffer_length;
}conn_params_t;

typedef struct wt_params
{
    void *p_token;
    // Note: use 'conn_params_t' to store the params of each connection,
    // in order to handle async request synchronously.
    conn_params_t conn_params;
    uint32_t output_data_length;
}wt_params_t;

static void handle_notify(va_list args)
{
    conn_params_t *p_params = (conn_params_t*)va_arg(args, void*);
    //server_notify_reason_t reason = va_arg(args, server_notify_reason_t);
    server_notify_reason_t reason = (server_notify_reason_t)va_arg(args, int);
    peer_addr_t *p_client_addr = va_arg(args, peer_addr_t*);
    const char *p_info = va_arg(args, const char*);
    uint32_t info_length = va_arg(args, uint32_t);
    (void)p_params;

    switch(reason)
    {
        case SERVER_NOTIFY_CONNECTION_ESTABLISHED:
            lwsl_notice("client %s:%d, connection established, info: %s\n", 
                    p_client_addr->address, p_client_addr->port, p_info);
            break;
        case SERVER_NOTIFY_GET_CLIENT_ADDR_FAILED:
            lwsl_notice("get client addr failed, info: %s\n", p_info);
            break;
        case SERVER_NOTIFY_CONNECTION_CLOSED:
            lwsl_notice("client %s:%d, connection closed, info: %s\n", 
                    p_client_addr->address, p_client_addr->port, p_info);
            break;
        case SERVER_NOTIFY_RECEIVE_DATA:
            ((char*)p_info)[info_length] = 0;
            lwsl_notice("client %s:%d, receive data, info: %s, size: %u\n", 
                    p_client_addr->address, p_client_addr->port, p_info, info_length);
            break;
        case SERVER_NOTIFY_RECEIVE_DATA_IGNORED:
            ((char*)p_info)[info_length] = 0;
            lwsl_notice("client %s:%d, receive data ignored, info: %s, size: %u\n", 
                    p_client_addr->address, p_client_addr->port, p_info, info_length);
            break;
        case SERVER_NOTIFY_RECEIVE_SELF_BUFFER_FULL:
            lwsl_notice("client %s:%d, receive self buffer full, info: %s\n", 
                    p_client_addr->address, p_client_addr->port, p_info);
            break;
        case SERVER_NOTIFY_RECEIVE_SELF_REQUEST_ERROR:
            lwsl_notice("client %s:%d, receive self request error, info: %s\n", 
                    p_client_addr->address, p_client_addr->port, p_info);
            break;
        case SERVER_NOTIFY_RECEIVE_WORKER_ERROR:
            lwsl_notice("client %s:%d, receive worker error, info: %s\n", 
                    p_client_addr->address, p_client_addr->port, p_info);
            break;
        case SERVER_NOTIFY_WORKER_ASYNC_REQUEST_ERROR:
            lwsl_notice("client %s:%d, worker async request error, info: %s\n", 
                    p_client_addr->address, p_client_addr->port, p_info);
            break;
        case SERVER_NOTIFY_WORKER_ASYNC_RESULT_ERROR:
            lwsl_notice("client %s:%d, worker async result error, info: %s\n", 
                    p_client_addr->address, p_client_addr->port, p_info);
            break;
        case SERVER_NOTIFY_SEND_DATA:
            ((char*)p_info)[info_length] = 0;
            lwsl_notice("client %s:%d, send data, info: %s, size: %u\n", 
                    p_client_addr->address, p_client_addr->port, p_info, info_length);
            break;
        case SERVER_NOTIFY_SEND_DATA_ERROR:
            ((char*)p_info)[info_length] = 0;
            lwsl_notice("client %s:%d, send data error, info: %s, size: %u\n", 
                    p_client_addr->address, p_client_addr->port, p_info, info_length);
            break;
        default:
            break;
    }
}

static int32_t prepare_output_buffer(conn_params_t *p_params, uint32_t request_length)
{
    int32_t result = 0;
    if(request_length > p_params->buffer_length)
    {
        char *p_temp = (char*)malloc(request_length);
        if (NULL != p_temp)
        {
            free(p_params->p_output_buffer);
            p_params->p_output_buffer = p_temp;
            p_params->buffer_length = request_length;
        }
        else
        {
            result = -1;
        }
    }
    return result;
}

static void handle_sync_request(va_list args)
{
    conn_params_t *p_params = (conn_params_t*)va_arg(args, void*);
    const char* p_input_data = va_arg(args, const char*);
    uint32_t input_data_length = va_arg(args, uint32_t);
    const char **pp_output_data = va_arg(args, const char**);
    uint32_t *p_output_data_length = va_arg(args, uint32_t*);

    uint32_t request_length = input_data_length + (uint32_t)sizeof(g_p_format_str);
    if (0 == prepare_output_buffer(p_params, request_length))
    {
        ((char*)p_input_data)[input_data_length] = 0;
        *p_output_data_length = (uint32_t)snprintf(p_params->p_output_buffer, 
                p_params->buffer_length, g_p_format_str, p_input_data);
        *pp_output_data = p_params->p_output_buffer;
    }
    else
    {
        lwsl_err("Handle sync request failed. Data: %s Length: %u\n",
                p_input_data, input_data_length);
    }
}

static void handle_async_request(va_list args)
{
    wt_params_t *p_params = (wt_params_t*)(va_arg(args, void*));
    peer_addr_t *p_client_addr = (peer_addr_t*)(va_arg(args, peer_addr_t*));
    (void)p_client_addr;
    void *p_token = va_arg(args, void*);
    const char *p_input_data = va_arg(args, const char*);
    uint32_t input_data_length = va_arg(args, uint32_t);

    conn_params_t *p_conn_params = &(p_params->conn_params);
    uint32_t request_length = input_data_length + (uint32_t)sizeof(g_p_format_str);
    ((char*)p_input_data)[input_data_length] = 0;
    if (0 == prepare_output_buffer(p_conn_params, request_length))
    {
        ((char*)p_input_data)[input_data_length] = 0;
        p_params->p_token = p_token;
        p_params->output_data_length = (uint32_t)snprintf(p_conn_params->p_output_buffer, 
                p_conn_params->buffer_length, g_p_format_str, p_input_data);
    }
    else
    {
        lwsl_err("Handle async request failed. Data: %s Length: %u\n",
                p_input_data, input_data_length);
    }
}

static void handle_async_result(va_list args)
{
    wt_params_t *p_params = (wt_params_t*)(va_arg(args, void*));
    void **pp_token = va_arg(args, void**);
    const char **pp_output_data = va_arg(args, const char**);
    uint32_t *p_output_data_length = va_arg(args, uint32_t*);

    *pp_token = p_params->p_token;
    *pp_output_data = p_params->conn_params.p_output_buffer;
    *p_output_data_length = p_params->output_data_length;

    // Note: indicate that the result of current message has been returned.
    p_params->p_token = NULL;
    p_params->output_data_length = 0;
}

static void *server_evloop_callback(server_cb_reason_t reason, ...)
{
    void *p_result = NULL;
    va_list args;
    va_start(args, reason);
    switch(reason)
    {
        case SERVER_CALLBACK_NOTIFY:
            handle_notify(args);
            break;
        case SERVER_CALLBACK_SYNC_REQUEST:
            handle_sync_request(args);
            break;
        case SERVER_CALLBACK_ASYNC_REQUEST:
            handle_async_request(args);
            break;
        case SERVER_CALLBACK_ASYNC_RESULT:
            handle_async_result(args);
            break;
        case SERVER_CALLBACK_CLOSABLE:
            p_result = (void*)false;
            break;
        default:
            break;
    }
    va_end(args);
    return p_result;
}

static void *create_conn_params(void *p_create_params)
{
    void *p_result = malloc(sizeof(conn_params_t));
    if (NULL != p_result)
    {
        conn_params_t *p = (conn_params_t*)p_result;
        p->p_output_buffer = NULL;
        p->buffer_length = 0;
    }
    return p_result;
}

static void destroy_conn_params(void *p_conn_params, void *p_destroy_params)
{
    free(p_conn_params);
}

static void run_evloop(lws_server_wt_t *p_wt)
{
    lws_server_evloop_info_t info;
    info.callbacks.unified_callback = server_evloop_callback,

    info.callbacks.conn_callbacks.create = create_conn_params,
    info.callbacks.conn_callbacks.p_create_params = NULL,
    info.callbacks.conn_callbacks.destroy = destroy_conn_params,
    info.callbacks.conn_callbacks.p_destroy_params = NULL,

    //info.protocol_group = PROTOCOL_GROUP_BASED_ON_WEBSOCKETS,
    info.protocol_group = PROTOCOL_GROUP_BASED_ON_RAW_SOCKET;
    info.working_thread = *p_wt;
    info.p_iface = NULL;
    info.port = 7681;
    info.hint_max_sending_size = 4096;
    info.dead_timeout = 0;

    lws_server_evloop_t *p_evloop = create_server_evloop(&info);

    launch_server_evloop(p_evloop, 500);

    destroy_server_evloop(p_evloop);
}

static int32_t initialize_wt_params(wt_params_t **p_wt_params_ptrs, 
        uint8_t working_thread_count)
{
    int32_t result = 0;
    uint8_t i = 0;
    for (; i < working_thread_count; ++i)
    {
        wt_params_t *p_temp = (wt_params_t*)malloc(sizeof(wt_params_t));
        if (NULL != p_temp)
        {
            p_temp->p_token = NULL;
            p_temp->conn_params.p_output_buffer = NULL;
            p_temp->conn_params.buffer_length = 0;
            p_temp->output_data_length = 0;

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
    if (NULL != pp_temp &&
            0 == initialize_wt_params(pp_temp, working_thread_count) )
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
        free(pp_wt_params_ptrs[i]);
    }
    free(pp_wt_params_ptrs);
}

static int32_t run_evloop_with_working_thread(uint8_t working_thread_count)
{
    wt_params_t **pp_wt_params = create_wt_params_ptrs(working_thread_count);
    if (NULL != pp_wt_params)
    {
        lws_server_wt_t wt = 
        {
            .pp_params = (void**)pp_wt_params,
            .count = (uint8_t)working_thread_count,
        };
        run_evloop(&wt);
        destroy_wt_params_ptrs(pp_wt_params, working_thread_count);
    }
    return 0;
}

static int32_t run_evloop_without_working_thread(void)
{
    lws_server_wt_t wt = 
    {
        .pp_params = (void**)NULL,
        .count = 0,
    };
    run_evloop(&wt);
    return 0;
}

int32_t main(int32_t argc, const char* argv[])
{
    int32_t result = -1;

    if (2 <= argc)
    {
        int32_t working_thread_count = (int32_t)strtol(argv[1], (char**)NULL, 10);

        if (0 == working_thread_count)
        {
            result = run_evloop_without_working_thread();
        }
        else if (0 < working_thread_count && 31 > working_thread_count)
        {
            result = run_evloop_with_working_thread((uint8_t)working_thread_count);
        }
        else
        {
            // Do nothing.
        }
    }
    else if (1 == argc)
    {
        result = run_evloop_without_working_thread();
    }
    else
    {
        // Do nothing.
    }

    return result;
}


