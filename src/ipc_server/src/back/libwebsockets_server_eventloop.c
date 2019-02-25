
#include "lws_config.h"
#include "libwebsockets.h"

#include "libwebsockets_server_eventloop.h"
#include "utils/utils_integer.h"
#include "utils/utils_signal.h"
#include "utils/utils_time.h"
#include "utils/utils_library.h"
#include "queue.h"
#include "shared_ptr.h"
#include "thread_pool.h"

#include <string.h>

#include <signal.h>
#include <pthread.h>
#include <syslog.h>
#include <netinet/in.h>

// 'lws' is short for "libwebsockets"
typedef struct lws_context_creation_info lws_server_context_creation_info_t;
typedef struct lws_context lws_server_context_t;
//typedef struct lws_client_connect_info lws_connection_info_t;
typedef struct lws_vhost lws_server_vhost_t;
typedef struct lws lws_connection_t;
typedef struct lws_protocols lws_protocol_t;
typedef struct lws_extension lws_protocol_extension_t;
typedef enum lws_callback_reasons lws_callback_reason_t;
typedef enum lws_write_protocol lws_write_method;

// Those global variables for quiting event loop when some signals triggered.
volatile int32_t g_force_exit = 0;
lws_server_context_t *g_p_server_context;

//int32_t g_debug_level = LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_INFO | LLL_DEBUG;
int32_t g_debug_level = LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_INFO;
//int32_t g_debug_level = LLL_ERR | LLL_WARN | LLL_NOTICE;

#if defined(LWS_OPENSSL_SUPPORT) && defined(LWS_HAVE_SSL_CTX_set1_param)
char crl_path[1024] = "";
#endif

/* http server gets files from this path */
#define LOCAL_RESOURCE_PATH LWS_INSTALL_DATADIR"/websockets_server"
const char *resource_path = LOCAL_RESOURCE_PATH;

/*
 * This mutex lock protects code that changes or relies on wsi list outside of
 * the service thread.	The service thread will acquire it when changing the
 * wsi list and other threads should acquire it while dereferencing wsis or
 * calling apis like lws_callback_on_writable_all_protocol() which
 * use the wsi list and wsis from a different thread context.
 */
pthread_mutex_t lock_established_conns;

/*
 * multithreaded version - protect wsi lifecycle changes in the library
 * these are called from protocol 0 callbacks
 */

void test_server_lock(struct lws *wsi, void *p_user, int care)
{
    if (care)
    {
        pthread_mutex_lock(&lock_established_conns);
    }
}

void test_server_unlock(struct lws *wsi, void *p_user, int care)
{
    if (care)
    {
        pthread_mutex_unlock(&lock_established_conns);
    }
}

struct per_session_data__http 
{
    lws_fop_fd_t fop_fd;
#ifdef LWS_WITH_CGI
    struct lws_cgi_args args;
#endif
#if defined(LWS_WITH_CGI) || !defined(LWS_NO_CLIENT)
    int reason_bf;
#endif
    unsigned int client_finished:1;

    struct lws_spa *spa;
    char result[500 + LWS_PRE];
    int result_len;

    char filename[256];
    long file_length;
    lws_filefd_type post_fd;
};

extern int
callback_http(struct lws *wsi, enum lws_callback_reasons reason, void *user,
        void *in, size_t len);

// Share with working threads.
typedef struct conn_shared_state
{
    lws_connection_t *p_connection;
    pthread_spinlock_t lock;
    void *p_conn_params;
}conn_shared_state_t;

typedef struct lws_connection_state
{
    // TODO: typedef type
    // TODO: tidy interfaces of Queue.
    struct Queue *p_queue;              // Use queue instead temporarily. And working thread run in a loop if queue is full.
    void *p_conn_params;
    conn_shared_state_t *sp_shared_state;
    peer_addr_t client_addr;

    // Note: Since return value of event LWS_CALLBACK_ESTABLISHED can not close connection
    // (I think this is a bug), this variable is used to indicate connections state is not ready. 
    // (avoid processing during event LWS_CALLBACK_RECEIVE and LWS_CALLBACK_SERVER_WRITEABLE.
    bool discard;
}lws_connection_state_t;

// 'td' is short for "thread data"
typedef struct td_scheduler_params
{
    uint8_t curr_thread_index;
    uint8_t max_thread_count;
}td_scheduler_params_t;

typedef struct thread_context
{
    // TODO: same as above
    struct Queue *p_queue;
    pthread_spinlock_t lock;
    callback_base *p_callback;
    void *p_async_params;
}thread_context_t;

typedef struct protocol_context
{
    thread_pool_t *p_thread_pool;
    thread_context_t **pp_thread_contexts;
    callback_base *p_callback;
    uint32_t total_buffer_size;
    bool is_data_sending;
    bool is_pending_stop;
    td_scheduler_params_t scheduler_params;
    uint8_t thread_context_count;
}protocol_context_t;

typedef struct lws_server_evloop
{
    lws_server_context_t *p_server_context;
    protocol_context_t protocol_context;
    // TODO: add "protocol count" to support multi-protocols in one server.
    volatile int32_t stop;
}lws_server_evloop_t;

typedef struct message
{
    unsigned char *p_sending_buf;
    uint32_t length;
}message_t;

typedef struct async_message
{
    conn_shared_state_t *sp_shared_state;
    message_t *p_message;
}async_message_t;

enum
{
    BUFFER_BLOCK_SIZE = 4096,

    INNER_RECEIVE_BUFFER_SIZE = 4096,
    //INNER_SENDING_BUFFER_SIZE = INNER_RECEIVE_BUFFER_SIZE,

    PING_PONG_INTERVAL = 0, //60,    // seconds

    NONE = 0,
    STOP_REQUEST = 1,
    STOP_FORCE = 2,
};

//////////////////////////////////////////////////////////////
// Private Implementations
//////////////////////////////////////////////////////////////
static conn_shared_state_t *create_conn_shared_state(lws_connection_state_t *p_state,
        lws_connection_t *p_connection)
{
    conn_shared_state_t *p_result = (conn_shared_state_t*)malloc(sizeof(conn_shared_state_t));
    if (NULL != p_result &&
            0 == pthread_spin_init(&(p_result->lock), 0) )
    {
        p_result->p_connection = p_connection;
        p_result->p_conn_params = p_state->p_conn_params;
    }
    return p_result;
}

