#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>

#include "proc_impl.h"
#include "proclist.h"

#ifndef PRIu64
#define PRIu64 llu
#endif

#define PROC_PATH   "/proc"

static int is_number(const char *str)
{
    while(*str) {
        if(!isdigit(*str))
            return 0;
        str++;
    }
    return 1;
}

static int is_directory(const char *name)
{
    struct stat st;
    if(stat(name, &st)) return 0;
    if(!S_ISDIR(st.st_mode)) return 0;

    return 1;
}

// static Proc *ProcSearchBySockIno(ProcListHead *head, ino_t ino)
// {
//     if(!head->procs) return NULL;
//     for(int i=head->used-1; i>=0; i--)
//         if(is_ino_in_fds(head->procs[i], ino))
//             return head->procs[i];
// 
//     return NULL;
// }

// static int IsPathUsedByProc(Proc *proc, const char *path)
// {
//     if(0 == strcmp(proc->abs_name.c_str(), path))
//         return 1;
// 
//     if(is_path_in_libs(proc, path))
//         return 1;
// 
//     if(is_path_in_fds(proc, path))
//         return 1;
// 
//     return 0;
// }

static int compare_proc_proc(const void *a, const void *b)
{
    Proc *first = *(Proc **)a;
    Proc *second= *(Proc **)b;

    if(NULL == first)
        return 1;
    else if(NULL == second)
        return -1;

    return first->pid - second->pid;
}

static int compare_pid_proc(const void *a, const void *b)
{
    uint64_t pid = *(uint64_t *)a;
    Proc *second= *(Proc **)b;

    assert(second);

    return pid - second->pid;
}

static void ProcListSort(ProcListHead *head)
{
    if(head && head->procs && head->used)
        qsort(head->procs, head->used, sizeof(head->procs[0]), compare_proc_proc);
}

Proc **ProcSearchByPID(ProcListHead *head, uint64_t pid)
{
    return (Proc **)bsearch(&pid, head->procs, head->used, sizeof(head->procs[0]), compare_pid_proc);
}

static time_t get_sys_boot_timestamp()
{
    struct sysinfo sys_info;
    if(sysinfo(&sys_info))  return 0;
    time_t boot_time = time(NULL) - (time_t)sys_info.uptime;
    return boot_time;
}

void ProcListDebug(ProcListHead *head)
{
    FILE *file = fopen("readproc.debug", "w");
    assert(file);

    time_t boot_time = get_sys_boot_timestamp();

    char line[2048];
    for(size_t i=0; i<head->used; i++) {
        Proc *proc = head->procs[i];
        snprintf(line, sizeof(line), 
            "pid:[%10lu]; ppid:[%10d]; exec_ino:[%16ld]; "
            "UserID:[effect:%10d, real:%10d, save:%10d, faccess:%10d]; "
            "ProcessStartTime:[%10llu]; "
            "UserModeTime:[%16llu]; KernelModeTime:[%16llu]; "
            "ChildUserModeTime:[%16llu]; ChildKernelModeTime:[%16llu]; "
            "VM_Size:[%16lu]; VM_RSS:[%16lu]; Size:[%16lu]; "
            "tty:[%10d];"
            "Abs:[%-50s]; Name:[%-20s]; CMDLine:[%s];"
            ,
            proc->pid, proc->ppid, proc->exec_ino,
            proc->euid, proc->ruid, proc->suid, proc->fuid,
            (boot_time+proc->start_time/sysconf(_SC_CLK_TCK)),
            proc->utime, proc->stime,
            proc->cutime, proc->cstime,
            proc->vm_size, proc->vm_rss, proc->size,
            proc->tty,
            proc->abs_name.c_str(), proc->name.c_str(), proc->cmdline.c_str()
        );

        fputs(line, file);
        fputc('\n', file);
    }
    fclose(file);
}

int ProcListInsert(ProcListHead *head, Proc *proc)
{
    if(!head)
        return -1;
    if(!proc)
        return -1;

    if(head->used == head->capacity) {
        size_t new_capacity = head->capacity*5/4 + 10;
        Proc **ptr = (Proc **)realloc(head->procs, new_capacity*sizeof(Proc *));
        assert(ptr);
        memset(ptr+head->capacity, 0, (new_capacity-head->capacity)*sizeof(Proc *));
        head->procs = ptr;
        head->capacity = new_capacity;
    }
    head->procs[head->used++] = (Proc *)proc;
#ifdef DEBUG
    printf("%s(%d)\n", __func__, (int)proc->pid);
#endif
    return 0;
}

void ProcListInit(ProcListHead *head)
{
    if(head) {
        head->used = 0;
        head->capacity = 0;
        head->procs = NULL;
    }
}

int ProcListRemove(ProcListHead *head, uint64_t pid)
{
    if(!head || !pid)
        return 0;

    size_t removed = 0;
    do {
        Proc **pproc = (Proc **)bsearch(&pid, head->procs, head->used, sizeof(head->procs[0]), compare_pid_proc);
        if(!pproc)
            break;
        ProcFree(*pproc);
        *pproc = NULL;
        ProcListSort(head);
#ifdef DEBUG
        printf("%s(%d)\n", __func__, (int)pid);
#endif
        head->used --;
        removed ++;
    } while(true);

    return removed;
}

