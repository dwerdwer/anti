#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>

#include <map>
#include <string>

#include "proc.h"

#define PROC_PATH   "/proc"

#ifndef unlikely
#define unlikely
#endif

#ifndef likely
#define likely
#endif

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

#if 1   // procps-v3.3.9

#if 0
static const char *ns_names[] = {
    [IPCNS] = "ipc",
    [MNTNS] = "mnt",
    [NETNS] = "net",
    [PIDNS] = "pid",
    [USERNS] = "user",
    [UTSNS] = "uts",
};
#endif

// dynamic 'utility' buffer support for file2str() calls
struct utlbuf_s {
    char *buf;     // dynamically grown buffer
    int   siz;     // current len of the above
    int   used;    // read size in buf
} utlbuf_s;

#ifndef SIGNAL_STRING
// convert hex string to unsigned long long
static unsigned long long unhex(const char *cp){
    unsigned long long ull = 0;
    for(;;){
        char c = *cp++;
        if(unlikely(c<0x30)) break;
        ull = (ull<<4) | (c - (c>0x57) ? 0x57 : 0x30) ;
    }
    return ull;
}
#endif

// #define LABEL_OFFSET
typedef struct status_table_struct {
    unsigned char name[8];        // /proc/*/status field name
    unsigned char len;            // name length
#ifdef LABEL_OFFSET
    long offset;                  // jump address offset
#else
    void *addr;
#endif
} status_table_struct;


#ifdef LABEL_OFFSET
#define F(x) {#x, sizeof(#x)-1, (long)(&&case_##x-&&base)},
#else
#define F(x) {#x, sizeof(#x)-1, &&case_##x},
#endif
#define NUL  {"", 0, 0},

// Derived from:
// gperf -7 --language=ANSI-C --key-positions=1,3,4 -C -n -c <if-not-piped>
//
// Suggested method:
// Grep this file for "case_", then strip those down to the name.
// Eliminate duplicates (due to #ifs), the '    case_' prefix and
// any c comments.  Leave the colon and newline so that "Pid:\n",
// "Threads:\n", etc. would be lines, but no quote, no escape, etc.
//
// After a pipe through gperf, insert the resulting 'asso_values'
// into our 'asso' array.  Then convert the gperf 'wordlist' array
// into our 'table' array by wrapping the string literals within
// the F macro and replacing empty strings with the NUL define.
//
// In the status_table_struct watch out for name size (grrr, expanding)
// and the number of entries (we mask with 63 for now). The table
// must be padded out to 64 entries, maybe 128 in the future.

static int file2str(const char *path, struct utlbuf_s *ub)
{
#define buffGRW 1024
    int fd, num, tot_read = 0;

    /* on first use we preallocate a buffer of minimum size to emulate
       former 'local static' behavior -- even if this read fails, that
       buffer will likely soon be used for another subdirectory anyway
       ( besides, with this calloc we will never need to use memcpy ) */
    if (ub->buf) ub->buf[0] = '\0';
    else ub->buf = (char *)calloc(1, (ub->siz = buffGRW));
    if (-1 == (fd = open(path, O_RDONLY, 0))) return -1;
    while (0 < (num = read(fd, ub->buf + tot_read, ub->siz - tot_read))) {
        tot_read += num;
        if (tot_read < ub->siz) break;
        char *ptr = (char *)realloc(ub->buf, (ub->siz + buffGRW));
        assert(ptr);
        ub->buf = ptr;
        ub->siz += buffGRW;
    };
    ub->buf[tot_read] = '\0';
    close(fd);
    if (unlikely(tot_read < 1)) return -1;
    ub->used = tot_read;
    return tot_read;
#undef buffGRW
}