static void destroy_conn_shared_state(conn_shared_state_t *p_shared_state)
{
    pthread_spin_destroy(&(p_shared_state->lock));
    free(p_shared_state);
}

static int32_t initialize_conn_shared_state(lws_connection_state_t *p_state,
        lws_connection_t *p_connection)
{
    int32_t result = -1;

    conn_shared_state_t *p_temp = NULL;
    void *p_sp_temp = NULL;
    if (NULL != (p_temp = create_conn_shared_state(p_state, p_connection)) && 
            NULL != (p_sp_temp = create_shared_ptr(p_temp, sizeof(conn_shared_state_t), 
                    (raw_object_destroyer_t)destroy_conn_shared_state)) )
    {
        p_state->sp_shared_state = static_cast<conn_shared_state_t*>(p_sp_temp);
        result = 0;
    }
    else if (NULL != p_temp)
    {
        destroy_conn_shared_state(p_temp);
    }
    else
    {
        // Do nothing.
    }

    return result;
}

static int32_t prepare_conn_shared_state(lws_connection_state_t *p_state, 
        protocol_context_t *p_protocol_context, lws_connection_t *p_connection)
{
    int32_t result = 0;
    if (NULL != p_protocol_context->p_thread_pool)
    {
        result = initialize_conn_shared_state(p_state, p_connection);
    }
    return result;
}

static int32_t fill_client_addr(peer_addr_t *p_client_addr, lws_connection_t *p_connection)
{
    int32_t result = -1;

    int32_t fd = lws_get_socket_fd(p_connection);
    socket_address_t socket_address;
    socklen_t len = sizeof(socket_address);
    result = getpeername(fd, &(socket_address.general), &len);
    if (0 == result)
    {
        result = fill_peer_addr(p_client_addr, &socket_address);
    }

    return result;
}

static int32_t get_client_addr(lws_connection_state_t *p_state, lws_connection_t *p_connection,
        callback_base *p_callback)
{
    int32_t result = fill_client_addr(&(p_state->client_addr), p_connection);
    if (0 != result)
    {
        p_callback->notify(p_state->p_conn_params, SERVER_NOTIFY_GET_CLIENT_ADDR_FAILED, 
                NULL, "", 0);
    }
    return result;
}

static int32_t callback_establish(lws_connection_t *p_connection, 
        protocol_context_t *p_protocol_context, lws_connection_state_t *p_state, 
        void *p_in, uint32_t in_len)
{
    int32_t result = -1;

    if (false == p_protocol_context->is_pending_stop)
    {
        callback_base *p_callback = p_protocol_context->p_callback;

        p_state->p_queue = createQueue(
                static_cast<uint32_t>(p_protocol_context->total_buffer_size / sizeof(void*)));

        int32_t get_result = -1;

        if (NULL != p_state->p_queue &&
                NULL != (p_state->p_conn_params = p_callback->connection_create()) &&
                0 == (get_result = get_client_addr(p_state, p_connection, p_callback)) &&
                0 == prepare_conn_shared_state(p_state, p_protocol_context, p_connection) ) 
        {
            p_state->discard = false;
            p_callback->notify(p_state->p_conn_params, SERVER_NOTIFY_CONNECTION_ESTABLISHED, 
                    &(p_state->client_addr), "", 0);
            result = 0; 
        }
        else if (0 == get_result)
        {
            destroy_shared_ptr(p_state->sp_shared_state);
            p_callback->connection_destroy(p_state->p_conn_params);
            destroyQueue(p_state->p_queue);
            p_state->discard = true;
        }
        else if (NULL != p_state->p_conn_params)
        {
            p_callback->connection_destroy(p_state->p_conn_params);
            destroyQueue(p_state->p_queue);
            p_state->discard = true;
        }
        else if (NULL != p_state->p_queue)
        {
            destroyQueue(p_state->p_queue);
            p_state->discard = true;
        }
        else
        {
            p_state->discard = true;
        }
    }

    return result;
}

static void destroy_message(message_t *p_message)
{
    if (NULL != p_message)
    {
        free(p_message->p_sending_buf);
        free(p_message);
    }
}

static int32_t get_one_message(lws_connection_state_t *p_state, message_t **pp_message)
{
    int32_t result = -1;
    if (NULL != p_state->sp_shared_state)
    {
        // Note: handle data by working threads.
        pthread_spin_lock(&(p_state->sp_shared_state->lock));
        result = dequeue(p_state->p_queue, (void**)pp_message);
        pthread_spin_unlock(&(p_state->sp_shared_state->lock));
    }
    else
    {
        // Note: handle data by eventloop itself
        result = dequeue(p_state->p_queue, (void**)pp_message);
    }
    return result;
}

static void destroy_queued_messages(lws_connection_state_t *p_state)
{
    message_t *p_message = NULL;
    int32_t dequeue_result = -1;
    while(1)
    {
        dequeue_result = get_one_message(p_state, &p_message);
        if (0 == dequeue_result)
        {
            destroy_message(p_message);
        }
        else
        {
            break;
        }
    }
}

static void mark_conn_as_close(conn_shared_state_t *sp_shared_state)
{
    if (NULL != sp_shared_state)
    {
        sp_shared_state->p_connection = NULL;    
    }
}

static int32_t callback_close(lws_connection_t *p_connection, 
        protocol_context_t *p_protocol_context, lws_connection_state_t *p_state, 
        void *p_in, uint32_t in_len)
{
    if (false == p_state->discard)
    {
        callback_base *p_callback = p_protocol_context->p_callback;

        p_callback->notify(p_state->p_conn_params, SERVER_NOTIFY_CONNECTION_CLOSED, 
                &(p_state->client_addr), "", 0);

        mark_conn_as_close(p_state->sp_shared_state);

        // Note: if there is working thread, 
        // 'destroy_queued_messages' uses lock to synchronize with working thread.
        destroy_queued_messages(p_state);
        destroyQueue(p_state->p_queue);

        destroy_shared_ptr(p_state->sp_shared_state);

        p_callback->connection_destroy(p_state->p_conn_params);

        p_state->discard = true;
    }
    return 0;
}

