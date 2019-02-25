#ifndef __STATISER_MAIN_H__
#define __STATISER_MAIN_H__

#include <pthread.h>
#include <inttypes.h>

#include "proclist.h"
// #include "statiser_proc.h"
#include "proc_monitor.h"

class statiser {
public:
    ~statiser();
    void run();
    void stop();

    static statiser *get_instance();

    void set_snapshot_delta(size_t seconds);
    size_t get_snapshot_delta();
    void set_user_data(proc_monitor_callback_t callback, void *p_user_data);

    int on_snap_shot();
    int on_proc_action_create(std::vector<uint64_t> &vec);
    int on_proc_action_destroy(std::vector<uint64_t> &vec);

    bool is_first_run();
    void after_first_run();

private:
    statiser();
    volatile bool m_run;
    static void *loop(void *arg);
    pthread_t m_pid;

    // sysinfo
    time_t boot_time;
    // sysconf
    long clock_ticks;

    uint64_t flush_time;
    uint64_t flush_delta;

    uint64_t snapshot_time;
    uint64_t snapshot_delta;

    bool first_run;
    ProcListHead proc_head;
//     proc_record proc_historys;

    void *p_user_data;
    proc_monitor_callback_t p_callback;
};

#endif /* __STATISER_MAIN_H__ */
