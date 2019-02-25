#ifndef __READ_PROC_H__
#define __READ_PROC_H__

#include <sys/types.h>
#include <inttypes.h>

#include "proc_impl.h"

typedef struct {
    size_t  capacity;   // capacity of procs, and capacity will increment 1/4 automically when used==capacity
    size_t  used;       // actually using
    Proc  **procs;      // array with <used> elements
} ProcListHead;

void ProcListInit(ProcListHead *head);
void ProcListClear(ProcListHead *head);
int ProcListInsert(ProcListHead *head, Proc *proc);
int ProcListRemove(ProcListHead *head, uint64_t pid);

Proc **ProcSearchByPID(ProcListHead *head, uint64_t pid);
void ProcListDebug(ProcListHead *head);

typedef struct {
    size_t    capacity;
    size_t    used;
    uint64_t *ids;      // array with <used> elements
} PIDs_t;

void PIDs_init(PIDs_t *head);
PIDs_t *PIDs_create();
PIDs_t *PIDs_load(PIDs_t *head);
void PIDs_clear(PIDs_t *head);
void PIDs_destroy(PIDs_t *head);

#endif // __READ_PROC_H__