static message_t *create_message(const char *p_data, uint32_t data_length)
{
    message_t *p_result = NULL;

    if (NULL != p_data && 0 < data_length)
    {
        unsigned char *p_sending_buf_temp = (unsigned char*)malloc(data_length + LWS_PRE);
        message_t *p_message_temp = (message_t*)malloc(sizeof(message_t));

        if (NULL != p_sending_buf_temp && NULL != p_message_temp)
        {
            memcpy(p_sending_buf_temp + LWS_PRE, p_data, data_length);
            p_message_temp->p_sending_buf = p_sending_buf_temp;
            p_message_temp->length = data_length;
            p_result = p_message_temp;
        }
        else
        {
            free(p_sending_buf_temp);
            free(p_message_temp);
        }
    }

    return p_result;
}

static async_message_t *create_async_message(lws_connection_state_t *p_state, 
        const char *p_data, uint32_t data_length)
{
    async_message_t *p_result = NULL;

    async_message_t *p_async_message_temp = (async_message_t*)malloc(sizeof(async_message_t));
    message_t *p_message_temp = create_message(p_data, data_length);

    if (NULL != p_async_message_temp && NULL != p_message_temp)
    {
        p_async_message_temp->sp_shared_state = 
            static_cast<conn_shared_state_t*>(copy_shared_ptr(p_state->sp_shared_state));
        p_async_message_temp->p_message = p_message_temp;
        p_result = p_async_message_temp;
    }
    else
    {
        free(p_async_message_temp);
        destroy_message(p_message_temp);
    }

    return p_result;
}

static void destroy_async_message(async_message_t *p_message)
{
    destroy_shared_ptr(p_message->sp_shared_state);
    destroy_message(p_message->p_message);
    free(p_message);
}

static void send_data_to_worker(protocol_context_t *p_protocol_context, 
        lws_connection_state_t *p_state, const char *p_data, uint32_t data_length)
{
    // Note: the connection state parameter is not used.
    async_message_t *p_message = 
        create_async_message(p_state, p_data, data_length);

    callback_base *p_callback = p_protocol_context->p_callback;

    if (NULL != p_message &&
            0 == send_data_to_thread_pool(p_protocol_context->p_thread_pool, 
                p_message, sizeof(async_message_t)))
    {
        goto SUCCESS;
    }
    else if (NULL != p_message)
    {
        goto ERROR_1;
    }
    else
    {
        goto ERROR_2;
    }

ERROR_1:
    destroy_async_message(p_message);
ERROR_2:
    p_callback->notify(p_state->p_conn_params, SERVER_NOTIFY_RECEIVE_WORKER_ERROR, 
            &(p_state->client_addr), p_data, data_length);
SUCCESS:
    return;
}

static int32_t store_sending_message(callback_base *p_callback, 
        lws_connection_state_t *p_state, message_t *p_message)
{
    int32_t result = enqueue(p_state->p_queue, p_message);
    if (0 != result)
    {
        p_callback->notify(p_state->p_conn_params, 
                SERVER_NOTIFY_RECEIVE_SELF_BUFFER_FULL, &(p_state->client_addr), 
                (const char*)(p_message->p_sending_buf + LWS_PRE), p_message->length);
    }
    return result;
}

static void handle_data_by_self(lws_connection_t *p_connection, 
        protocol_context_t *p_protocol_context, lws_connection_state_t *p_state, 
        const char *p_data, uint32_t data_length)
{
    callback_base *p_callback = p_protocol_context->p_callback;

    const char *p_sending_data = NULL;
    uint32_t sending_length = 0;
    p_callback->sync_request(p_state->p_conn_params, p_data, data_length,
            &p_sending_data, &sending_length);

    message_t *p_message_temp = create_message(p_sending_data, sending_length);
    if (NULL != p_message_temp && 
            0 == store_sending_message(p_callback, p_state, p_message_temp))
    {
        lws_callback_on_writable(p_connection);
    }
    else if (NULL != p_message_temp)
    {
        destroy_message(p_message_temp);
    }
    else
    {
        p_callback->notify(p_state->p_conn_params, 
                SERVER_NOTIFY_RECEIVE_SELF_REQUEST_ERROR, &(p_state->client_addr), 
                p_data, data_length);
    }
}

static void handle_data(lws_connection_t *p_connection, 
        protocol_context_t *p_protocol_context, lws_connection_state_t *p_state, 
        const char *p_data, uint32_t data_length)
{
    if (NULL != p_protocol_context->p_thread_pool)
    {
        send_data_to_worker(p_protocol_context, p_state, p_data, data_length);
    }
    else
    {
        handle_data_by_self(p_connection, p_protocol_context, p_state, p_data, data_length);
    }
}

static int32_t receive_data(lws_connection_t *p_connection, 
        protocol_context_t *p_protocol_context, lws_connection_state_t *p_state, 
        const char *p_data, uint32_t data_length)
{
    int32_t result = -1;
    // Note: Calling 'CLOSABLE' callback should not be before handling data.
    handle_data(p_connection, p_protocol_context, p_state, p_data, data_length);

    callback_base *p_callback = p_protocol_context->p_callback;
    bool is_close_connection = p_callback->is_closable(p_state->p_conn_params);
    if (false == is_close_connection)
    {
        result = 0;
    }
    return result;
}

static int32_t callback_receive_internal(lws_connection_t *p_connection, 
        protocol_context_t *p_protocol_context, lws_connection_state_t *p_state,
        void *p_in, uint32_t in_len)
{
    int32_t result = 0;
    callback_base *p_callback = p_protocol_context->p_callback;

    if (false == p_protocol_context->is_pending_stop)
    {
        p_callback->notify(p_state->p_conn_params, SERVER_NOTIFY_RECEIVE_DATA, 
                &(p_state->client_addr), (const char *)p_in, in_len);
        result = receive_data(p_connection, p_protocol_context, 
                p_state, (const char*)p_in, in_len);
    }
    else
    {
        p_callback->notify(p_state->p_conn_params, SERVER_NOTIFY_RECEIVE_DATA_IGNORED, 
                &(p_state->client_addr), (const char *)p_in, in_len);
        result = -1;
    }
    return result;
}

