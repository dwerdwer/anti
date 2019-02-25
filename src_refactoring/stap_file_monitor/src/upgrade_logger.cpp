#include <time.h>
#include <sys/time.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "debug_print.h"

#include "module_interfaces.h"
#include "module_data.h"
#include "upgrade_interface.h"
#include "upgrade_logger.h"

#define MAX_LOG_LENGTH                  4096

#define LOG_PREFIX                      "COMMON: "
#define FORMAT_FIRST_MESSAGE_ID         "MSG_TYPE_LOG"
#define FORMAT_SECOND_MESSAGE_TYPE      "LOG_MESSAGE"

CommonLogger g_logger;

CommonLogger::CommonLogger()
{
    this->prefix = LOG_PREFIX;
}

CommonLogger::~CommonLogger()
{
    this->p_info = NULL;
}

void CommonLogger::set_prefix(const char *prefix)
{
    this->prefix.clear();
    this->prefix += "[";
    this->prefix += prefix;
    this->prefix += "] ";
}

void CommonLogger::set_module_info(module_info_t *p_info)
{
    this->p_info = p_info;
}

void CommonLogger::log(const char* fmt, ...)
{
    char log[MAX_LOG_LENGTH] = {0};


    snprintf(log, sizeof(log), "%s: ", this->prefix.c_str());
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(log + strlen(log), sizeof(log) - strlen(log), fmt, ap);
    va_end(ap);

    struct timeval now;
    struct tm tm;
    char time_buf[25] = {0};
    gettimeofday(&now, NULL);
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime_r(&now.tv_sec, &tm));
    snprintf(time_buf + strlen(time_buf),
             sizeof(time_buf) - strlen(time_buf), ":%lu", now.tv_usec);
    debug_print("[%s] %s\n", time_buf, log);

    if(this->p_info == NULL)
    {
        debug_print("[%s] module_info_t is null, don't send to logger\n", this->prefix.c_str());
        return ;
    }

    module_data_t *p_data = create_module_data();
    set_module_data_property(p_data, g_p_message_id, FORMAT_FIRST_MESSAGE_ID, strlen(FORMAT_FIRST_MESSAGE_ID));
    set_module_data_property(p_data, FORMAT_SECOND_MESSAGE_TYPE, log, strlen(log));

    module_message_t module_msg;
    module_msg.category = (module_category_t)(this->p_info->category);
    module_msg.p_data = p_data;

    mdh_sync_params_t sync_data;
    sync_data.is_sync = false;
    sync_data.ptrs_alloc = NULL;
    sync_data.result.pp_ptrs = NULL;
    sync_data.result.count = 0;

    this->p_info->notifier(&module_msg, this->p_info->p_params, &sync_data);

    destroy_module_data(p_data);
}
