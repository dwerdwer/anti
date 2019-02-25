#ifndef __EDR_SYS_INFO_LOGGER_H__
#define __EDR_SYS_INFO_LOGGER_H__

#include <string.h>
#include <stdarg.h>

#include "debug_print.h"

#include "module_interfaces.h"
#include "module_data.h"
#include "sysinfo_interface.h"

#define MAX_LOG_LENGTH                  4096
#define LOG_PREFIX                      "EDR_sysinfo: "
#define FORMAT_FIRST_MESSAGE_ID         "MSG_TYPE_LOG"
#define FORMAT_SECOND_MESSAGE_TYPE      "LOG_MESSAGE"
static void sysinfo_log(module_info_t *m_info, const char* fmt, ...)
{
    if(NULL == m_info)
        return ;

    char log[MAX_LOG_LENGTH] = {0};

    strcpy(log, LOG_PREFIX);

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(log + strlen(log), sizeof(log) - strlen(log), fmt, ap);
    va_end(ap);

    debug_print("%s", log);

    module_data_t *p_data = create_module_data();
    set_module_data_property(p_data, g_p_message_id, FORMAT_FIRST_MESSAGE_ID, sizeof(FORMAT_FIRST_MESSAGE_ID)-1);
    set_module_data_property(p_data, FORMAT_SECOND_MESSAGE_TYPE, log, strlen(log));
    module_message_t module_message;
    module_message.p_data = p_data;
    module_message.category = (module_category_t)(m_info->category);

    mdh_sync_params_t sync_param;
    sync_param.is_sync = false;

    m_info->notifier(&module_message, m_info->p_params, &sync_param);
    destroy_module_data(p_data);
}


#endif /*__EDR_SYS_INFO_LOGGER_H__*/