static int32_t callback_receive(lws_connection_t *p_connection, 
        protocol_context_t *p_protocol_context, lws_connection_state_t *p_state,
        void *p_in, uint32_t in_len)
{
    int32_t result = -1;
    if (false == p_state->discard)
    {
        result = callback_receive_internal(p_connection, p_protocol_context, p_state,
                p_in, in_len);
    }
    return result;
}

static void send_data_to_lws(lws_connection_t *p_connection, 
        callback_base *p_callback, lws_connection_state_t *p_state,
        unsigned char *p_data, uint32_t data_length,
        lws_write_method write_method)
{
    p_callback->notify(p_state->p_conn_params, SERVER_NOTIFY_SEND_DATA, 
            &(p_state->client_addr), (const char *)p_data, data_length);

    // During LWS sending data (including remainder data), WRITABLE callback will be suppressed.
    int32_t write_count = lws_write(p_connection, p_data, data_length, write_method);
    if (0 > write_count || (uint32_t)write_count < data_length)
    {
        lwsl_err("write failed\n");
        p_callback->notify(p_state->p_conn_params, SERVER_NOTIFY_SEND_DATA_ERROR, 
                &(p_state->client_addr), (const char *)p_data, data_length);
    }
}

static int32_t write_data(lws_connection_t *p_connection,
        callback_base *p_callback, lws_connection_state_t *p_state, 
        unsigned char *p_data, uint32_t data_length,
        lws_write_method write_method)
{
    int32_t result = -1;
    // Note: Calling 'CLOSABLE' callback should not be before sending data.
    send_data_to_lws(p_connection, p_callback, p_state, p_data, data_length, write_method);
    bool is_close_connection = p_callback->is_closable(p_state->p_conn_params);
    if (false == is_close_connection)
    {
        result = 0;
    }
    return result;
}

static int32_t no_message(lws_connection_state_t *p_state)
{
    int32_t result = -1;
    if (NULL != p_state->sp_shared_state)
    {
        // Note: handle data by working threads.
        pthread_spin_lock(&(p_state->sp_shared_state->lock));
        result = isQueueEmpty(p_state->p_queue);
        pthread_spin_unlock(&(p_state->sp_shared_state->lock));
    }
    else
    {
        // Note: handle data by eventloop itself
        result = isQueueEmpty(p_state->p_queue);
    }
    return result;

}

static void handle_next_message(lws_connection_t *p_connection, 
        lws_connection_state_t *p_state)
{
    if (0 != no_message(p_state))
    {
        // Note: call writable again to ensure following data can be sent.
        lws_callback_on_writable(p_connection);
    }
}

static int32_t callback_writable_internal(lws_connection_t *p_connection, 
        protocol_context_t *p_protocol_context, lws_connection_state_t *p_state, 
        void *p_in, uint32_t in_len, lws_write_method write_method)
{
    int32_t result = 0;

    message_t *p_message = NULL;
    int32_t get_result = get_one_message(p_state, &p_message);

    if (0 == get_result)
    {
        //lwsl_notice("send %u bytes to client : %s on conn %p\n", 
        //        p_message->length, (char*)p_message->p_sending_buf + LWS_PRE, p_connection);

        callback_base *p_callback = p_protocol_context->p_callback;

        // Note: If data will be written as data type other than BTYE, padding LWS_PRE to the CPU word size 
        // is necessary. This makes references to address of data, which is immediately after the padding,
        // won't cause an unaligned access error. 
        // Sometimes for performance reasons the recommended padding is even larger than sizeof(void *).
        result = write_data(p_connection, p_callback, p_state, p_message->p_sending_buf + LWS_PRE, 
                p_message->length, write_method);

        p_protocol_context->is_data_sending = true;

        destroy_message(p_message);

        handle_next_message(p_connection, p_state);
    }

    return result;
}

static int32_t callback_writable(lws_connection_t *p_connection, 
        protocol_context_t *p_protocol_context, lws_connection_state_t *p_state, 
        void *p_in, uint32_t in_len, lws_write_method write_method)
{
    int32_t result = -1;
    if (false == p_state->discard)
    {
        result = callback_writable_internal(p_connection, p_protocol_context, p_state, 
                p_in, in_len, write_method);
    }
    return result;
}

static protocol_context_t *get_protocol_context(lws_connection_t *p_connection)
{
    protocol_context_t *p_result = NULL;
    const struct lws_protocols *p_protocol = lws_get_protocol(p_connection);
    if (NULL != p_protocol)
    {
        p_result = (protocol_context_t*)(p_protocol->user);
    }
    return p_result;
}

static int32_t callback_websockets(lws_connection_t *p_connection, 
        lws_callback_reason_t reason, void *p_connection_data, void *p_in, size_t in_len)
{
    int32_t result = 0;
    protocol_context_t *p_protocol_context = get_protocol_context(p_connection);
    lws_connection_state_t *p_state = (lws_connection_state_t*)p_connection_data;

    switch (reason) {

        case LWS_CALLBACK_ESTABLISHED:
            lwsl_notice("virus_checking: LWS_CALLBACK_ESTABLISHED\n");
            result = callback_establish(p_connection, p_protocol_context, p_state, 
                    p_in, (uint32_t)in_len);
            break;

        case LWS_CALLBACK_CLOSED:
            lwsl_notice("virus_checking: LWS_CALLBACK_CLOSED\n");
            result = callback_close(p_connection, p_protocol_context, p_state, 
                    p_in, (uint32_t)in_len);
            break;

        case LWS_CALLBACK_SERVER_WRITEABLE:
            lwsl_notice("virus_checking: LWS_CALLBACK_SERVER_WRITEABLE\n");
            result = callback_writable(p_connection, p_protocol_context, p_state, 
                    p_in, (uint32_t)in_len, LWS_WRITE_BINARY);
            break;

        case LWS_CALLBACK_RECEIVE:
            lwsl_notice("virus_checking: LWS_CALLBACK_RECEIVE\n");
            result = callback_receive(p_connection, p_protocol_context, p_state, 
                    p_in, (uint32_t)in_len);
            break;

            /*
             * this just demonstrates how to handle
             * LWS_CALLBACK_WS_PEER_INITIATED_CLOSE and extract the peer's close
             * code and auxiliary data.  You can just not handle it if you don't
             * have a use for this.
             */
        case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
            lwsl_notice("LWS_CALLBACK_WS_PEER_INITIATED_CLOSE: len %lu\n", (unsigned long)in_len);
            {
                size_t i = 0;
                for (i = 0; i < in_len; ++i)
                {
                    lwsl_notice(" %lu: 0x%02X\n", i, ((unsigned char *)p_in)[i]);
                }
            }
            break;

        default:
            break;
    }

    return result;
}

