#include <pthread.h>
#include <functional>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <unistd.h>
#include <sys/time.h>
#include <sys/sysinfo.h>

#include "proclist.h"
// #include "statiser_proc.h"
#include "statiser_main.h"

#ifndef INFO_PRINT
#define INFO_PRINT(...)\
        printf("[%s] ", __func__);\
        printf(__VA_ARGS__);
#endif

static uint64_t gettimeofday_ms()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
}

static time_t get_sys_boot_timestamp()
{
    struct sysinfo sys_info;
    if(sysinfo(&sys_info))  return 0;
    time_t boot_time = time(NULL) - (time_t)sys_info.uptime;
    return boot_time;
}

size_t statiser::get_snapshot_delta()
{
    return (size_t)(snapshot_delta/1000);
}

bool statiser::is_first_run()
{
    return first_run;
}

void statiser::after_first_run()
{
    first_run = false;
}

void statiser::set_snapshot_delta(size_t seconds)
{
    snapshot_delta = seconds * 1000;
    INFO_PRINT("[%s] %llums\n", __func__, (unsigned long long)snapshot_delta);
}

statiser::statiser() :
    m_run(false), m_pid(0), 
    flush_time(0), flush_delta(60),
    snapshot_time(0), snapshot_delta(0),
    first_run(true),
    p_callback(NULL)
{
    // default delta 100ms
    set_snapshot_delta(100);

    ProcListInit(&proc_head);

    // init system info
    boot_time = get_sys_boot_timestamp();
    clock_ticks = sysconf(_SC_CLK_TCK);
}

statiser::~statiser()
{
    stop();
}

statiser *statiser::get_instance()
{
    static statiser ins;
    return &ins;
}

static void update_ps(PIDs_t *p_old, PIDs_t *p_new, std::vector<uint64_t> &destroyed, std::vector<uint64_t> &created)
{
    // o:  index of p_old
    // ou: used  of p_old
    // n:  index of p_new
    // nu: used  of p_new

    for(size_t o=0, n=0, ou=p_old->used, nu=p_new->used; 
            o<ou || n<nu; ) {
        if(o==ou && n!=nu) {
            created.push_back(p_new->ids[n]);
            // ProcListInsert(ps_head, ReadSingleProc(p_new->ids[n]));
            n++;
            continue;
        }
        else if(o!=ou && n==nu) {
            destroyed.push_back(p_old->ids[o]);
            // ProcListRemove(ps_head, p_old->ids[o]);
            o++;
            continue;
        }
        else if(o==ou && n==nu) {
            break;
        }

        if(p_old->ids[o] > p_new->ids[n]) {
            created.push_back(p_new->ids[n]);
            // ProcListInsert(ps_head, ReadSingleProc(p_new->ids[n]));
            n++;
        }
        else if(p_old->ids[o] < p_new->ids[n]) {
            destroyed.push_back(p_old->ids[o]);
            // ProcListRemove(ps_head, p_old->ids[o]);
            o++;
        }
        else {
            o++;
            n++;
        }
    }
}

int statiser::on_proc_action_destroy(std::vector<uint64_t> &vec)
{
    for(std::vector<uint64_t>::iterator itr=vec.begin();
            itr!=vec.end(); itr++) {
        Proc **pproc = ProcSearchByPID(&proc_head, *itr);
        if(pproc && *pproc) {
            (*pproc)->stoptime = time(NULL);
            p_callback(PROC_ACTION_DESTROY, 1, pproc, p_user_data);
        }
        ProcListRemove(&proc_head, *itr);
    }
    return 0;
}

int statiser::on_proc_action_create(std::vector<uint64_t> &vec)
{
    for(std::vector<uint64_t>::iterator itr=vec.begin();
            itr!=vec.end(); itr++) {
        Proc *proc = ReadSingleProc(*itr);
        if(proc) {
            proc->starttime = boot_time + proc->start_time/clock_ticks;
            ProcListInsert(&proc_head, proc);
            p_callback(PROC_ACTION_CREATE, 1, &proc, p_user_data);
        }
    }
    return 0;
}

int statiser::on_snap_shot()
{
    p_callback(PROC_SNAPSHOT, proc_head.used, proc_head.procs, p_user_data);
    return 0;
}

void *statiser::loop(void *arg)
{
    statiser *st = (statiser *)arg;

    PIDs_t *p1 = PIDs_create();
    PIDs_t *p2 = PIDs_create();
    assert(p1 && p2);

    std::vector<uint64_t> destroyed(std::max(p1->used, p2->used));
    std::vector<uint64_t> created(std::max(p1->used, p2->used));

    while(st->m_run) {
        uint64_t curr = gettimeofday_ms();

        if(curr >= st->flush_time + st->flush_delta) {
            st->flush_time = curr;

            PIDs_load(p2);
            if(st->is_first_run()) {
                PIDs_load(p1);
                p1->used = 0;
                st->after_first_run();
                printf("---------------\n");
            }
            update_ps(p1, p2, destroyed, created);
#ifdef DEBUG
            printf("p1:%lu, p2:%lu, create:%lu, destroy:%lu\n",
                p1->used, p2->used, created.size(), destroyed.size());
#endif
//             st->proc_historys.update(st->ps_prev);
            if(st->p_callback) {
                st->on_proc_action_create(created);
                st->on_proc_action_destroy(destroyed);
            }
            created.clear();
            destroyed.clear();

            std::swap(p1, p2);
            PIDs_clear(p2);
        }
        if(curr >= st->snapshot_time + st->snapshot_delta) {
            st->snapshot_time = curr;
            if(st->p_callback)
                st->on_snap_shot();
            ProcListDebug(&st->proc_head);
        }

        usleep(20000);
    }
    return NULL;
}

void statiser::run()
{
    m_run = true;
    if(0 == m_pid) {
        pthread_create(&m_pid, NULL, statiser::loop, this);
//         m_thread = new std::thread(&statiser::loop, this);
//         m_thread = new std::thread(std::bind(&statiser::loop, this));
    }
}

void statiser::stop()
{
    m_run = false;
    printf("[%s] m_run(%p):%d\n", __func__, &m_run, m_run);
    if(m_pid) {
        pthread_join(m_pid, NULL);
        m_pid = 0;
    }
}

void statiser::set_user_data(proc_monitor_callback_t callback, void *data)
{
    p_callback = callback;
    p_user_data = data;
}
