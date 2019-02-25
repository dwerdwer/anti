#ifndef __PROC_MONITOR_H__
#define __PROC_MONITOR_H__

#include <inttypes.h>
#include <time.h>

#include "proc.h"

typedef enum {
    PROC_SNAPSHOT,
    PROC_ACTION_CREATE,
    PROC_ACTION_DESTROY,
} proc_event_type_t;

typedef void (*proc_monitor_callback_t)(proc_event_type_t event_type, size_t nmemb, Proc **procs, void *p_user_data);
typedef void proc_monitor_t;

proc_monitor_t *init_proc_monitor(proc_monitor_callback_t callback, void *p_user_data, size_t option_count, const char *(*pp_options)[2]);
void fin_proc_monitor(proc_monitor_t *p_proc_monitor);

bool proc_monitor_set_option(proc_monitor_t *p_proc_monitor, const char *p_option_name, const char *p_option_value);
const char* proc_monitor_get_option(proc_monitor_t *p_proc_monitor, const char *p_option_name);

#endif /* __PROC_MONITOR_H__ */
