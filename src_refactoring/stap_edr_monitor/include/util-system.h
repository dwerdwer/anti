#ifndef __UTIL_SYSTEM_H__
#define __UTIL_SYSTEM_H__
#include <netdb.h>

#include <sys/types.h>
#include <pwd.h>
#include <signal.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#if 0
struct servent {
   char  *s_name;       /* official service name */
   char **s_aliases;    /* alias list */
   int    s_port;       /* port number */
   char  *s_proto;      /* protocol to use */
};

struct passwd {
   char   *pw_name;       /* username */
   char   *pw_passwd;     /* user password */
   uid_t   pw_uid;        /* user ID */
   gid_t   pw_gid;        /* group ID */
   char   *pw_gecos;      /* user information */
   char   *pw_dir;        /* home directory */
   char   *pw_shell;      /* shell program */
};
#endif

#ifndef IN 
#define IN
#endif

#ifndef OUT 
#define OUT
#endif

typedef struct cpu_status_ {
    char          name[64];
    unsigned long user;
    unsigned long nice;
    unsigned long system;
    unsigned long idle;
} cpu_status_t;

typedef struct mem_status_ {    // KiB
    unsigned long total;
    unsigned long free;
    unsigned long buffers;
    unsigned long cached;
    unsigned long used;
    unsigned long swaptotal;
    unsigned long swapfree;
} mem_status_t;

typedef struct disk_status_ {   // KiB
    unsigned long total;
    unsigned long avail;
    unsigned long free;
    unsigned long used;
} disk_status_t;

// getservbyport:    get the process name<->port info
    // params:     proto : "tcp", "udp", set NULL if don't know which belongs to
int get_serv_name_by_port(uint16_t port, const char *proto, OUT char *buff, size_t buff_size);

// timestamp of system boot up
time_t get_sys_boot_timestamp();

// di-da clock
int get_clock_ticks();

int get_cpu_status(cpu_status_t *cpu);
float calc_cpu_rate(cpu_status_t *small, cpu_status_t *big);
int get_mem_status(mem_status_t *mem);
int get_disk_status(disk_status_t *disk);
// int get_logging_users(users_info_t *users);

int kill_process(pid_t pid, int sig);
int delete_file(const char *fname);
int move_file(const char *old_path, const char *new_path);


typedef struct {
    char *logname;  // name when log in
    char *passwd;   // password, usually is 'x'.
    int uid;        // user id
    int gid;        // group id
    char *name;     // detail name, usually same as logname
    char *home;     // home directory
    char *shell;    // default shell
} user_passwd_t;

typedef struct {
    size_t        capacity;
    size_t        used;
    user_passwd_t *users;
} users_passwd_t;

// get from /etc/passwd
void users_passwd_free(users_passwd_t *users);
void users_passwd_print(users_passwd_t *users);
users_passwd_t *get_users_passwd();

char *get_user_name_by_uid(users_passwd_t *users, int uid);


typedef struct {
    char *name;
} user_utmp_t;

typedef struct {
    size_t capacity;
    size_t used;
    user_utmp_t *users;
} users_utmp_t;

void users_utmp_free(users_utmp_t *users);
void users_utmp_print(users_utmp_t *users);
users_utmp_t *get_users_utmp();

int get_logging_users(char *buff, size_t buff_size);

#ifdef __cplusplus
}
#endif

#endif // __UTIL_SYSTEM_H__
