#include <stdint.h>
#include "defs.h"
#include "module_interfaces.h"
#include "module_data.h"

#ifndef __SYSTEM_INFOMATION_INTERFACE_H__
#define __SYSTEM_INFOMATION_INTERFACE_H__

EXTERN_C_BEGIN

typedef struct {
    uint32_t            category;
    notify_scheduler_t  notifier;
    void               *p_params;
    uint32_t            arg_count;
    const char        **p_pargs;
} module_info_t;

#define SYSINFO_PUBLIC_API __attribute__((visibility("default")))

SYSINFO_PUBLIC_API module_t *create(uint32_t category, notify_scheduler_t notifier, 
        void *p_params, uint32_t arg_count, const char** p_pargs);
SYSINFO_PUBLIC_API void destroy(module_t *p_module);
SYSINFO_PUBLIC_API module_state_t run(module_t *p_module);
SYSINFO_PUBLIC_API module_state_t stop(module_t *p_module);
SYSINFO_PUBLIC_API module_data_t *assign(module_t *p_module, const module_data_t *p_data, bool is_sync);
SYSINFO_PUBLIC_API void get_inputted_message_type(module_t *p_module, 
        const char *** const ppp_inputted_message_types, uint32_t *p_message_type_count);

EXTERN_C_END
#endif // __SYSTEM_INFOMATION_INTERFACE_H__
