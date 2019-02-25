#ifndef __READ_PROC_H__
#define __READ_PROC_H__

#include <sys/types.h>
#include <inttypes.h>

#include "proc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    size_t  capacity;   // capacity of procs, and capacity will increment 1/4 automically when used==capacity
    size_t  used;       // actually number
    Proc  **procs;      // null-terminated array
} ps_t;

ps_t *ps_load();
void ps_free_shallow(ps_t *p_ps);
void ps_free(ps_t *p_ps);
Proc *get_proc(ps_t *p_ps, int pid);

#ifdef __cplusplus
}
#endif

#endif // __READ_PROC_H__
