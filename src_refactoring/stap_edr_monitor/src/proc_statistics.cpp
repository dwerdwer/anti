#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <map>
#include <string>
#include <assert.h>

#include "defs.h"
#include "proc_statistics.h"
#include "proc_monitor.h"

#ifdef EDR_DEBUG
#include "util-system.h"
#endif /* EDR_DEBUG */

static bool is_path_in_libs(Proc *proc, const char *path)
{
    for(std::map<std::string, DynamicLibInfo>::iterator it=proc->libs.begin();
            it!=proc->libs.end(); it++)
    {
        if(0 == strcmp(it->first.c_str(), path))
        {
            return true;
        }
    }
    return false;
}

static bool is_path_in_fds(Proc *proc, const char *path)
{
    for(std::vector<FdInfo>::iterator it=proc->fds.begin();
        it != proc->fds.end(); it++)
    {
        if(0 == strcmp(it->name.c_str(), path))
            return true;
    }
    return false;
}

static bool is_ino_in_fds(Proc *proc, ino_t ino)
{
    for(std::vector<FdInfo>::iterator it=proc->fds.begin();
        it != proc->fds.end(); it++)
    {
        if (it->ino == ino)
            return true;
    }
    return false;
}

static Proc *find_proc_by_sock_ino(ps_t *p_ps, ino_t ino)
{
    if(!p_ps->procs) return NULL;
    for(size_t i=0, size=p_ps->size; i<size; i++)
        if(is_ino_in_fds(p_ps->procs[i], ino))
            return p_ps->procs[i];

    return NULL;
}

Proc *get_proc_by_sock_ino(ps_t *p_ps, ino_t ino)
{
    if(p_ps && ino)
    {
        return find_proc_by_sock_ino(p_ps, ino);
    }
    return NULL;
}

static int is_path_used_by_proc(Proc *proc, const char *path)
{
    if(0 == strcmp(proc->abs_name.c_str(), path))
        return 1;

    if(is_path_in_libs(proc, path))
        return 1;

    if(is_path_in_fds(proc, path))
        return 1;

    return 0;
}

Proc *get_proc_by_path(ps_t *p_ps, const char *path)
{
    if(!p_ps || !path || !path[0])
        return NULL;
    Proc **procs = p_ps->procs;
    for(size_t i=0, size=p_ps->size; i<size; i++)
    {
        if(is_path_used_by_proc(procs[i], path))
            return procs[i];
    }
    return NULL;
}

const char *get_name_by_sock_ino(ps_t *p_ps, ino_t ino)
{
    Proc *proc = get_proc_by_sock_ino(p_ps, ino);
    if(proc)
    {
        return proc->abs_name.c_str();
    }
    return NULL;
}

static int proc_compare_by_PID(const void *a, const void *b)
{
    Proc *first = *(Proc **)a;
    Proc *second= *(Proc **)b;

    return first->pid - second->pid;
}

static void ps_sort(ps_t *p_ps)
{
    if(p_ps && p_ps->procs && p_ps->size)
    {
        qsort(p_ps->procs, p_ps->size, sizeof(p_ps->procs[0]), proc_compare_by_PID);
    }
}

static Proc *ProcSearchByPID(ps_t *p_ps, Proc *proc)
{
    Proc **proc_addr = (Proc **)bsearch(&proc, p_ps->procs, p_ps->size, sizeof(p_ps->procs[0]), proc_compare_by_PID);
    if(proc_addr)
    {
        return *proc_addr;
    }
    return NULL;
}

Proc *get_proc_by_PID(ps_t *p_ps, uint64_t pid)
{
    if(!p_ps)
        return NULL;
    Proc proc;
    proc.pid = pid;
    return ProcSearchByPID(p_ps, &proc);
}

ps_t *ps_load(size_t nmemb, Proc **procs)
{
    ps_t *p_ps = (ps_t *)calloc(1, sizeof(ps_t));
    assert(p_ps);

    p_ps->procs = (Proc **)calloc(nmemb+1, sizeof(Proc *));
    assert(p_ps->procs);

    p_ps->size = nmemb;
    memcpy(p_ps->procs, procs, nmemb*sizeof(Proc *));

    return p_ps;
}
void ps_destroy(ps_t *p_ps)
{
    if(!p_ps)
    {
        return;
    }
    if(p_ps->procs)
    {
        while(p_ps->size > 0)
        {
            proc_free(p_ps->procs[p_ps->size-1]);
            p_ps->size--;
        }
        free(p_ps->procs);
    }
    free(p_ps);
}