static void ReadStatus(char *s, Proc *p, int is_proc) {
    long Threads = 0;
    long Tgid = 0;
    long Pid = 0;

    // 128 entries because we trust the kernel to use ASCII names
    static const unsigned char asso[] = {
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 28, 64,
      64, 64, 64, 64, 64, 64,  8, 25, 23, 25,
       6, 25,  0,  3, 64, 64,  3, 64, 25, 64,
      20,  1,  1,  5,  0, 30,  0,  0, 64, 64,
      64, 64, 64, 64, 64, 64, 64,  3, 64,  0,
       0, 18, 64, 10, 64, 10, 64, 64, 64, 20,
      64, 20,  0, 64, 25, 64,  3, 15, 64,  0,
      30, 64, 64, 64, 64, 64, 64, 64
    };

    static const status_table_struct table[] = {
        F(VmHWM)
        NUL NUL
        F(VmLck)
        NUL
        F(VmSwap)
        F(VmRSS)
        NUL
        F(VmStk)
        NUL
        F(Tgid)
        F(State)
        NUL
        F(VmLib)
        NUL
        F(VmSize)
        F(SigQ)
        NUL
        F(SigIgn)
        NUL
        F(VmPTE)
        F(FDSize)
        NUL
        F(SigBlk)
        NUL
        F(ShdPnd)
        F(VmData)
        NUL
        F(CapInh)
        NUL
        F(PPid)
        NUL NUL
        F(CapBnd)
        NUL
        F(SigPnd)
        NUL NUL
        F(VmPeak)
        NUL
        F(SigCgt)
        NUL NUL
        F(Threads)
        NUL
        F(CapPrm)
        NUL NUL
        F(Pid)
        NUL
        F(CapEff)
        NUL NUL
        F(Gid)
        NUL
        F(VmExe)
        NUL NUL
        F(Uid)
        NUL
        F(Groups)
        NUL NUL
        F(Name)
    };
#undef F
#undef NUL

// ENTER(0x220);

    goto base;

    for(;;){
        char *colon;
        status_table_struct entry;

        // advance to next line
        s = strchr(s, '\n');
        if(unlikely(!s))    break;  // if no newline
        s++;

        // examine a field name (hash and compare)
    base:
        if(unlikely(!*s))   break;
        entry = table[63 & (asso[(int)s[3]] + asso[(int)s[2]] + asso[(int)s[0]])];
        colon = strchr(s, ':');
        if(unlikely(!colon))    break;
        if(unlikely(colon[1]!='\t'))    break;
        if(unlikely(colon-s != entry.len))  continue;
        if(unlikely(memcmp(entry.name,s,colon-s)))  continue;

        // past the '\t'
        s = colon+2;

#ifdef LABEL_OFFSET
        goto *(&&base + entry.offset);
#else
        goto *entry.addr;
#endif

    case_Name:
        {
            unsigned u = 0;
            p->cmd[u++] = '[';
            while(u < sizeof p->cmd - 1u){
                int c = *s++;
                if(unlikely(c=='\n')) break;
                if(unlikely(c=='\0')) break; // should never happen
                if(unlikely(c=='\\')){
                    c = *s++;
                    if(c=='\n') break; // should never happen
                    if(!c)      break; // should never happen
                    if(c=='n') c='\n'; // else we assume it is '\\'
                }
                p->cmd[u++] = c;
            }
            p->cmd[u++] = ']';
            p->cmd[u] = '\0';
            s--;   // put back the '\n' or '\0'
            continue;
        }
#ifdef SIGNAL_STRING
    case_ShdPnd:
        memcpy(p->signal, s, 16);
        p->signal[16] = '\0';
        continue;
    case_SigBlk:
        memcpy(p->blocked, s, 16);
        p->blocked[16] = '\0';
        continue;
    case_SigCgt:
        memcpy(p->sigcatch, s, 16);
        p->sigcatch[16] = '\0';
        continue;
    case_SigIgn:
        memcpy(p->sigignore, s, 16);
        p->sigignore[16] = '\0';
        continue;
    case_SigPnd:
        memcpy(p->_sigpnd, s, 16);
        p->_sigpnd[16] = '\0';
        continue;
#else
    case_ShdPnd:
        p->signal = unhex(s);
        continue;
    case_SigBlk:
        p->blocked = unhex(s);
        continue;
    case_SigCgt:
        p->sigcatch = unhex(s);
        continue;
    case_SigIgn:
        p->sigignore = unhex(s);
        continue;
    case_SigPnd:
        p->_sigpnd = unhex(s);
        continue;
#endif
    case_State:
        p->state = *s;
        continue;
    case_Tgid:
        Tgid = strtol(s,&s,10);
        continue;
    case_Pid:
        Pid = strtol(s,&s,10);
        continue;
    case_PPid:
        p->ppid = strtol(s,&s,10);
        continue;
    case_Threads:
        Threads = strtol(s,&s,10);
        continue;
    case_Uid:
        p->ruid = strtol(s,&s,10);
        p->euid = strtol(s,&s,10);
        p->suid = strtol(s,&s,10);
        p->fuid = strtol(s,&s,10);
        continue;
    case_Gid:
        p->rgid = strtol(s,&s,10);
        p->egid = strtol(s,&s,10);
        p->sgid = strtol(s,&s,10);
        p->fgid = strtol(s,&s,10);
        continue;
    case_VmData:
        p->vm_data = strtol(s,&s,10);
        continue;
    case_VmExe:
        p->vm_exe = strtol(s,&s,10);
        continue;
    case_VmLck:
        p->vm_lock = strtol(s,&s,10);
        continue;
    case_VmLib:
        p->vm_lib = strtol(s,&s,10);
        continue;
    case_VmRSS:
        p->vm_rss = strtol(s,&s,10);
        continue;
    case_VmSize:
        p->vm_size = strtol(s,&s,10);
        continue;
    case_VmStk:
        p->vm_stack = strtol(s,&s,10);
        continue;
    case_VmSwap: // Linux 2.6.34
        p->vm_swap = strtol(s,&s,10);
        continue;
    case_Groups:
        {
            continue;
        }
    case_CapBnd:
    case_CapEff:
    case_CapInh:
    case_CapPrm:
    case_FDSize:
    case_SigQ:
    case_VmHWM: // 2005, peak VmRSS unless VmRSS is bigger
    case_VmPTE:
    case_VmPeak: // 2005, peak VmSize unless VmSize is bigger
        continue;
    }

#if 0
    // recent kernels supply per-tgid pending signals
    if(is_proc && *ShdPnd){
        memcpy(p->signal, ShdPnd, 16);
        p->signal[16] = '\0';
    }
#endif

    // recent kernels supply per-tgid pending signals
#ifdef SIGNAL_STRING
    if(!is_proc || !p->signal[0]) {
        memcpy(p->signal, p->_sigpnd, 16);
        p->signal[16] = '\0';
    }
#else
    if(!is_proc) {
        p->signal = p->_sigpnd;
    }
#endif

    // Linux 2.4.13-pre1 to max 2.4.xx have a useless "Tgid"
    // that is not initialized for built-in kernel tasks.
    // Only 2.6.0 and above have "Threads" (nlwp) info.

    if(Threads) {
        p->nlwp = Threads;
        p->tgid = Tgid;     // the POSIX PID value
        p->tid  = Pid;      // the thread ID
    } else {
        p->nlwp = 1;
        p->tgid = Pid;
        p->tid  = Pid;
    }

// LEAVE(0x220);
}

