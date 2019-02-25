#include <netdb.h>      // getservbyport
#include <sys/types.h>  // getpwuid, kill, open
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>        // getpwuid
#include <sys/sysinfo.h>// sysinfo
#include <sys/statfs.h> // statfs
#include <utmp.h>       // get_logging_users
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdio.h>      // remove
#include <stdlib.h>
#include <signal.h>    // kill

#include "util-system.h"
#include "util-string.h"

int get_serv_name_by_port(uint16_t port, const char *proto, char *buff, size_t buff_size)
{
    struct servent *sv = getservbyport(htons(port), proto);
    if(sv)
    {
        if(sv->s_name)
        {
            strncpy(buff, sv->s_name, buff_size);
            return 0;
        }
    }
    return -1;
}

// int get_user_name_by_uid(uid_t uid, char *user, size_t size)
// {
// // int getpwuid_r(uid_t uid, struct passwd *pwd,
// //                       char *buf, size_t buflen, struct passwd **result);
//     struct passwd pwd, *p_pwd = NULL;
//     user[0] = 0;
//     char buff[1024] = {0};
//     if(0 == getpwuid_r(uid, &pwd, buff, sizeof(buff), &p_pwd))
// {
//         strncpy(user, pwd.pw_name, size);
//         return 0;
//     }
//     return -1;
// }

time_t get_sys_boot_timestamp()
{
    static time_t boot_time = 0;
    if(0==boot_time)
    {
        struct sysinfo sys_info;
        if(sysinfo(&sys_info))  return 0;
        boot_time = time(NULL) - (time_t)sys_info.uptime;
    }
    return boot_time;
}

int get_clock_ticks()
{
    static int clock_ticks = 1;
    if(1==clock_ticks)
    {
        clock_ticks = sysconf(_SC_CLK_TCK);
    }
    return clock_ticks;
}

int get_cpu_status(cpu_status_t *cpu)
{
    if(!cpu) return 1;

    char buff[256];
    FILE *fp = fopen("/proc/stat", "r");
    if(!fp) return -1;

    fgets(buff, sizeof(buff), fp);
    memset(cpu, 0, sizeof(cpu_status_t));
    sscanf(buff, "%s %lu %lu %lu %lu", cpu->name, &cpu->user, &cpu->nice, &cpu->system, &cpu->idle);

    fclose(fp);
    return 0;
}

float calc_cpu_rate(cpu_status_t *small, cpu_status_t *big)
{
    unsigned long   old_cpu_Time, new_cpu_Time;
    unsigned long   usr_Time_Diff, sys_Time_Diff, nic_Time_Diff;
    float           cpu_use = 0.0;

    old_cpu_Time = (unsigned long)(small->user + small->nice + small->system + small->idle);
    new_cpu_Time = (unsigned long)(big->user + big->nice + big->system + big->idle);
    usr_Time_Diff = (unsigned long)(big->user - small->user);
    sys_Time_Diff = (unsigned long)(big->system - small->system);
    nic_Time_Diff = (unsigned long)(big->nice -small->nice);

    if(new_cpu_Time != old_cpu_Time)
        cpu_use = (float)100*(usr_Time_Diff + sys_Time_Diff + nic_Time_Diff)/(new_cpu_Time - old_cpu_Time);
    else
        cpu_use = 0.0;

    return cpu_use;
}

int get_mem_status(mem_status_t *mem)
{
    if(!mem)    return 0;
    memset(mem, 0, sizeof(mem_status_t));

    char buff[256];
    FILE *fp = fopen("/proc/meminfo", "r");
    if(!fp) return -1;

    char *ptr;
    while(fgets(buff, sizeof(buff), fp))
    {
        if(0==mem->total && 0==strncmp(buff, "MemTotal:", sizeof("MemTotal:")-1))
        {
            ptr = buff+sizeof("MemTotal:");
            mem->total = (unsigned long)atol(ptr);
        }
        else if(0==mem->free && 0==strncmp(buff, "MemFree:", sizeof("MemFree:")-1))
        {
            ptr = buff+sizeof("MemFree:");
            mem->free = (unsigned long)atol(ptr);
        }
        else if(0==mem->buffers && 0==strncmp(buff, "Buffers:", sizeof("Buffers:")-1))
        {
            ptr = buff+sizeof("Buffers:");
            mem->buffers = (unsigned long)atol(ptr);
        }
        else if(0==mem->cached && 0==strncmp(buff, "Cached:", sizeof("Cached:")-1))
        {
            ptr = buff+sizeof("Cached:");
            mem->cached = (unsigned long)atol(ptr);
        }
        else if(0==mem->swaptotal && 0==strncmp(buff, "SwapTotal:", sizeof("SwapTotal:")-1))
        {
            ptr = buff+sizeof("SwapTotal:");
            mem->swaptotal = (unsigned long)atol(ptr);
        }
        else if(0==mem->swapfree && 0==strncmp(buff, "SwapFree:", sizeof("SwapFree:")-1))
        {
            ptr = buff+sizeof("SwapFree:");
            mem->swapfree = (unsigned long)atol(ptr);
        }
    }
    if(mem->total > (mem->free + mem->buffers + mem->cached))
        mem->used = mem->total - (mem->free + mem->buffers + mem->cached);

    fclose(fp);
    return 0;
}

