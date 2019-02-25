#ifndef __PROC_HISTORY_H__
#define __PROC_HISTORY_H__

#include "proc.h"
#include <time.h>
#include <string>

class proc_history_impl;

class proc_history {
public:
    proc_history();
    ~proc_history();

    void on_proc_create(Proc *proc);
    void on_proc_destroy(int pid, time_t tm=0);
    time_t get_start_time(int pid);
    time_t get_end_time(int pid);
    std::string get_name(int pid);
    void clear_timeout(time_t timeout);

private:
    proc_history_impl *p_his;
};

#endif /* __PROC_HISTORY_H__ */
