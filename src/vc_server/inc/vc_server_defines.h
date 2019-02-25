
#ifndef VC_SERVER_INTERFACES_HHH
#define VC_SERVER_INTERFACES_HHH

#include <stdint.h>
#include <string>
#include <queue>

#include "module_interfaces.h"
#include "vc_server_function.h"

typedef std::pair<std::string, std::string> member_t;
typedef std::queue<member_t> recdata_queue_t;

struct module 
{
    uint32_t category;
    notify_scheduler_t notifier;
    void *p_params;
 
    uint32_t wt_count;
    uint32_t port;
    ipc_connect_t *p_ipc_connect;
    recdata_queue_t *p_cache_data_queue;
};
extern "C" {

module_t *create(uint32_t category, 
            notify_scheduler_t notifier, void *p_params, uint32_t arg_count, const char **p_args);


void get_inputted_message_type(module_t *p_module, 
            const char ***const ppp_inputted_message_types, uint32_t *p_message_type_count);

module_state_t run(module_t *p_module);

module_state_t stop(module_t *p_module);

void destroy(module_t *p_module);

module_data_t* assign(module_t *p_module, const module_data_t *p_data, bool is_sync);

}

#endif