/* list of supported protocols and callbacks which are based on websockets protocol*/
enum valid_websockets_protocols
{
    /* always first */
    PROTOCOL_HTTP = 0,

    PROTOCOL_WEBSOCKTS,

    /* always last */
    WEBSOCKETS_PROTOCOL_COUNT
};

static struct lws_protocols websockets_protocols[] = {
    /* first protocol must always be HTTP handler */
    {
        "http-only",        /* name */
        callback_http,        /* callback */
        sizeof (struct per_session_data__http),    /* per_session_data_size */
        0,            /* max frame size / rx buffer */
    },
    {
        "websockets_protocol",
        callback_websockets,
        sizeof(lws_connection_state_t),
        INNER_RECEIVE_BUFFER_SIZE,
    },
    { NULL, NULL, 0, 0 } /* terminator */
};

static int32_t callback_raw_socket(lws_connection_t *p_connection, 
        lws_callback_reason_t reason, void *p_connection_data, void *p_in, size_t in_len)
{
    int32_t result = 0;
    protocol_context_t *p_protocol_context = get_protocol_context(p_connection);
    lws_connection_state_t *p_state = (lws_connection_state_t*)p_connection_data;

    switch (reason) {


        case LWS_CALLBACK_RAW_ADOPT:
            lwsl_notice("raw_socket_test: LWS_CALLBACK_RAW_ADOPT\n");
            result = callback_establish(p_connection, p_protocol_context, p_state, 
                    p_in, (uint32_t)in_len);
            break;

        case LWS_CALLBACK_RAW_CLOSE:
            lwsl_notice("raw_socket_test: LWS_CALLBACK_RAW_CLOSE\n");
            result = callback_close(p_connection, p_protocol_context, p_state, 
                    p_in, (uint32_t)in_len);
            break;

        case LWS_CALLBACK_RAW_WRITEABLE:
            lwsl_notice("raw_socket_test: LWS_CALLBACK_RAW_WRITEABLE\n");
            // Note: LWS_WRITE_HTTP LWS_WRITE_HTTP_FINAL LWS_WRITE_HTTP_HEADERS ...
            // Strange names for raw socket sending ......
            // Those enum names were got from the implementation in libwebsockets/lib/output.c
            // and libwebsockets/plugins/protocol_lws_raw_test.c
            result = callback_writable(p_connection, p_protocol_context, p_state, 
                    p_in, (uint32_t)in_len, LWS_WRITE_HTTP);
            break;

        case LWS_CALLBACK_RAW_RX:
            lwsl_notice("raw_socket_test: LWS_CALLBACK_RAW_RX\n");
            result = callback_receive(p_connection, p_protocol_context, p_state, 
                    p_in, (uint32_t)in_len);
            break;
        default:
            break;
    }

    return result;
}

/* list of supported protocols and callbacks which are based on raw socket*/
enum valid_raw_socket_protocols
{
    PROTOCOL_RAW_SOCKET,

    /* always last */
    RAW_SOCKET_PROTOCOL_COUNT
};

static struct lws_protocols raw_socket_protocols[] = {
    {
        "raw_socket_protocol",
        callback_raw_socket,
        sizeof(lws_connection_state_t),
        INNER_RECEIVE_BUFFER_SIZE,
    },
    { NULL, NULL, 0, 0 } /* terminator */
};

void sighandler(int32_t signo, void *p_param_1, void *p_param_2)
{
    g_force_exit = 1;
    lws_cancel_service(g_p_server_context);
}

static const lws_protocol_extension_t protocol_extensions[] = 
{
    {
        "permessage-deflate",
        lws_extension_callback_pm_deflate,
        "permessage-deflate"
    },
    {
        "deflate-frame",
        lws_extension_callback_pm_deflate,
        "deflate_frame"
    },
    { NULL, NULL, NULL /* terminator */ }
};

static int32_t check_working_thread(lws_server_wt_t *p_working_thread)
{
    int32_t result = -1;
    uint8_t count = p_working_thread->count;
    if (0 == count ||
            (0 < count && 31 >= count && NULL != p_working_thread->pp_params))
    {
        result = 0;
    }
    return result;
}

static int32_t check_callbacks(callback_base *p_callback)
{
    int32_t result = -1;
    if (NULL != p_callback)
    {
        result = 0;
    }
    return result;
}

static int32_t check_params(lws_server_evloop_info_t *p_evloop_info)
{
    int32_t result = -1;
    if (NULL != p_evloop_info &&
            0 != p_evloop_info->port &&
            0 == check_callbacks(p_evloop_info->p_callback) &&
            0 == check_working_thread(&(p_evloop_info->working_thread)) )
    {
        result = 0;    
    }
    return result;
}

static void do_async_request(thread_context_t *p_context, async_message_t *p_async_message)
{
    if (NULL != p_async_message->sp_shared_state->p_connection)
    {
        message_t *p_message = p_async_message->p_message;
        unsigned char *p_data = p_message->p_sending_buf + LWS_PRE;
        uint32_t data_length = p_message->length;

        void *p_async_params = p_context->p_async_params;

        p_context->p_callback->async_request(p_async_params, 
                p_async_message->sp_shared_state->p_conn_params, 
                p_async_message->sp_shared_state, (const char *)p_data, data_length);
    }
}

static bool handle_one_async_message(thread_context_t *p_context)
{
    bool result = false;

    async_message_t *p_async_message = NULL;
    pthread_spin_lock(&(p_context->lock));
    int32_t dequeue_result = dequeue(p_context->p_queue, (void**)(&p_async_message));
    pthread_spin_unlock(&(p_context->lock));

    if (0 == dequeue_result)
    {
        do_async_request(p_context, p_async_message);
        destroy_async_message(p_async_message);
        result = true;
    }

    return result;
}

