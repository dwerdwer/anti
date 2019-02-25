#ifndef MODULE_INTERFACES_HHH
#define MODULE_INTERFACES_HHH

#include "module_defines.h"

#include <stdbool.h>
#include <stdint.h>

__attribute__((unused)) static const char *g_p_message_id = "MESSAGE_ID";

typedef enum
{
    MODULE_OK = 0,
    MODULE_ERROR = -1,
    MODULE_FATAL = -2,
}module_state_t;

typedef struct module_data module_data_t;

typedef struct module_message
{
    // Note: there is always a property specified by 'g_p_message_id' in module_data_t.
    const module_data_t *p_data;
    module_category_t category;
}module_message_t;

typedef struct module_data_ptrs
{
    const module_data_t **pp_ptrs;
    uint32_t count;
}module_data_ptrs_t;

typedef const module_data_t **(*module_data_ptrs_alloc)(uint32_t count);
// 'mdh' is short for "module data handle".
typedef struct mdh_sync_params
{
    module_data_ptrs_alloc ptrs_alloc;
    module_data_ptrs_t result;
    bool is_sync;
}mdh_sync_params_t;


// If 'mdh_sync_params_t::is_sync' is true, 
// module data will be handled immediately, and return results.
// The module which receives those results has responsibility to 'destroy' those module data.
typedef void (*notify_scheduler_t)(const module_message_t *p_module_message, 
        void *p_params, mdh_sync_params_t *p_sync);

typedef struct module module_t;

typedef module_t *(*module_creator_t)(uint32_t category, notify_scheduler_t notifier, 
        void *p_params, uint32_t arg_count, const char **p_args);
typedef void (*module_destroyer_t)(module_t *p_module);

// 'imt' is short for "inputted message type"
typedef void (*imt_getter_t)(module_t *p_module, const char *** const ppp_inputted_message_types, 
        uint32_t *p_message_type_count);

typedef module_state_t (*module_runner_t)(module_t *p_module);
typedef module_state_t (*module_stopper_t)(module_t *p_module);

// If 'is_sync' is true, 
// module data will be handled immediately, and return result.
typedef module_data_t *(*data_assigner_t)(module_t *p_module, const module_data_t *p_data, 
        bool is_sync);

#endif

