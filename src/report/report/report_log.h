#include <iostream>
#include <stdarg.h>
#include "module_interfaces.h"
#include "module_data.h"
#define MAX_LOG_LENGTH 1024

static void report_log(module_t *p_module, const char* fmt, ...)
{
	if(NULL == p_module)
		return ;

	char msg_log[MAX_LOG_LENGTH] = {0};

	sprintf(msg_log, "kv_report:");

	va_list ap;
	va_start(ap, fmt);
	
	vsprintf(msg_log + strlen(msg_log), fmt, ap);
	va_end(ap);

	module_data_t *p_data = create_module_data();
	std::string message_id = "MSG_TYPE_LOG";
	set_module_data_property(p_data, g_p_message_id, (const char*)message_id.c_str(), message_id.length());
	set_module_data_property(p_data, "LOG_MESSAGE", msg_log, strlen(msg_log));
	module_message_t module_msg;
	module_msg.p_data = p_data;
	module_msg.category = static_cast<module_category_t>(p_module->uCategory);

    mdh_sync_params_t sync_param;
    sync_param.is_sync = false;
	
	p_module->pNotify(&module_msg, p_module->pParams, &sync_param);
	destroy_module_data(p_data);
}
