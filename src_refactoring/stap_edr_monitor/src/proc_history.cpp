#include <map>
#include <string>

#include <time.h>
#include <unistd.h>
#include <sys/sysinfo.h>

#include "debug_print.h"
#include "proc.h"
#include "proc_history.h"

struct simple_proc{
    simple_proc() : pid(0), starttime(0), stoptime(0) {}
    simple_proc(int id, std::string n, time_t tm_strt, time_t tm_end):
        pid(id), name(n), starttime(tm_strt), stoptime(tm_end) {}

    int pid;
    std::string name;
    time_t starttime;
    time_t stoptime;
};

class proc_history_impl {
public:
    proc_history_impl();
    ~proc_history_impl();
    void on_proc_create(Proc *proc);
    void on_proc_destroy(int pid, time_t tm);
    time_t get_start_time(int pid);
    time_t get_end_time(int pid);
    std::string get_name(int pid);
    void clear_timeout(time_t timeout);
private:
    time_t boot_time;
    long clock_ticks;

    std::map<int, simple_proc> procs;        // key:pid
};

static time_t get_sys_boot_timestamp()
{
    struct sysinfo sys_info;
    if(sysinfo(&sys_info))  return 0;
    time_t boot_time = time(NULL) - (time_t)sys_info.uptime;
    return boot_time;
}

proc_history_impl::proc_history_impl()
{
    boot_time = get_sys_boot_timestamp();
    clock_ticks = sysconf(_SC_CLK_TCK);
}

void proc_history_impl::on_proc_create(Proc *proc)
{
    const char *name = proc->abs_name.size() ? proc->abs_name.c_str() : proc->cmd;
    procs[proc->pid] = simple_proc(proc->pid, name, (time_t)(boot_time+proc->start_time/clock_ticks), 0);
    debug_print("proc_history insert pid:%d starttm:%lu\n", proc->pid, procs[proc->pid].starttime);
}

void proc_history_impl::on_proc_destroy(int pid, time_t tm)
{
    procs[pid].stoptime = tm;
}

time_t proc_history_impl::get_start_time(int pid)
{
    debug_print("proc_history search pid:%d starttm:%lu\n", pid, procs[pid].starttime);
    return procs[pid].starttime;
}

time_t proc_history_impl::get_end_time(int pid)
{
    return procs[pid].stoptime;
}

std::string proc_history_impl::get_name(int pid)
{
    return procs[pid].name;
}

void proc_history_impl::clear_timeout(time_t timeout)
{
    time_t curr = time(NULL);
    for(std::map<int, simple_proc>::iterator itr=procs.begin(); itr!=procs.end(); )
    {
        if(itr->second.stoptime == 0)
        {
            ++itr;
        }
        else if(curr > itr->second.stoptime + timeout)
        {
            procs.erase(itr++);
        }
        else
        {
            ++itr;
        }
    }
}

proc_history::~proc_history()
{
    delete p_his;
}

proc_history::proc_history() :
    p_his(new proc_history_impl)
{
}

void proc_history::on_proc_create(Proc *proc)
{
    return p_his->on_proc_create(proc);
}

void proc_history::on_proc_destroy(int pid, time_t tm)
{
    return p_his->on_proc_destroy(pid, tm?tm:time(NULL));
}

time_t proc_history::get_start_time(int pid)
{
    return p_his->get_start_time(pid);
}

time_t proc_history::get_end_time(int pid)
{
    return p_his->get_end_time(pid);
}

std::string proc_history::get_name(int pid)
{
    return p_his->get_name(pid);
}

void proc_history::clear_timeout(time_t timeout)
{
    return p_his->clear_timeout(timeout);
}
