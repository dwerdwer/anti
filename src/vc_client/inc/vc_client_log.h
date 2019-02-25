#ifndef __VC_CLIENT_LOG_H__
#define __VC_CLIENT_LOG_H__

#include <string.h>
#include <stdarg.h>

#include "module_data.h"
#include "debug_print.h"
#include "vc_client_defines.h"
#include "module_interfaces.h"

#define MAX_LOG_LENGTH                  1024 
#define LOG_PREFIX                      "VC_CLIENT: "
#define FORMAT_FIRST_MESSAGE_ID         "MSG_TYPE_LOG"
#define FORMAT_SECOND_MESSAGE_TYPE      "LOG_MESSAGE"

static void vc_client_log(module_t *p_module, const char* fmt, ...)
{
    if(NULL == p_module)
        return;

    char log_str[MAX_LOG_LENGTH] = { 0 };

    strcpy(log_str, LOG_PREFIX);

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(log_str + strlen(log_str), MAX_LOG_LENGTH - strlen(log_str), fmt, ap);
    va_end(ap);

    debug_print("%s\n", log_str);

    module_data_t *p_data = create_module_data();
    set_module_data_property(p_data, g_p_message_id, FORMAT_FIRST_MESSAGE_ID, strlen(FORMAT_FIRST_MESSAGE_ID));
    set_module_data_property(p_data, FORMAT_SECOND_MESSAGE_TYPE, log_str, strlen(log_str));
    module_message_t module_message;
    module_message.p_data = p_data;
    module_message.category = (module_category_t)p_module->category;

    mdh_sync_params_t sync_param;
    sync_param.is_sync = false;

    p_module->notifier(&module_message, p_module->p_params, &sync_param);
    destroy_module_data(p_data);
}


#endif
