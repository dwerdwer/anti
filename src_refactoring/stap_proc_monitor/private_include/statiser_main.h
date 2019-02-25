#ifndef __STATISER_MAIN_H__
#define __STATISER_MAIN_H__

#include <pthread.h>
#include <inttypes.h>

#include "proc_monitor.h"
#include "stap_monitor.h"

struct statiser {
    statiser(proc_monitor_callback_t p_callback, void *p_user_data);
    ~statiser();
    int run();
    void stop();

    void enable_snapshot(int enable);
    void set_snapshot_delta(int second);
    time_t get_snapshot_delta();
    static void on_proc_action(const stap_proc_action_t *pst, void *p_user_data);


    bool m_run;
    bool enable_ss;

    // sysinfo
    time_t boot_time;
    // sysconf
    long clock_ticks;

    std::map<int, Proc *> procs;

    time_t ps_time;
    time_t ps_delta;

    void *p_user_data;
    proc_monitor_callback_t p_callback;

    stap_monitor_t *p_stap_proc_monitor;
};

#endif /* __STATISER_MAIN_H__ */
