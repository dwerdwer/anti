#include <inttypes.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "readproc.h"
#include "proc_monitor.h"
#include "statiser_main.h"

struct proc_monitor {
    statiser *p_ins;
};

int proc_monitor_set_option(proc_monitor_t *p_proc_monitor, const char *p_option_name, const char *p_option_value)
{
    int ret = -1;
    if(p_proc_monitor == NULL || p_proc_monitor->p_ins == NULL || p_option_value == NULL) {
        return ret;
    }

    if(0 == strcmp(p_option_name, "snapshot_delta")) {
        p_proc_monitor->p_ins->set_snapshot_delta(*(size_t *)(p_option_value));
        ret = 0;
    }
    else if(0 == strcmp(p_option_name, "enable_snapshot")) {
        p_proc_monitor->p_ins->enable_snapshot(*(int32_t  *)(p_option_value));
        ret = 0;
    }


    return ret;
}

int proc_monitor_get_option(proc_monitor_t *p_proc_monitor, const char *p_option_name, char *p_option_value)
{
    int ret = -1;
    if(p_proc_monitor == NULL || p_proc_monitor->p_ins == NULL || p_option_value == NULL) {
        return ret;
    }

    if(0 == strcmp(p_option_name, "snapshot_delta")) {
        *(size_t *)p_option_value = (size_t)p_proc_monitor->p_ins->get_snapshot_delta();
        ret = 0;
    }

    return ret;
}

proc_monitor_t *init_proc_monitor(proc_monitor_callback_t p_callback, void *p_user_data, size_t option_count, const char *(*pp_options)[2])
{
    proc_monitor_t *p_proc_monitor = (proc_monitor_t *)calloc(1, sizeof(proc_monitor_t));
    assert(p_proc_monitor);

    p_proc_monitor->p_ins = new statiser(p_callback, p_user_data);
    assert(p_proc_monitor->p_ins);

    for(size_t n=0; n<option_count; n++) {
        proc_monitor_set_option(p_proc_monitor, pp_options[n][0], pp_options[n][1]);
    }

    return p_proc_monitor;
}

int run_proc_monitor(proc_monitor_t *p_proc_monitor)
{
    if (p_proc_monitor && p_proc_monitor->p_ins) {
        return p_proc_monitor->p_ins->run();
    }
    return -1;
}

void stop_proc_monitor(proc_monitor_t *p_proc_monitor)
{
    if (p_proc_monitor && p_proc_monitor->p_ins) {
        return p_proc_monitor->p_ins->stop();
    }

    return ;
}

void fin_proc_monitor(proc_monitor_t *p_proc_monitor)
{
    if(p_proc_monitor) {
        if(p_proc_monitor->p_ins) {
            delete p_proc_monitor->p_ins;
        }
        delete p_proc_monitor;
    }
}


