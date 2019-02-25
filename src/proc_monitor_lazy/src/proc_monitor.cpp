#include <string.h>

#include <inttypes.h>
#include <time.h>
#include "proc.h"

#include "proc_monitor.h"
#include "statiser_main.h"

#define PUBLIC_API __attribute__((visibility("default")))

PUBLIC_API proc_monitor_t *init_proc_monitor(proc_monitor_callback_t callback, void *p_user_data, size_t option_count, const char *(*pp_options)[2])
{
    statiser *p_ins = statiser::get_instance();
    p_ins->set_user_data(callback, p_user_data);
    for(size_t n=0; n<option_count; n++) {
        if(0 == strcmp(pp_options[n][0], "snapshot_delta")) {
            p_ins->set_snapshot_delta(*(size_t *)(pp_options[n][1]));
        }
    }
    p_ins->run();

    return (void *)p_ins;
}

PUBLIC_API void fin_proc_monitor(proc_monitor_t *p_proc_monitor)
{
    statiser *p_ins = (statiser *)p_proc_monitor;
    p_ins->stop();
}

PUBLIC_API bool proc_monitor_set_option(proc_monitor_t *p_proc_monitor, const char *p_option_name, const char *p_option_value)
{
    statiser *p_ins = (statiser *)p_proc_monitor;
    if(0 == strcmp(p_option_name, "snapshot_delta")) {
        p_ins->set_snapshot_delta(*(size_t  *)(p_option_value));
        return true;
    }
    return false;
}

PUBLIC_API const char* proc_monitor_get_option(proc_monitor_t *p_proc_monitor, const char *p_option_name)
{
    statiser *p_ins = (statiser *)p_proc_monitor;
    if(0 == strcmp(p_option_name, "snapshot_delta")) {
        static size_t snapshot_delta = p_ins->get_snapshot_delta();
        return (const char *)&snapshot_delta;
    }
 
    return NULL;
}