static int32_t send_message_to_queue(lws_connection_t *p_connection, message_t *p_message)
{
    // Note: in libwebsockets 2.3.0, 'lws_wsi_user', which just returns a pointer,
    // is thread-safe.
    // And in current situation, this pointer can not be NULL.
    lws_connection_state_t *p_state = 
        (lws_connection_state_t*)lws_wsi_user(p_connection);

    int32_t result = -1;
    if (NULL != p_state &&
            0 == (result = enqueue(p_state->p_queue, p_message)) )
    {
        // Note: 'lws_callback_on_writable' is thread-safe.
        lws_callback_on_writable(p_connection);
    }
    return result;
}

static void send_message_to_connection(conn_shared_state_t *sp_shared_state, message_t *p_message)
{
    int32_t enqueue_result = -1;
    while (0 != enqueue_result)
    {
        lws_connection_t *p_connection = sp_shared_state->p_connection;
        // Note: 'lock-if'
        pthread_spin_lock(&(sp_shared_state->lock));
        if (NULL != p_connection)
        {
            enqueue_result = send_message_to_queue(p_connection, p_message);
        }
        else
        {
            enqueue_result = 0;
        }
        pthread_spin_unlock(&(sp_shared_state->lock));
    }
}

static void notify_async_result_error(thread_context_t *p_context, 
        conn_shared_state_t *sp_shared_state, const char *p_data, uint32_t data_length)
{
    lws_connection_t *p_connection = sp_shared_state->p_connection;
    // Note: 'lock-if'
    pthread_spin_lock(&(sp_shared_state->lock));
    if (NULL != p_connection)
    {
        lws_connection_state_t *p_state = 
            (lws_connection_state_t*)lws_wsi_user(p_connection);
        p_context->p_callback->notify(p_state->p_conn_params, 
                SERVER_NOTIFY_WORKER_ASYNC_RESULT_ERROR, &(p_state->client_addr), 
                p_data, data_length);
    }
    pthread_spin_unlock(&(sp_shared_state->lock));
}

static void do_async_result(thread_context_t *p_context, conn_shared_state_t *sp_shared_state, 
        const char *p_data, uint32_t data_length)
{
    message_t *p_message = create_message(p_data, data_length);
    if (NULL != p_message)
    {
        send_message_to_connection(sp_shared_state, p_message);
    }
    else
    {
        notify_async_result_error(p_context, sp_shared_state, p_data, data_length);
    }
}

static bool handle_one_async_result(thread_context_t *p_context)
{
    bool result = false;

    conn_shared_state_t *sp_shared_state;
    const char *p_data = NULL;
    uint32_t data_length = 0;
    p_context->p_callback->async_result(p_context->p_async_params, 
            (void**)&sp_shared_state, &p_data, &data_length);

    // Note: there is a if--lock--if pattern.
    // The first 'if' is here (NULL != sp_shared_state->p_connection), 
    // The 'lock' and second 'if' are in 'send_message_to_connection' and 'notify_async_result_error'.
    if (NULL != sp_shared_state && NULL != sp_shared_state->p_connection 
            && NULL != p_data && 0 < data_length)
    {
        do_async_result(p_context, sp_shared_state, p_data, data_length);
        result = true;
    }

    return result;
}

static void *run_thread(void *p_params)
{
    thread_context_t *p_context = (thread_context_t*)p_params;

    while(1)
    {
        bool has_message = handle_one_async_message(p_context);
        bool has_result = handle_one_async_result(p_context);

        if (false == has_message && false == has_result)
        {
            // Sleep 0.5 second.
            sleep_thread(0, 500000000);
        }
    }

    return NULL;
}

static int32_t send_data_to_thread(void *p_params, void *p_data, uint32_t data_length)
{
    thread_context_t *p_context = (thread_context_t*)p_params;
    async_message_t *p_message = (async_message_t*)p_data;

    int32_t enqueue_result = -1;
    while (0 != enqueue_result)
    {
        pthread_spin_lock(&(p_context->lock));
        enqueue_result = enqueue(p_context->p_queue, p_message);
        pthread_spin_unlock(&(p_context->lock));
    }
    return enqueue_result;
}

static uint8_t schedule_thread_data(void *p_params, 
        void *p_data, uint32_t data_length)
{
    td_scheduler_params_t *p = (td_scheduler_params_t*)p_params;
    uint8_t result = p->curr_thread_index;
    p->curr_thread_index = (uint8_t)((result + 1) % p->max_thread_count);
    return result;
}

static thread_pool_t *initialize_thread_pool(lws_server_evloop_t *p_evloop, 
        lws_server_evloop_info_t *p_evloop_info, void **pp_thread_params_array)
{
    p_evloop->protocol_context.scheduler_params.max_thread_count = 
        p_evloop_info->working_thread.count;
    p_evloop->protocol_context.scheduler_params.curr_thread_index = 0;

    thread_pool_params_t params;
    params.schedule = schedule_thread_data;
    params.p_schedule_params = &(p_evloop->protocol_context.scheduler_params);
    params.run = run_thread;
	params.p_thread_attr = NULL;
    params.send = send_data_to_thread;
    params.pp_thread_params_array = pp_thread_params_array;
    params.thread_count = p_evloop_info->working_thread.count;

    return create_thread_pool(&params);
}

static void destroy_thread_contexts(thread_context_t **pp_thread_context_ptrs, uint8_t count)
{
    uint32_t i = 0;
    for (; i < count; ++i)
    {
        destroyQueue(pp_thread_context_ptrs[i]->p_queue);
        pthread_spin_destroy(&(pp_thread_context_ptrs[i]->lock));
        free(pp_thread_context_ptrs[i]);
    }
}

static int32_t initialize_thread_context_ptrs(thread_context_t **pp_thread_context_ptrs, 
    uint8_t count, lws_server_evloop_info_t *p_evloop_info)
{
    int32_t result = 0;

    // Note: clear memory should be done first !
    memset(pp_thread_context_ptrs, 0, count * sizeof(thread_context_t*));

    uint32_t i = 0;
    for (; i < count; ++i)
    {
        thread_context_t *p_context_temp = (thread_context_t*)malloc(sizeof(thread_context_t));

        uint32_t queue_space = 
            round_up_u32(p_evloop_info->hint_max_sending_size, BUFFER_BLOCK_SIZE); 
        struct Queue *p_queue_temp = createQueue(
                static_cast<uint32_t>(queue_space / sizeof(void*)));

        if (NULL != p_context_temp && 
                NULL != p_queue_temp && 
                0 == pthread_spin_init(&(p_context_temp->lock), 0) )
        {
            p_context_temp->p_queue = p_queue_temp;
            p_context_temp->p_callback = p_evloop_info->p_callback;
            p_context_temp->p_async_params = p_evloop_info->working_thread.pp_params[i];
            // TODO: initialize other data member here.

            pp_thread_context_ptrs[i] = p_context_temp;
        }
        else
        {
            destroy_thread_contexts(pp_thread_context_ptrs, count);
            destroyQueue(p_queue_temp);
            result = -1;
            break;
        }
    }

    return result;
}