int get_disk_status(disk_status_t *disk)
{
    struct statfs fs;
    if(statfs("/", &fs))    return -1;
    disk->total = fs.f_blocks * fs.f_bsize / 1024;
    disk->avail = fs.f_bavail * fs.f_bsize / 1024;
    disk->free  = fs.f_bfree  * fs.f_bsize / 1024;
    disk->used  = disk->total - disk->free;
    return 0;
}

// int get_logging_users(users_info_t *users)
// {
//     FILE *ufp;
//     struct utmp usr;

//     ufp = fopen(_PATH_UTMP, "r");
//      if(!ufp)    return -1;

//      memset(users, 0, sizeof(users_info_t));
//      while (fread((char *)&usr, sizeof(usr), 1, ufp) == 1)
//      {
//          if (*usr.ut_name && *usr.ut_line && *usr.ut_line != '~')
//          {
//              int i, exists_flag=0;
//              for(i=0; i<users->user_nums; i++)
//              {
//                 if(0==strncmp(users->user_name[i], usr.ut_name, strlen(usr.ut_name)+1))
    // {
//                     exists_flag = 1;
//                     break;
//                 }
//             }
//             if(exists_flag)    continue;
//             if(users->user_nums == sizeof(users->user_name)/sizeof(users->user_name[0]))
//                 continue;
//             strncpy(users->user_name[i], usr.ut_name, sizeof(users->user_name[i]));
//             users->user_nums++;
//         }
//     }
//     fclose(ufp);
//     return 0;
// }

int kill_process(pid_t pid, int sig)
{
    return kill(pid, sig);
}

int delete_file(const char *fname)
{
    return remove(fname);
}

int move_file(const char *old_path, const char *new_path)
{
    int ret = -1;
    int old_fd = open(old_path, O_RDONLY);
    if(old_fd > 0)
    {
        int new_fd = open(new_path, O_RDWR|O_CREAT|O_TRUNC, 0777);
        if(new_fd > 0)
        {
            char buff[256];
            size_t readn;
            while(1)
            {
                readn = read(old_fd, buff, sizeof(buff));
                if(readn <= 0)
                {
                    break;
                }
                write(new_fd, buff, readn);
            }
            ret = 0;
            close(new_fd);
        }
        close(old_fd);
        if(ret == 0)
            ret = delete_file(old_path);
    }
    return ret;
}

#ifdef EDR_DEBUG
#define PRINT(fmt, arg...)      printf("[%s] " fmt, __func__, ##arg)
#else
#define PRINT(fmt, arg...)      
#endif 

#define PASSWD_FILE         "/etc/passwd"
#define PASSWD_FILE_COLUMN  7

__attribute__ ((unused)) static int get_file_lines(const char *fname)
{
    FILE *file = fopen(fname, "r");
    int total = 0;
    if(file)
    {
        int c;
        while(EOF != (c = fgetc(file)))
            if('\n' == c)
                total++; 
        fclose(file);
    }
    return total;
}

static int user_passwd_prepare_alloc(users_passwd_t *users)
{
    if(users->capacity == users->used)
    {
        size_t capacity = users->capacity * 5 / 4 + 1;
        user_passwd_t *ptr = (user_passwd_t *)realloc(users->users, capacity * sizeof(user_passwd_t));
        if(!ptr)
        {
            return -1;
        }
        memset(ptr + users->capacity, 0, (capacity - users->capacity) * sizeof(user_passwd_t));
        users->users = ptr;
        users->capacity = capacity;
    }
    return 0;
}

