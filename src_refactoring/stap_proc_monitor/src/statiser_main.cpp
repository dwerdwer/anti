#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/sysinfo.h>

#include "defs.h"
#include "debug_print.h"
#include "readproc.h"
#include "stap_monitor.h"
#include "statiser_main.h"

static const char *stap_default_exe =	"SYSTEMTAP_STAPIO=./stapio ./staprun -R";
static const char *stap_default_stp =   "../lib/edr_stap_proc_monitor.ko";

static uint64_t gettimeofday_ms()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000ULL + tv.tv_usec/1000ULL;
}


static time_t get_sys_boot_timestamp()
{
    struct sysinfo sys_info;
    if(sysinfo(&sys_info))  return 0;
    time_t boot_time = time(NULL) - (time_t)sys_info.uptime;
    return boot_time;
}

void statiser::enable_snapshot(int enable)
{
    this->enable_ss = enable ? true : false;
}

time_t statiser::get_snapshot_delta()
{
    return ps_delta;
}

void statiser::set_snapshot_delta(int seconds)
{
    ps_delta = seconds;
    debug_print("[%s] %d\n", __func__, seconds);
}

void statiser::on_proc_action(const stap_proc_action_t *p_action, void *p_user_data)
{
    statiser *st = (statiser *)p_user_data;

    Proc *p_proc = NULL;
    proc_event_type_t ev;

    // a proc created
    if(p_action->action == STAP_PROC_ACTION_CREATE) {
        ev = PROC_ACTION_CREATE;
        std::map<int, Proc *>::iterator itr=st->procs.find(p_action->pid);
        if(itr != st->procs.end())
        {
            if(itr->second == NULL)
            {
                /* this item read_proc failed before */
            }
            else if(gettimeofday_ms() < itr->second->current_time + 1000)
            {
                /* this record is repeat, ignore */
                return;
            }
            else
            {
                /* something error */
                proc_free(itr->second);
                itr->second = NULL;
            }
        }
        p_proc = read_proc(p_action->pid);
        st->procs[p_action->pid] = proc_deep_copy(p_proc);
    }
    // a proc destroyed
    else if(p_action->action == STAP_PROC_ACTION_EXIT) {
        ev = PROC_ACTION_DESTROY;
        std::map<int, Proc *>::iterator itr = st->procs.find(p_action->pid);
        if(itr != st->procs.end()) {
            p_proc = itr->second;
            st->procs.erase(itr);
        }
    }

    if(p_proc == NULL) {
        return ;
    }

    if(ev == PROC_ACTION_CREATE) {
        // p_proc->starttime = p_action->time;
    }
    else if(ev == PROC_ACTION_DESTROY) {
        p_proc->stoptime = p_action->time;
        if(p_proc->stoptime == 0) {
            p_proc->stoptime = time(NULL);
        }
    }

    debug_print("%s procs size:%d\n", __func__, (int)st->procs.size());

    st->p_callback(ev, 1, &p_proc, st->p_user_data);
}

static void on_stap_proc_action(const stap_proc_action_t *p_action, void *p_user_data)
{
    statiser *st = (statiser *)p_user_data;
    if(!st || !st->p_callback || !st->m_run)
        return ;
    debug_print("%s action:%1d, pid:%10d, ppid:%10d, time:%lu\n",
            __func__, p_action->action, p_action->pid, p_action->ppid, p_action->time);
    statiser::on_proc_action(p_action, (statiser *)p_user_data);
}

statiser::statiser(proc_monitor_callback_t p_callback, void *p_user_data) :
    m_run(false),
    enable_ss(false),
    /* init system info */
    boot_time(get_sys_boot_timestamp()), clock_ticks(sysconf(_SC_CLK_TCK)),
    ps_time(0), ps_delta(5),
    p_user_data(p_user_data), p_callback(p_callback)
{
    // default delta 5s
    set_snapshot_delta(5);

    // run systemtap at last
    this->p_stap_proc_monitor = init_stap_monitor(stap_default_exe, stap_default_stp, on_stap_proc_action, this);
    assert(this->p_stap_proc_monitor);
}

statiser::~statiser()
{
    this->stop();
    fin_stap_monitor(this->p_stap_proc_monitor);
    for(std::map<int, Proc *>::iterator itr = procs.begin();
            itr != procs.end();) {
        proc_free(itr->second);
    }
}

static int loop(statiser *st)
{
    if(!st->p_callback) {
        return 0;
    }

    ps_t *p_ps = ps_load();
    if(p_ps && p_ps->procs) {
        /* all processes are regarded as new process */
        for(size_t i=0; i<p_ps->used; i++) {
            stap_proc_action_t action;
            action.action = STAP_PROC_ACTION_CREATE;
            action.pid = p_ps->procs[i]->pid;
            action.ppid = p_ps->procs[i]->ppid;
            action.time = p_ps->procs[i]->starttime;
            on_stap_proc_action(&action, st);
            /* note: p_ps->procs[n] is un-usable after p_callback */
        }
    }
    ps_free_shallow(p_ps);

    struct timeval timeout;
    while(st->m_run) {
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        wait_stap_monitor(st->p_stap_proc_monitor, &timeout);

        if (st->enable_ss == false) {
            usleep(100000);
            continue;
        }

        time_t curr = time(NULL);
        if(curr >= st->ps_time + st->ps_delta) {
            st->ps_time = curr;

            p_ps = ps_load();
            if(!p_ps || !p_ps->procs)
                continue;

            debug_print("%s snaps size:%d\n", __func__, (int)p_ps->used);

            if(st->p_callback) {
                st->p_callback(PROC_SNAPSHOT, p_ps->used, p_ps->procs, st->p_user_data);
            }
            ps_free_shallow(p_ps);
        }
    }


    return 0;
}

int statiser::run()
{
    this->m_run = true;
    return loop(this);
}

void statiser::stop()
{
    m_run = false;
}