void ProcListClear(ProcListHead *head)
{
    if(!head || !head->procs)
        return;
    while(head->used > 0) {
        ProcFree(head->procs[head->used-1]);
        head->used--;
    }
    free(head->procs);
    head->procs = NULL;
    head->used = head->capacity = 0;
}

typedef int (*read_dir_cb_ptr)(const char *dir, const char *name, void *arg);
static int ReadDir(const char *dir, void *arg, read_dir_cb_ptr cb)
{
    DIR *d = opendir(dir);
    if(!d)  return -1;

    struct dirent *file;
    while(NULL != (file = readdir(d))) {
        cb(dir, file->d_name, arg);
    }

    closedir(d);
    return 0;
}

static int on_proc(const char *dir, const char *name, void *arg)
{
    if(!is_number(name))
        return 0;

    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/%s", dir, name);
    if(!is_directory(path))
        return 0;

    PIDs_t *head = (PIDs_t *)arg;

    // realloc
    if(head->used == head->capacity) {
        size_t new_capacity = head->capacity*5/4 + 10;
        head->ids = (uint64_t *)realloc(head->ids, new_capacity*sizeof(uint64_t));
        assert(head->ids);
        memset(head->ids+head->capacity, 0, (new_capacity-head->capacity)*sizeof(uint64_t));
        head->capacity = new_capacity;
    }
    // append a PID
    head->ids[head->used++] = (uint64_t)atoll(name);

    return 0;
}

void PIDs_init(PIDs_t *head)
{
    if(head) {
        head->capacity = 0;
        head->used = 0;
        head->ids = NULL;
    }
}

PIDs_t *PIDs_create()
{
    PIDs_t *head = (PIDs_t *)malloc(sizeof(PIDs_t));
    assert(head);
    PIDs_init(head);
    return head;
}

PIDs_t *PIDs_load(PIDs_t *head)
{
    if(!head) {
        head = PIDs_create();
    }

    ReadDir(PROC_PATH, head, on_proc);
    return head;
}

void PIDs_clear(PIDs_t *head)
{
    if(head) {
        head->used = 0;
    }
}

void PIDs_destroy(PIDs_t *head)
{
    if(head) {
        if(head->ids)
            free(head->ids);
        free(head);
    }
}


#ifdef PROC_LIST_UNIT_TEST

#include <iostream>
#include <stdlib.h>
#include <time.h>
using namespace std;


static int test_on_proc(const char *dir, const char *name, void *arg)
{
    if(!is_number(name))
        return 0;
    ProcListHead *head = (ProcListHead *)arg;

    ProcListInsert(head, ReadSingleProc(atoll(name)));
    return 0;
}

static void update_ps(ProcListHead *ps_head, PIDs_t *p_old, PIDs_t *p_new)
{
    // o:  index of p_old
    // ou: used  of p_old
    // n:  index of p_new
    // nu: used  of p_new

    for(size_t o=0, n=0, ou=p_old->used, nu=p_new->used; 
            o<ou || n<nu; ) {
        if(o==ou && n!=nu) {
            ProcListInsert(ps_head, ReadSingleProc(p_new->ids[n]));
            n++;
            continue;
        }
        else if(o!=ou && n==nu) {
            ProcListRemove(ps_head, p_old->ids[o]);
            o++;
            continue;
        }
        else if(o==ou && n==nu) {
            break;
        }

        if(p_old->ids[o] > p_new->ids[n]) {
            ProcListInsert(ps_head, ReadSingleProc(p_new->ids[n]));
            n++;
        }
        else if(p_old->ids[o] < p_new->ids[n]) {
            ProcListRemove(ps_head, p_old->ids[o]);
            o++;
        }
        else {
            o++;
            n++;
        }
    }
}

static void run_x_times(int x)
{
    ProcListHead ps_head;
    ProcListInit(&ps_head);

    PIDs_t pid_head1;
    PIDs_init(&pid_head1);

    // first load all Process_ID
    PIDs_load(&pid_head1);
    // actually load all Process
    for(size_t i=0; i<pid_head1.used; i++) {
        ProcListInsert(&ps_head, ReadSingleProc(pid_head1.ids[i]));
    }
    sleep(1);

    // second load
    PIDs_t *pid_head2 = PIDs_load(NULL);

    PIDs_t *id_old = &pid_head1;
    PIDs_t *id_new = pid_head2;

rerun:
    update_ps(&ps_head, id_old, id_new);

    // clear old
    PIDs_clear(id_old);

    // redirect old&&new
    PIDs_t *tmp = id_old;
    id_old = id_new;
    id_new = tmp;

    // load new
    PIDs_load(id_new);

    // sleep 
    usleep(50000);

    if(--x > 0)
        goto rerun;


    ProcListClear(&ps_head);
    cout << endl << endl;
}

int main(void)
{
    run_x_times(1000000);
    return 0;
}

#endif