void users_passwd_free(users_passwd_t *users)
{
    if(users)
    {
        if(users->users)
        {
            for(size_t index=0; index<users->used; index++)
            {
                free(users->users[index].logname);
                free(users->users[index].passwd);
                free(users->users[index].name);
                free(users->users[index].home);
                free(users->users[index].shell);
            }
            free(users->users);
        }
        free(users);
    }
}

static int get_users_passwd_impl(users_passwd_t *users)
{
    FILE *file = fopen(PASSWD_FILE, "r");
    if(!file)
    {
        PRINT("open %s failed\n", PASSWD_FILE);
        return -1;
    }
    char line[4096];
    users->used = 0;
    while(NULL != fgets(line, sizeof(line), file))
    {
        char *start = str_strip(line);
        char **toks;
        int num_toks = 0;
        toks = str_split(start, ':', &num_toks);
        if(PASSWD_FILE_COLUMN != num_toks)
        {
            str_split_free(toks, num_toks);
            continue;
        }
        if(user_passwd_prepare_alloc(users) < 0)
            continue;
        if(NULL != strchr(toks[4], ','))
            *strchr(toks[4], ',') = '\0';
        users->users[users->used].logname  = strdup(toks[0]);
        users->users[users->used].passwd   = strdup(toks[1]);
        users->users[users->used].uid      = atoi(toks[2]);
        users->users[users->used].gid      = atoi(toks[3]);
        users->users[users->used].name     = strdup(toks[4]);
        users->users[users->used].home     = strdup(toks[5]);
        users->users[users->used].shell    = strdup(toks[6]);
        str_split_free(toks, num_toks);
        users->used++;
    }
    fclose(file);
    return 0;
}

users_passwd_t *get_users_passwd()
{
    users_passwd_t *users = (users_passwd_t *)calloc(1, sizeof(users_passwd_t));
    if(!users)
        return NULL;
    if(0 == get_users_passwd_impl(users))
    {
        return users;
    }
    else
    {
        users_passwd_free(users);
    }
    return NULL;
}

void users_passwd_print(users_passwd_t *users)
{
    if(!users)
        return;
    printf("[%s] print all users in /etc/passwd :\n", __func__);
    for(size_t index=0; index<users->used; index++)
    {
        printf("%s :\n", users->users[index].name);
        printf("\tlogname: %s\n", users->users[index].logname);
        printf("\tpasswd : %s\n", users->users[index].passwd);
        printf("\tuid    : %d\n", users->users[index].uid);
        printf("\tgid    : %d\n", users->users[index].gid);
        printf("\tname   : %s\n", users->users[index].name);
        printf("\thome   : %s\n", users->users[index].home);
        printf("\tshell  : %s\n", users->users[index].shell);
    }
}

char *get_user_name_by_uid(users_passwd_t *users, int uid)
{
    for(size_t index=0; index < users->used; index++)
    {
        if(users->users[index].uid == uid)
            return users->users[index].logname;
    }
    return NULL;
}

// /var/run/utmp  --  database of currently logged-in users
// /var/log/wtmp  --  database of past user logins

// /usr/include/bits/utmp.h --> struct utmp
// The structure describing the status of a terminated process. 
// This type is used in `struct utmp' below.

// struct exit_status
// {
//     short int e_termination;  // Process termination status.
//     short int e_exit;         // Process exit status.
// };

// The structure describing an entry in the user accounting database.
// struct utmp
// {
//   short int ut_type;          // Type of login.
//   pid_t ut_pid;               // Process ID of login process.
//   char ut_line[UT_LINESIZE];  // Devicename.
//   char ut_id[4];              // Inittab ID.
//   char ut_user[UT_NAMESIZE];  // Username.
//   char ut_host[UT_HOSTSIZE];  // Hostname for remote login.
//   struct exit_status ut_exit; // Exit status of a process marked as DEAD_PROCESS.

//   The ut_session and ut_tv fields must be the same size when compiled 32 && 64 bit.
//   This allows data files and shared memory to be shared between 32 && 64-bit applications.

