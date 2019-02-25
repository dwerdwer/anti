#ifndef VC_CLIENT_FUNCTION_HHH
#define VC_CLIENT_FUNCTION_HHH

#include <stdint.h>

extern "C" {

typedef struct rpc_client rpc_client_t;

typedef void (*notifier_t)(const char *p_result, uint32_t result_length, void *p_params);

typedef union send_params
{
    struct
    {
        char *p_result_buffer;
        uint32_t result_buffer_length;
    }sync_params;
    struct
    {
        notifier_t notifier;
        void *p_params;
    }async_params;

}send_params_t;


rpc_client_t *create_rpc_client(const char* host, int16_t port);


void destroy_rpc_client(rpc_client_t* p_client);

int32_t send_to_server(rpc_client_t* p_client, int32_t cmd_id, 
            int32_t sub_cmd_id, const char *p_data, uint32_t data_length, send_params_t *p_params);

}

#endif
