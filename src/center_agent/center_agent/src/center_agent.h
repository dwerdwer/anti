//center_agent api.h
#ifndef _CENTER_AGENT_H_
#define _CENTER_AGENT_H_
#include <stdint.h>
//#include "center_agent_def.h"
#include "module_interfaces.h"
#ifdef __cplusplus
extern "C"
{
#endif

module_t *create(uint32_t category, notify_scheduler_t notifier, 
		void *p_params, uint32_t arg_count, const char** p_pargs);

void destroy(module_t *p_module);

module_state_t run(module_t *p_module);
module_state_t stop(module_t *p_module);

module_data_t *assign(module_t *p_module, module_data_t *p_data, bool is_sync);

void get_inputted_message_type(module_t *p_module, const char *** const ppp_inputted_message_types,
		uint32_t *p_message_type_count);

#ifdef __cplusplus
}
#endif
#endif
