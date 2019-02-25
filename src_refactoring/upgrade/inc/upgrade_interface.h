#pragma once

#include "module_data.h"
#include "module_interfaces.h"

#include <stdint.h>


#define UPGRADE_MSG_ID          "Update"
#define UPGRADE_MSG_ID_RESULT   "UpdateResult"

#define TO_CENTER_AGENT_VIRUSLIB_VERSION "CENTER_MESSAGE_VIRUS_LIB_DATE"



extern "C"
{

typedef struct {
    uint32_t            category;
    notify_scheduler_t  notifier;
    void               *p_params;
    uint32_t            arg_count;
    const char        **pp_args;
} module_info_t;

typedef struct module module_t;

module_t * create(uint32_t category, notify_scheduler_t notifier, void *p_params, uint32_t arg_count, const char **pp_args);

void destroy(module_t *p_module);

void get_inputted_message_type(module_t *p_module, const char *** const ppp_inputted_message_types,
                                     uint32_t *p_message_type_count);
module_state_t run(module_t *p_module);

module_state_t stop(module_t *p_module);

module_data_t* assign(module_t *p_module, const module_data_t *p_data, bool is_sync);

}
