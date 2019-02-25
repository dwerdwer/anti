#ifndef __PROC_STATISTICS_H__
#define __PROC_STATISTICS_H__

#include <sys/types.h>
#include <inttypes.h>

#include "proc.h"

typedef struct {
    size_t  size;       // actually number
    Proc  **procs;      // null-terminated list
} ps_t;

ps_t *ps_load(size_t nmemb, Proc **procs);
void ps_destroy(ps_t *p_ps);

Proc *get_proc_by_PID(ps_t *p_ps, uint64_t pid);
Proc *get_proc_by_path(ps_t *p_ps, const char *path);
Proc *get_proc_by_sock_ino(ps_t *p_ps, ino_t ino);
const char *get_name_by_sock_ino(ps_t *p_ps, ino_t ino);

#endif // __PROC_STATISTICS_H__
