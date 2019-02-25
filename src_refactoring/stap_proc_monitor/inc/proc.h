#ifndef __PROC_H__
#define __PROC_H__

#include <map>
#include <vector>
#include <string>

#include <sys/types.h>  // ino_t
#include <inttypes.h>

#if defined(k64test) || (defined(_ABIN32) && _MIPS_SIM == _ABIN32)
#define KLONG long long    // not typedef; want "unsigned KLONG" to work
#define KLF "ll"
#define STRTOUKL strtoull
#else
#define KLONG long
#define KLF "l"
#define STRTOUKL strtoul
#endif

#define PROC_STATE_UNKNOWN      0
#define PROC_STATE_START        1
#define PROC_STATE_CLOSED       2
#define PROC_STATE_RUNNING      3

struct FdInfo {
    FdInfo() {}
    FdInfo(ino_t i, std::string n) :
        ino(i), name(n) {}
    ino_t       ino;
    std::string name;
};

struct DynamicLibInfo {
    DynamicLibInfo() {}
    DynamicLibInfo(std::string n, std::string abs_n) :
        name(n), abs_name(abs_n) {}
    std::string name;
    std::string abs_name;
};

// used in pwcache and in readproc to set size of username or groupname
#define P_G_SZ 20

enum ns_type {
    IPCNS = 0,
    MNTNS,
    NETNS,
    PIDNS,
    USERNS,
    UTSNS,
    NUM_NS         // total namespaces (fencepost)
};

struct Proc {
    ino_t       exec_ino;               // /proc/<pid>/exe -> exec file inode num
    int         pid;                    // pid
    int         comparing_state;        // addtional : compare(time_old, time_now)  state is[1:new_proc, 2:destroy_proc, 3:running_proc]
    time_t      starttime;              // addtional : timestamp(s) of proc start
    time_t      stoptime;               // addtional : timestamp(s) of proc destroy
    uint64_t    current_time;           // addtional : timestamp(ms) of read this proc
    std::string name;                   // filename
    std::string abs_name;               // path/name
    std::string cmdline;                // shell command line

    std::vector<FdInfo> fds;          // file descripter
    std::map<std::string, DynamicLibInfo> libs;   // librarys

// below all elems in procps-v3.3.9