// Reads /proc/*/stat files, being careful not to trip over processes with
// names like ":-) 1 2 3 4 5 6".
static void ReadStat(char *s, Proc *proc)
{
    unsigned num;
    char* tmp;

// ENTER(0x160);

    /* fill in default values for older kernels */
    proc->processor = 0;
    proc->rtprio = -1;
    proc->sched = -1;
    proc->nlwp = 0;

    s = strchr(s, '(') + 1;
    tmp = strrchr(s, ')');
    num = tmp - s;
    if(unlikely(num >= sizeof(proc->cmd)))
        num = sizeof(proc->cmd) - 1;
    memcpy(proc->cmd, s, num);
    proc->cmd[num] = '\0';
    {
        if(0 == proc->name.size()) {
            char cmd[20] = {0};
            strcat(cmd, "[");
            strcat(cmd, proc->cmd);
            strcat(cmd, "]");
            strcpy(proc->cmd, cmd);
        }
    }
    s = tmp + 2;                 // skip ") "

    num = sscanf(s,
       "%c "
       "%d %d %d %d %d "
       "%lu %lu %lu %lu %lu "
       "%Lu %Lu %Lu %Lu "  /* utime stime cutime cstime */
       "%ld %ld "
       "%d "
       "%ld "
       "%llu "  /* start_time */
       "%lu "
       "%ld "
       "%lu %" KLF "u %" KLF "u %" KLF "u %" KLF "u %" KLF "u "
       "%*s %*s %*s %*s " /* discard, no RT signals & Linux 2.1 used hex */
       "%" KLF "u %*u %*u "
       "%d %d "
       "%lu %lu",
       &proc->state,
       &proc->ppid, &proc->pgrp, &proc->session, &proc->tty, &proc->tpgid,
       &proc->flags, &proc->min_flt, &proc->cmin_flt, &proc->maj_flt, &proc->cmaj_flt,
       &proc->utime, &proc->stime, &proc->cutime, &proc->cstime,
       &proc->priority, &proc->nice,
       &proc->nlwp,
       &proc->alarm,
       &proc->start_time,
       &proc->vsize,
       &proc->rss,
       &proc->rss_rlim, &proc->start_code, &proc->end_code, &proc->start_stack, &proc->kstk_esp, &proc->kstk_eip,
/*     proc->signal, proc->blocked, proc->sigignore, proc->sigcatch,   */ /* can't use */
       &proc->wchan, /* &proc->nswap, &proc->cnswap, */  /* nswap and cnswap dead for 2.4.xx and up */
/* -- Linux 2.0.35 ends here -- */
       &proc->exit_signal, &proc->processor,  /* 2.2.1 ends with "exit_signal" */
/* -- Linux 2.2.8 to 2.5.17 end here -- */
       &proc->rtprio, &proc->sched  /* both added to 2.5.18 */
    );

    if(!proc->nlwp){
        proc->nlwp = 1;
    }

// LEAVE(0x160);
}

