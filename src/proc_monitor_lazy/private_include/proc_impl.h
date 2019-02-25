#ifndef __PROC_IMPLEMENT_H__
#define __PROC_IMPLEMENT_H__

#include "proc.h"
#include <inttypes.h>

Proc *ReadSingleProc(uint64_t pid);
void ProcFree(Proc *proc);
bool is_ino_in_fds(Proc *proc, ino_t ino);
bool is_path_in_fds(Proc *proc, const char *path);
bool is_path_in_libs(Proc *proc, const char *path);

#endif /* __PROC_IMPLEMENT_H__ */