static thread_context_t **initialize_thread_contexts(lws_server_evloop_info_t *p_evloop_info)
{
    thread_context_t **pp_result = NULL;

    uint8_t count = p_evloop_info->working_thread.count;
    thread_context_t **pp_thread_context_ptr_array_temp = 
        (thread_context_t **)malloc(count * sizeof(thread_context_t*));
    if (NULL != pp_thread_context_ptr_array_temp &&
            0 == initialize_thread_context_ptrs(pp_thread_context_ptr_array_temp, 
                count, p_evloop_info))
    {
        pp_result = pp_thread_context_ptr_array_temp;
    }
    else if (NULL != pp_thread_context_ptr_array_temp)
    {
        free(pp_thread_context_ptr_array_temp);
    }
    else
    {
        // Do nothing.
    }

    return pp_result;
}

static int32_t initialize_working_threads(lws_server_evloop_t *p_evloop, 
        lws_server_evloop_info_t *p_evloop_info)
{
    int32_t result = -1;

    thread_context_t **pp_thread_contexts_temp = initialize_thread_contexts(p_evloop_info);
    thread_pool_t *p_thread_pool_temp = NULL;
    if (NULL != pp_thread_contexts_temp &&
            NULL != (p_thread_pool_temp = initialize_thread_pool(p_evloop, p_evloop_info, 
                    (void**)pp_thread_contexts_temp)))
    {
        p_evloop->protocol_context.p_thread_pool = p_thread_pool_temp;
        p_evloop->protocol_context.pp_thread_contexts = pp_thread_contexts_temp;
        p_evloop->protocol_context.thread_context_count = p_evloop_info->working_thread.count;
        result = 0;
    }
    else if (NULL != pp_thread_contexts_temp)
    {
        free(pp_thread_contexts_temp);
    }
    else
    {
        // Do nothing.
    }

    return result;
}

static int32_t prepare_working_threads(lws_server_evloop_t *p_evloop, 
        lws_server_evloop_info_t *p_evloop_info)
{
    int32_t result = -1;
    if (0 < p_evloop_info->working_thread.count)
    {
        result = initialize_working_threads(p_evloop, p_evloop_info);
    }
    else
    {
        p_evloop->protocol_context.p_thread_pool = NULL;
        p_evloop->protocol_context.pp_thread_contexts = NULL;
        p_evloop->protocol_context.thread_context_count = 0;
        result = 0;
    }
    return result;
}

static void prepare_protocols_by_current_group(lws_server_evloop_t *p_evloop, 
        lws_protocol_group_t protocol_group)
{
    switch(protocol_group)
    {
        case PROTOCOL_GROUP_BASED_ON_WEBSOCKETS:
            websockets_protocols[PROTOCOL_WEBSOCKTS].user = &(p_evloop->protocol_context);
            break;
        case PROTOCOL_GROUP_BASED_ON_RAW_SOCKET:
            raw_socket_protocols[PROTOCOL_RAW_SOCKET].user = &(p_evloop->protocol_context);
            break;
        default:
            break;
    }
}

// TODO: Each protocol can be initialize during LWS_CALLBACK_PROTOCOL_INIT event,
// and set initialized data to lws_protocols::user during that event. 
// Those data which is used in initialization can come from lws_context_creation_info::user.
static int32_t prepare_protocols(lws_server_evloop_t *p_evloop, 
        lws_server_evloop_info_t *p_evloop_info)
{
    p_evloop->protocol_context.is_data_sending = false;
    p_evloop->protocol_context.is_pending_stop = false;
    p_evloop->protocol_context.p_callback = p_evloop_info->p_callback;
    // If 'total_buffer_size' is used to store bytes stream, do not forget add a header 'LWS_PRE'.
    // Otherwise, it is not necessary to pend 'LWS_PRE' space, such as storing pointers.
    p_evloop->protocol_context.total_buffer_size = 
        round_up_u32(p_evloop_info->hint_max_sending_size + (uint32_t)LWS_PRE, BUFFER_BLOCK_SIZE);
    prepare_protocols_by_current_group(p_evloop, p_evloop_info->protocol_group);

    return prepare_working_threads(p_evloop, p_evloop_info);
}

static lws_protocol_t *get_current_protocols(lws_protocol_group_t protocol_group)
{
    lws_protocol_t *p_result = NULL;
    switch(protocol_group)
    {
        case PROTOCOL_GROUP_BASED_ON_WEBSOCKETS:
            p_result = websockets_protocols;
            break;
        case PROTOCOL_GROUP_BASED_ON_RAW_SOCKET:
            p_result = raw_socket_protocols;
            break;
        default:
            break;
    }
    return p_result;
}

