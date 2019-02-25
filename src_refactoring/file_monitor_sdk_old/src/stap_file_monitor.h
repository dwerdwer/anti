#ifndef __STAP_MONITOR_H__
#define __STAP_MONITOR_H__

#include <inttypes.h>
#include <time.h>

#define MAX(a, b) ((a)>(b) ? (a) : (b))
#define MIN(a, b) ((a)<(b) ? (a) : (b))

#define MAX_TOKS 20 // (MAX(MAX(INDEX_PROC_MAX, INDEX_NETWORK_MAX), INDEX_FILE_MAX))

#define STAP_FILE_ACTION_OPEN   1
#define STAP_FILE_ACTION_UNLINK 2
#define STAP_FILE_ACTION_RENAME 3
#define STAP_FILE_ACTION_CHMOD  4
#define STAP_FILE_ACTION_CHOWN  5
#define STAP_FILE_ACTION_MKDIR  6
#define STAP_FILE_ACTION_RMDIR  7

typedef struct {
    int         action;
    int         pid;
    int         ppid;
    const char *file_name;      // absolute file name
    time_t      time;
} stap_file_action;

typedef void (*stap_action_cb)(const void *action, void *p_user_data);

typedef struct {
    stap_action_cb   p_callback;
    void            *p_user_data;
} stap_interface_t;

typedef struct stap_monitor_ {
    FILE           *rfp;
    int             running;

    stap_interface_t   itf;
} stap_monitor;

enum tokes_file_index {
    INDEX_FILE_TIMESTAMP    = 0,
    INDEX_FILE_PARENT_PID   = 1,
    INDEX_FILE_PID          = 2,
    INDEX_FILE_FILE_NAME    = 3,
    INDEX_FILE_PWD          = 4,
    INDEX_FILE_ACTION       = 5,
    INDEX_FILE_MAX
};

struct kv_p_2_i {
    const char *p;
    int         i;
};

#ifdef __cplusplus
extern "C" {
#endif

stap_monitor *init_stap_monitor(const char *exe, const char *stp, const stap_interface_t *info);

void stop_stap_monitor(stap_monitor *p_stap_monitor);

void fin_stap_monitor(stap_monitor *p_stap_handle);

#ifdef __cplusplus
}
#endif

#endif /* __STAP_MONITOR_H__ */