static void ReadStatm(const char *s, Proc *proc) {
    sscanf(s, "%ld %ld %ld %ld %ld %ld %ld",
       &proc->size, &proc->resident, &proc->share,
       &proc->trs, &proc->lrs, &proc->drs, &proc->dt);
}
#endif  // procps-v3.3.9

static ino_t file2ino(const char *name)
{
    struct stat st;
    if(stat(name, &st)) return 0;
    return st.st_ino;
}

#define PRG_SOCKET_PFX    "socket:["
#define PRG_SOCKET_PFXl (strlen(PRG_SOCKET_PFX))
static int extract_type_1_socket_inode(const char lname[], ino_t * inode_p)
{
    /* If lname is of the form "socket:[12345]", extract the "12345"
       as *inode_p.  Otherwise, return -1 as *inode_p.
    */
    if (strlen(lname) < PRG_SOCKET_PFXl+3) return -1;

    if (memcmp(lname, PRG_SOCKET_PFX, PRG_SOCKET_PFXl)) return -1;
    if (lname[strlen(lname)-1] != ']') return -1;

    {
        char inode_str[strlen(lname + 1)];  /* e.g. "12345" */
        const int inode_str_len = strlen(lname) - PRG_SOCKET_PFXl - 1;
        char *serr;

        strncpy(inode_str, lname+PRG_SOCKET_PFXl, inode_str_len);
        inode_str[inode_str_len] = '\0';
        *inode_p = (ino_t)strtoul(inode_str, &serr, 0);
        if (!serr || *serr || (int)(*inode_p) == ~0)
            return -1;
    }
    return 0;
}

#define PRG_SOCKET_PFX2   "[0000]:"
#define PRG_SOCKET_PFX2l  (strlen(PRG_SOCKET_PFX2))
static int extract_type_2_socket_inode(const char lname[], ino_t * inode_p)
{
    /* If lname is of the form "[0000]:12345", extract the "12345"
       as *inode_p.  Otherwise, return -1 as *inode_p.
       */

    if (strlen(lname) < PRG_SOCKET_PFX2l+1) return -1;
    if (memcmp(lname, PRG_SOCKET_PFX2, PRG_SOCKET_PFX2l)) return -1;

    {
        char *serr;

        *inode_p = (ino_t)strtoul(lname + PRG_SOCKET_PFX2l, &serr, 0);
        if (!serr || *serr || (int)(*inode_p) == ~0)
            return -1;
    }
    return 0;
}