static lws_server_context_t *prepare_server(lws_server_evloop_t *p_evloop, 
        lws_server_evloop_info_t *p_evloop_info)
{
    lws_server_context_t *p_result = NULL;

    lws_server_context_creation_info_t creation_info;
    memset(&creation_info, 0, sizeof(creation_info));

    creation_info.iface = p_evloop_info->p_iface; 
    creation_info.port = p_evloop_info->port;
    creation_info.protocols = get_current_protocols(p_evloop_info->protocol_group);
    creation_info.extensions = protocol_extensions;
    creation_info.gid = -1;
    creation_info.uid = -1;
    creation_info.ws_ping_pong_interval = PING_PONG_INTERVAL;
    // If ka_time does not work, 
    // try to use lws_set_timeout(wsi, reason, secs) with 'reason' PENDING_TIMEOUT_CLOSE_SEND
    //creation_info.ka_time = 
    //    (0 == p_evloop_info->dead_timeout) ? 300 : p_evloop_info->dead_timeout; // 5 minutes.
    //creation_info.ka_probes = 1;
    //creation_info.ka_interval = 1;
    creation_info.timeout_secs = 5;

    creation_info.count_threads = 1; // Only one service thread, aka single thread mode.
    // multi-thread mode of libwebsockets 2.3.0 has crashing problems.

    creation_info.max_http_header_pool = 16;
    // TODO: when working in raw socket mode, 
    // use 'LWS_SERVER_OPTION_ONLY_RAW' instead of 'LWS_SERVER_OPTION_FALLBACK_TO_RAW'.
    creation_info.options = LWS_SERVER_OPTION_VALIDATE_UTF8 | LWS_SERVER_OPTION_EXPLICIT_VHOSTS |
        LWS_SERVER_OPTION_FALLBACK_TO_RAW
#if defined(LWS_OPENSSL_SUPPORT)
        | LWS_SERVER_OPTION_REQUIRE_VALID_OPENSSL_CLIENT_CERT
#endif
        ;
    //context_info.ip_limit_ah = 4; /* for testing */
    //context_info.ip_limit_wsi = 105; /* for testing */

    // TODO: daemonize

    lws_server_context_t *p_context = NULL;
    lws_server_vhost_t *p_vhost = NULL;
    if (0 == prepare_protocols(p_evloop, p_evloop_info) &&
            NULL != (p_context = lws_create_context(&creation_info)) &&
            NULL != (p_vhost = lws_create_vhost(p_context, &creation_info)) )
    {
        p_result = p_context;
        g_p_server_context = p_context;

        // TODO: how to get signal in non-main thread?
        // I guess signals can only be got in main thread. And when it is got, main thread invokes 'stop_server_evloop'.
        handle_signal(SIGINT, sighandler, false, false);

        /* tell the library what debug level to emit and to send it to syslog */
        lws_set_log_level(g_debug_level, NULL);

        /* we will only try to log things according to our debug_level */
#ifdef __sun
        //int syslog_options = LOG_PID;
#else	     
        //int syslog_options = LOG_PID | LOG_PERROR;
#endif
        //setlogmask(LOG_UPTO (LOG_DEBUG));
        //openlog("lwsts", syslog_options, LOG_DAEMON);
        //lws_set_log_level(g_debug_level, lwsl_emit_syslog);
    }

    return p_result;
}

static lws_server_evloop_t *create_evloop_internal(lws_server_evloop_info_t *p_evloop_info)
{
    lws_server_evloop_t *p_result = NULL;

    lws_server_evloop_t *p_temp = NULL;
    lws_server_context_t *p_context_temp = NULL;

    if (NULL != (p_temp = (lws_server_evloop_t*)malloc(sizeof(lws_server_evloop_t))) && 
            NULL != (p_context_temp = prepare_server(p_temp, p_evloop_info)) )
    {
        p_temp->p_server_context = p_context_temp;
        // 'protocol_context' is initialized in 'prepare_server()->prepare_protocols()'
        p_temp->stop = NONE;
        p_result = p_temp; 
    }
    else if (NULL != p_temp)
    {
        free(p_temp);
    }
    else
    {
        // Do nothing.
    }

    return p_result;
}

static void destroy_protocol_contexts(protocol_context_t *p_protocol_context)
{
    destroy_thread_pool(p_protocol_context->p_thread_pool);
    destroy_thread_contexts(p_protocol_context->pp_thread_contexts, 
            p_protocol_context->thread_context_count);
    free(p_protocol_context->pp_thread_contexts);
}

static bool reset_data_sending_state(lws_server_evloop_t *p_evloop)
{
    bool old_state = p_evloop->protocol_context.is_data_sending;
    p_evloop->protocol_context.is_data_sending = false;
    return old_state;
}

static void launch_evloop_internal(lws_server_evloop_t *p_evloop, int32_t blocking_timeout)
{
    int32_t service_result = 0;
    while (0 <= service_result && 0 == g_force_exit) 
    {
        // Send all pending data when not "force stopped"
        if ((STOP_FORCE == p_evloop->stop) ||
                (STOP_REQUEST == p_evloop->stop && false == reset_data_sending_state(p_evloop)) )
        {
            // tell 'stop_server_evloop' that this event loop really stops.
            p_evloop->stop = NONE;
            break;
        }
        service_result = lws_service(p_evloop->p_server_context, blocking_timeout);
    }
    lwsl_info("Exiting\n");
}

//////////////////////////////////////////////////////////////
// Public Interfaces
//////////////////////////////////////////////////////////////
LIB_PUBLIC lws_server_evloop_t *create_server_evloop(lws_server_evloop_info_t *p_evloop_info)
{
    lws_server_evloop_t *p_result = NULL;
    if (0 == check_params(p_evloop_info))
    {
        p_result = create_evloop_internal(p_evloop_info);
    }
    return p_result;
}

LIB_PUBLIC void destroy_server_evloop(lws_server_evloop_t *p_evloop)
{
    if (NULL != p_evloop)
    {
        lws_context_destroy(p_evloop->p_server_context);

        destroy_protocol_contexts(&(p_evloop->protocol_context));

        // If use system log facilities, stop logging here.
        //closelog();

        free(p_evloop);
    }
}

LIB_PUBLIC void launch_server_evloop(lws_server_evloop_t *p_evloop, int32_t blocking_timeout)
{
    if (NULL != p_evloop)
    {
        launch_evloop_internal(p_evloop, blocking_timeout);
    }
}

LIB_PUBLIC void stop_server_evloop(lws_server_evloop_t *p_evloop, bool is_by_force)
{
    if (NULL != p_evloop)
    {
        if (true == is_by_force)
        {
            // No need to lock the writing of 'stop'.
            p_evloop->stop = STOP_FORCE;
            p_evloop->protocol_context.is_pending_stop = true;
            // NOTE: 'lws_cancel_service' is thread-safe.
            lws_cancel_service(p_evloop->p_server_context);
        }
        else
        {
            // No need to lock the writing of 'stop'.
            p_evloop->stop = STOP_REQUEST;
            p_evloop->protocol_context.is_pending_stop = true;
        }

        // Poll here infinitely, until event loop really stops.
        while(NONE != p_evloop->stop);
    }
}