// #if __WORDSIZE == 64 && defined __WORDSIZE_COMPAT32
//   int32_t ut_session;         // Session ID, used for windowing.
//   struct
//   {
//     int32_t tv_sec;           // Seconds.
//     int32_t tv_usec;          // Microseconds.
//   } ut_tv;                    // Time entry was made.
// #else
//   long int ut_session;        // Session ID, used for windowing.
//   struct timeval ut_tv;       // Time entry was made.
// #endif

//   int32_t ut_addr_v6[4];      // Internet address of remote host.
//   char __unused[20];          // Reserved for future use.
// };

// // Values for the `ut_type' field of a `struct utmp'. 
// #define EMPTY           0       // No valid user accounting information.
// #define RUN_LVL         1       // The system's runlevel.
// #define BOOT_TIME       2       // Time of system boot. 
// #define NEW_TIME        3       // Time after system clock changed. 
// #define OLD_TIME        4       // Time when system clock changed. 
// #define INIT_PROCESS    5       // Process spawned by the init process. 
// #define LOGIN_PROCESS   6       // Session leader of a logged in user. 
// #define USER_PROCESS    7       // Normal process. 
// #define DEAD_PROCESS    8       // Terminated process. 
// #define ACCOUNTING      9

static user_utmp_t *get_user_by_name(users_utmp_t *users, const char *name)
{
    if(!users || !users->users)
        return NULL;
    for(size_t index=0; index<users->used; index++)
    {
        if(0 == strcmp(users->users[index].name, name))
            return &users->users[index];
    }
    return NULL;
}

void users_utmp_free(users_utmp_t *users)
{
    if(users)
    {
        if(users->users)
        {
            for(size_t index=0; index<users->used; index++)
            {
                free(users->users[index].name);
            }
            free(users->users);
        }
        free(users);
    }
}

static int user_utmp_prepare_alloc(users_utmp_t *users)
{
    if(users->capacity == users->used)
    {
        size_t capacity = users->capacity * 5 / 4 + 1;
        user_utmp_t *ptr = (user_utmp_t *)realloc(users->users, capacity * sizeof(user_utmp_t));
        if(!ptr)
        {
            return -1;
        }
        memset(ptr + users->capacity, 0, (capacity - users->capacity) * sizeof(user_utmp_t));
        users->users = ptr;
        users->capacity = capacity;
    }
    return 0;
}

static int get_users_utmp_impl(users_utmp_t *users)
{
#ifndef __linux__
    return -1;
#endif
    const char *path = NULL;
    // /usr/include/paths.h --> #define _PATH_UTMP "/var/run/utmp"
    path = _PATH_UTMP;
    FILE *file = fopen(path, "r");
    if(!file)
    {
        printf("%s open failed\n", path);
        return -1;
    }
    struct utmp usr;
    while (fread((char *)&usr, sizeof(usr), 1, file) == 1)
    {
        if (*usr.ut_name && *usr.ut_line && *usr.ut_line != '~')
        {
            if(get_user_by_name(users, usr.ut_name))
                continue;
            if(user_utmp_prepare_alloc(users) < 0)
                continue;
            PRINT("%s\n", usr.ut_name);
            users->users[users->used].name = strdup(usr.ut_name);
            users->used++;
        }
    }
    fclose(file);
    return 0;
}

users_utmp_t *get_users_utmp()
{
    users_utmp_t *users = (users_utmp_t *)calloc(1, sizeof(users_utmp_t));
    if(!users)
        return NULL;
    if(0 == get_users_utmp_impl(users))
    {
        return users;
    }
    else
    {
        users_utmp_free(users);
    }
    return NULL;
}

void users_utmp_print(users_utmp_t *users)
{
    if(!users)
        return;
    printf("[%s] print currently logged-in users:\n", __func__);
    for(size_t index=0; index<users->used; index++)
    {
        printf("%s,", users->users[index].name);
    }
    printf("\b \n");
}

int get_logging_users(char *buff, size_t buff_size)
{
    if(!buff)
        return 0;
    buff[0] = 0;
    users_utmp_t *users = get_users_utmp();
    if(users)
    {
        for(size_t i=0; i<users->used; i++)
        {
            snprintf(buff+strlen(buff), buff_size-strlen(buff), "%s,", users->users[i].name);
        }
        users_utmp_free(users);
    }
    if(buff[0])
        buff[strlen(buff)-1] = 0;
    return 0;
}