static int link2name(const char *link, char *name, size_t nameLen)
{
    int n = readlink(link, name, nameLen);
    if(n < 0) {
        name[0] = '\0';
        return n;
    }
    if(n > (int)nameLen)
        n = nameLen;
    name[n] = '\0';
    return n;
}

static int ReadFdCallBack(const char *dir, const char *name, void *arg)
{
    Proc *proc = (Proc *)arg;
    char link_name[PATH_MAX];
    char real_name[PATH_MAX];
    snprintf(link_name, sizeof(link_name), "%s/%s", dir, name);

    int n = link2name(link_name, real_name, sizeof(real_name));
    if(n < 0)   return -1;

    ino_t ino = 0;
    if(extract_type_1_socket_inode(real_name, &ino) < 0)
        extract_type_2_socket_inode(real_name, &ino);
    if(!ino)
        ino = file2ino(real_name);

    proc->fds.push_back(FdInfo(ino, real_name));
    return 0;
}

static int ReadMaps(const char *file_name, Proc *proc)
{
    char line[2048];
    char *line_end;
    FILE *file = fopen(file_name, "r");
    if(!file)   return -1;

    while(fgets(line, sizeof(line), file)) {
        line_end = line+strlen(line);
        if(line_end - line < 30)
            continue;
        if('\r' == *(line_end-2))
            *(line_end-2) = 0;
        if('\n' == *(line_end-1))
            *(line_end-1) = 0;

        char *abs_name = strchr(line, '/');
        if(!abs_name)  continue;
        char *name = strrchr(line, '/') + 1;
        char *end = strstr(line, "so");
        if(!end)    continue;
        while(end<line_end && !isspace(*end))   end++;
        (*end) = '\0';
        if(name >= end) continue;

        proc->libs[abs_name] = DynamicLibInfo(name, abs_name);
    }
    fclose(file);
    return 0;
}

static int ReadCMDLine(struct utlbuf_s *ub, Proc *proc)
{
    for(int i=0; i<ub->used; i++) {
        if(ub->buf[i] == '\0' || isspace(ub->buf[i]))
            ub->buf[i] = ' ';
    }
    ub->buf[ub->used] = '\0';
    proc->cmdline = ub->buf;
    return 0;
}

static int is_directory(const char *name)
{
    struct stat st;
    if(stat(name, &st)) return 0;
    if(!S_ISDIR(st.st_mode)) return 0;

    return 1;
}

static uint64_t gettimeofday_ms()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000ULL + tv.tv_usec/1000ULL;
}

void ProcFree(Proc *proc)
{
    delete proc;
}

Proc::Proc() :
        exec_ino(0), pid(0), comparing_state(0), starttime(0), stoptime(0)
{
    for(char *start=(char *)&tid, *end=(char *)&ns[NUM_NS]; start<end; start++)
        *start = '\0';
}

