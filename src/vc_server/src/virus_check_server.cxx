
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <string>
#include <queue>

#include "debug_print.h"
#include "vc_server_defines.h"
#include "utils/utils_library.h"
#include "module_data.h"
                            
static const char *message_types[] = {"receive_token"}; // temp test 

LIB_PUBLIC module_t *create(uint32_t category, 
            notify_scheduler_t notifier, void *p_params, uint32_t arg_count, const char **p_args)
{
    if(arg_count < 2 || NULL == p_args)
        return NULL;

    module *p_result = new module;

    if (NULL != p_result)
    {
        p_result->category = category;
        p_result->notifier = notifier;
        p_result->p_params = p_params;

        p_result->wt_count = atoi(p_args[0]);
        p_result->port = atoi(p_args[1]);
        
        p_result->p_ipc_connect = NULL;
        p_result->p_cache_data_queue = new recdata_queue_t;
    }
    return p_result;
}

LIB_PUBLIC void get_inputted_message_type(module_t *p_module, 
            const char ***const ppp_inputted_message_types, uint32_t *p_message_type_count)
{
    *ppp_inputted_message_types = message_types;

    *p_message_type_count = sizeof(message_types) / sizeof(message_types[0]);
}

LIB_PUBLIC module_state_t run(module_t *p_module)
{
    if (NULL == p_module)
        return MODULE_ERROR;

    ipc_connect_t *p_ipc_connect = create_ipc_connect(p_module->wt_count, 
                                            p_module->port, (void*)p_module->p_cache_data_queue);
    if(NULL == p_ipc_connect){
        delete p_module; p_module = NULL;
        return MODULE_ERROR;
    }
    else
        p_module->p_ipc_connect = p_ipc_connect;

    vc_server_log(p_module, "Virus check server start now");

    return MODULE_OK;	
}

LIB_PUBLIC module_state_t stop(module_t *p_module)
{
    if (NULL == p_module)
        return MODULE_ERROR;

    return MODULE_OK;
}

LIB_PUBLIC void destroy(module_t *p_module)
{
    if (NULL != p_module)
    {
        if(p_module->p_ipc_connect) {
            destroy_ipc_connect(p_module->p_ipc_connect);
        }
        if(p_module->p_cache_data_queue) {
            delete p_module->p_cache_data_queue; p_module->p_cache_data_queue = NULL;
        }
        delete p_module; p_module = NULL;
    }
}

LIB_PUBLIC module_data_t* assign(module_t *p_module, const module_data_t *p_data, bool is_sync)
{
    // nothing to do
    return NULL;
}
