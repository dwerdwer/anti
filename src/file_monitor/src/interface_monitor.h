
#ifndef _file_monitor_
#define _file_monitor_

#include <stdint.h>

typedef enum 
{
    EVENT_NEW_DIR = 0,
	EVENT_REMOVE_DIR = 1,
	EVENT_NEW_FILE = 2,
	EVENT_REMOVE_FILE = 3,
	EVENT_MODIFY_FILE = 4,
} event_type_t;

extern "C"{

module_t * create(uint32_t category, notify_scheduler_t notifier, void *p_params, uint32_t arg_count, const char **p_args);

void destroy(module_t *p_module);

void get_inputted_message_type(module_t *p_module, const char *** const ppp_inputted_message_types,
                                     uint32_t *p_message_type_count);
module_state_t run(module_t *p_module);

module_state_t stop(module_t *p_module);

module_data_t* assign(module_t *p_module, const module_data_t *p_data, bool is_sync);

}

#endif