static Proc *ReadSingleProc(const char *procs_path, const char *pid)
{
    char abs_dir_name[PATH_MAX];
    snprintf(abs_dir_name, sizeof(abs_dir_name), "%s/%s", procs_path, pid);
    if(!is_directory(abs_dir_name))   return NULL;

    char abs_exec_name[PATH_MAX];
    ino_t exec_ino = 0;
    {   /*  /proc/<pid>/exe is a softlink, read it  */
        char abs_soft_link_name[PATH_MAX];
        snprintf(abs_soft_link_name, sizeof(abs_soft_link_name), "%s/exe", abs_dir_name);
        int n = link2name(abs_soft_link_name, abs_exec_name, sizeof(abs_exec_name));
        if(n > 0) {
            // exec inode number
            exec_ino = file2ino(abs_exec_name);
        }
    }

    Proc *proc = new Proc;
    assert(proc);

    proc->pid = strtoull(pid, NULL, 10);
    proc->exec_ino = exec_ino;
    proc->current_time = gettimeofday_ms();
    if(strrchr(abs_exec_name, '/'))
        proc->name = strrchr(abs_exec_name, '/')+1;
    proc->abs_name = abs_exec_name;

    {   // parse /proc/<pid>/fd/  dir
        char abs_fd_dir[PATH_MAX];
        snprintf(abs_fd_dir, sizeof(abs_fd_dir), "%s/%s", abs_dir_name, "fd");
        ReadDir(abs_fd_dir, proc, ReadFdCallBack);
    }


    // printf("pid:[%lu] exec_ino:[%lu] soft:[%s/%s], exec:[%s]\n", 
    //             proc->pid, proc->exec_ino, procs_path, "exe", proc->abs_name);
    {   // parse /proc/<pid>/maps
        char abs_maps_name[PATH_MAX];
        snprintf(abs_maps_name, sizeof(abs_maps_name), "%s/%s", abs_dir_name, "maps");
        ReadMaps(abs_maps_name, proc);
    }

    struct utlbuf_s ub = {NULL, 0, 0};
    {   // parse /proc/<pid>/cmdline
        char abs_cmdline_name[PATH_MAX];
        snprintf(abs_cmdline_name, sizeof(abs_cmdline_name), "%s/%s", abs_dir_name, "cmdline");
        if(file2str(abs_cmdline_name, &ub) > 0)
            ReadCMDLine(&ub, proc);
    }

    {   // parse /proc/<pid>/stat
        char abs_stat_name[PATH_MAX];
        snprintf(abs_stat_name, sizeof(abs_stat_name), "%s/%s", abs_dir_name, "stat");
        if(file2str(abs_stat_name, &ub) > 0)
            ReadStat(ub.buf, proc);
    }

    {   // parse /proc/<pid>/statm
        char abs_statm_name[PATH_MAX];
        snprintf(abs_statm_name, sizeof(abs_statm_name), "%s/%s", abs_dir_name, "statm");
        if(file2str(abs_statm_name, &ub) > 0)
            ReadStatm(ub.buf, proc);
    }

    {   // parse /proc/<pid>/status
        char abs_status_name[PATH_MAX];
        snprintf(abs_status_name, sizeof(abs_status_name), "%s/%s", abs_dir_name, "status");
        if(file2str(abs_status_name, &ub) > 0)
            ReadStatus(ub.buf, proc, 1);
    }
    free(ub.buf);

    return proc;
}

Proc *ReadSingleProc(uint64_t pid)
{
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%" PRIu64 "", pid);
    return ReadSingleProc(PROC_PATH, buffer);
}


bool is_path_in_libs(Proc *proc, const char *path)
{
    for(std::map<std::string, DynamicLibInfo>::iterator it=proc->libs.begin();
            it!=proc->libs.end(); it++) {

        if(0 == strcmp(it->first.c_str(), path)) {
            return true;
        }
    }
    return false;
}

bool is_path_in_fds(Proc *proc, const char *path)
{
    for(std::vector<FdInfo>::iterator it=proc->fds.begin();
        it != proc->fds.end(); it++) {
        if(0 == strcmp(it->name.c_str(), path))
            return true;
    }
    return false;
}

bool is_ino_in_fds(Proc *proc, ino_t ino)
{
    for(std::vector<FdInfo>::iterator it=proc->fds.begin();
        it != proc->fds.end(); it++) {
        if (it->ino == ino)
            return true;
    }
    return false;
}

#ifdef PROC_UNIT_TEST
#include <iostream>
using namespace std;

static int is_number(const char *str)
{
    while(*str) {
        if(!isdigit(*str))
            return 0;
        str++;
    }
    return 1;
}

static int on_proc(const char *dir, const char *name, void *arg)
{
    if(!is_number(name))
        return 0;
    Proc *proc = ReadSingleProc(dir, name);
    if(proc) {
        cout << proc->pid << "\t" << proc->abs_name << endl;
        ProcFree(proc);
    }
    return 0;
}

int main(void)
{
    while(1) {
        ReadDir(PROC_PATH, NULL, on_proc);
        sleep(3);
    }
}
#endif
