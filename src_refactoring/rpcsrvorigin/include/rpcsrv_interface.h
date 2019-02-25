#ifndef _RPC_MODULE_
#define _RPC_MODULE_
#include <string>
#include "module_interfaces.h"

#define _export_ __attribute__((visibility("default")))

enum
{
    KV_ENGINE_MSG_TYPE_MIN = 1234,
	KV_ENGINE_MSG_TYPE_ENABLE_FILE_MONITOR,
	KV_ENGINE_MSG_TYPE_ENABLE_PROC_MONITOR,
	KV_ENGINE_MSG_TYPE_ENABLE_NET_MONITOR,
	KV_ENGINE_MSG_TYPE_GET_ASSETS,
	KV_ENGINE_MSG_TYPE_CENTER_AGENT_POST,
	KV_ENGINE_MSG_TYPE_RPCSRV,
    KV_ENGINE_MSG_TYPE_MAX
};

/* every msg receive from this module should contents msg_cmd in g_p_message_id */
static std::string msg_req_cmd("msg_req_cmd");
/* every msg response to this module must specified msg_result_cmd into g_p_message_id */
static std::string msg_result_cmd("msg_result_cmd");

/* msg contents key */
static std::string msg_id_key("id");
static std::string msg_type_key("type");
static std::string msg_payload_key("payload");


extern "C"
{
    _export_ module_t *create(uint32_t category, notify_scheduler_t notifier, void *p_params, uint32_t arg_count, const char **p_args);
    _export_ void destroy(module_t *p_module);
    _export_ module_state_t run(module_t *p_module);
    _export_ module_state_t stop(module_t *p_module);
    _export_ module_data_t *assign(module_t *p_module, const module_data_t *p_data, bool is_sync);
    _export_ void get_inputted_message_type(module_t *p_module, 
        const char *** const ppp_inputted_message_types, uint32_t *p_message_type_count);
}

#endif
