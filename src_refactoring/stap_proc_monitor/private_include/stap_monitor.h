#ifndef __STAP_MONITOR_H__
#define __STAP_MONITOR_H__

#include <time.h>
#include <sys/time.h>

#define STAP_PROC_ACTION_EXIT   1
#define STAP_PROC_ACTION_CREATE 2

typedef struct {
    int     action;
    int     pid;
    int     ppid;
    time_t  time;
} stap_proc_action_t;

typedef void (*stap_action_cb)(const stap_proc_action_t *action, void *p_user_data);

typedef struct {
    stap_action_cb   p_callback;
    void            *p_user_data;
} stap_interface_t;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct stap_monitor stap_monitor_t;

stap_monitor_t *init_stap_monitor(const char *exe, const char *stp, stap_action_cb p_callback, void *p_user_data);
void wait_stap_monitor(stap_monitor_t *p_stap_monitor, struct timeval *timeout);
void fin_stap_monitor(stap_monitor_t *p_stap_monitor);

#ifdef __cplusplus
}
#endif

#endif /* __STAP_MONITOR_H__ */