    // 1st 16 bytes
    int
        tid,            // (special)       task id, the POSIX thread ID (see also: tgid)
        ppid;           // stat,status     pid of parent process
    unsigned long       // next 2 fields are NOT filled in by readproc
        maj_delta,      // stat (special) major page faults since last update
        min_delta;      // stat (special) minor page faults since last update
    unsigned
        pcpu;           // stat (special)  %CPU usage (is not filled in by readproc!!!)
    char
        state,          // stat,status     single-char code for process state (S=sleeping)
#ifdef QUICK_THREADS
        pad_1,          // n/a             padding (psst, also used if multi-threaded)
#else
        pad_1,          // n/a             padding
#endif
        pad_2,          // n/a             padding
        pad_3;          // n/a             padding
        // 2nd 16 bytes
    unsigned long long
        utime,          // stat            user-mode CPU time accumulated by process
        stime,          // stat            kernel-mode CPU time accumulated by process
        // and so on...
        cutime,         // stat            cumulative utime of process and reaped children
        cstime,         // stat            cumulative stime of process and reaped children
        start_time;     // stat            the delay:{ (sys boot --> process start) * sysconf(_SC_CLK_TCK) }. If you want this timestamp, Use (time(NULL)-sys.boot_time)+(start_time/sysconf(_SC_CLK_TCK)).
#ifdef SIGNAL_STRING
    char
        // Linux 2.1.7x and up have 64 signals. Allow 64, plus '\0' and padding.
        signal[18],     // status          mask of pending signals, per-task for readtask() but per-proc for readproc()
        blocked[18],    // status          mask of blocked signals
        sigignore[18],  // status          mask of ignored signals
        sigcatch[18],   // status          mask of caught  signals
        _sigpnd[18];    // status          mask of PER TASK pending signals
#else
    long long
        // Linux 2.1.7x and up have 64 signals.
        signal,         // status          mask of pending signals, per-task for readtask() but per-proc for readproc()
        blocked,        // status          mask of blocked signals
        sigignore,      // status          mask of ignored signals
        sigcatch,       // status          mask of caught  signals
        _sigpnd;        // status          mask of PER TASK pending signals
#endif
    unsigned KLONG
        start_code,     // stat            address of beginning of code segment
        end_code,       // stat            address of end of code segment
        start_stack,        // stat            address of the bottom of stack for the process
        kstk_esp,       // stat            kernel stack pointer
        kstk_eip,       // stat            kernel instruction pointer
        wchan;          // stat (special)  address of kernel wait channel proc is sleeping in
    long
        priority,       // stat            kernel scheduling priority
        nice,           // stat            standard unix nice level of process
        rss,            // stat            identical to 'resident'
        alarm,          // stat            ?
        // the next 7 members come from /proc/#/statm
        size,           // statm           total virtual memory (as # pages), usually,  real memory(KB) = size * 4 (4K/page)
        resident,       // statm           resident non-swapped memory (as # pages)
        share,          // statm           shared (mmap'd) memory (as # pages)
        trs,            // statm           text (exe) resident set (as # pages)
        lrs,            // statm           library resident set (always 0 w/ 2.6)
        drs,            // statm           data+stack resident set (as # pages)
        dt;             // statm           dirty pages (always 0 w/ 2.6)
    unsigned long
        vm_size,        // status          equals 'size' (as kb)
        vm_lock,        // status          locked pages (as kb)
        vm_rss,         // status          equals 'rss' and/or 'resident' (as kb)
        vm_data,        // status          data only size (as kb)
        vm_stack,       // status          stack only size (as kb)
        vm_swap,        // status          based on linux-2.6.34 "swap ents" (as kb)
        vm_exe,         // status          equals 'trs' (as kb)
        vm_lib,         // status          total, not just used, library pages (as kb)
        rtprio,         // stat            real-time priority
        sched,          // stat            scheduling class
        vsize,          // stat            number of pages of virtual memory ...
        rss_rlim,       // stat            resident set size limit?
        flags,          // stat            kernel flags for the process
        min_flt,        // stat            number of minor page faults since process start
        maj_flt,        // stat            number of major page faults since process start
        cmin_flt,       // stat            cumulative min_flt of process and child processes
        cmaj_flt;       // stat            cumulative maj_flt of process and child processes
    char
        // Be compatible: Digital allows 16 and NT allows 14 ???
        euser[P_G_SZ],  // stat(),status   effective user name
        ruser[P_G_SZ],  // status          real user name
        suser[P_G_SZ],  // status          saved user name
        fuser[P_G_SZ],  // status          filesystem user name
        rgroup[P_G_SZ], // status          real group name
        egroup[P_G_SZ], // status          effective group name
        sgroup[P_G_SZ], // status          saved group name
        fgroup[P_G_SZ], // status          filesystem group name
        // cmd[16];        // stat,status     basename of executable file in call to exec(2)
        cmd[18];        // stat,status     [basename] of executable file in call to exec(2),  add [] around basename
        // struct proc_t
        // *ring,       // n/a             thread group ring
        // *next;       // n/a             various library uses
    int
        pgrp,           // stat            process group id
        session,        // stat            session id
        nlwp,           // stat,status     number of threads, or 0 if no clue
        tgid,           // (special)       thread group ID, the POSIX PID (see also: tid)
        tty,            // stat            full device number of controlling terminal
        /* FIXME: int uids & gids should be uid_t or gid_t from pwd.h */
        euid, egid,     // stat(),status   effective
        ruid, rgid,     // status          real
        suid, sgid,     // status          saved
        fuid, fgid,     // status          fs (used for file access only)
        tpgid,          // stat            terminal process group id
        exit_signal,    // stat            might not be SIGCHLD
        processor;      // stat            current (or most recent?) CPU
#ifdef OOMEM_ENABLE
    int
        oom_score,      // oom_score       (badness for OOM killer)
        oom_adj;        // oom_adj         (adjustment to OOM score)
#endif
    long
        ns[NUM_NS];     // (ns subdir)     inode number of namespaces
};

#define PUBLIC_API __attribute__((visibility("default")))
PUBLIC_API Proc *read_proc(int pid);
PUBLIC_API Proc *proc_deep_copy(Proc *that);
PUBLIC_API void proc_free(Proc *proc);
#undef PUBLIC_API

#endif /* __PROC_H__ */
