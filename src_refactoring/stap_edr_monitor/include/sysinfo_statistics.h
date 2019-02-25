#ifndef __SYSTEM_INFOMATION_STATISTICS__
#define __SYSTEM_INFOMATION_STATISTICS__

#include "edition.h"
#include "defs.h"
#include "cJSON.h"
#include "netstat.h"
#include "proc_statistics.h"
#include "sys_task.h"
#include "white_list.h"
#include "ring_buffer.h"
#include "module_actions.h"
#include "util-system.h"
#include "USBMonitorDef.h"
#include "sysinfo_interface.h"
#include "proc_history.h"
#include "net_history.h"
#include "net_monitor.h"
#include "proc_monitor.h"
#include "uploader_edr.h"
#include "rules_processor.h"

#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdint.h>
#include <map>
#include <vector>
#include <list>
#include <string>
#include <string.h>

EXTERN_C_BEGIN

typedef struct system_stat_ {
    cpu_status_t  cpu_status;
    mem_status_t  mem_status;
    disk_status_t disk_status;
    char  users_login[2048];
} system_stat_t;

struct MainStatistics {
    module_info_t   *p_m_info               = NULL;

    edr::BaseUploader *p_upload             = NULL;
    pthread_t upload_pid                    = 0;

    proc_monitor_t *p_proc_monitor          = NULL;
    pthread_t proc_monitor_pid              = 0;

    net_monitor_t *p_net_monitor            = NULL;
    pthread_t net_monitor_pid               = 0;

    time_t         sys_info_time_prev       = 0;
    time_t         sys_info_time_delta      = 0;

    // for proc action
    ring_buffer_t *proc_actions             = NULL;
    time_t         proc_action_time_prev    = 0;
    time_t         proc_action_time_delta   = 0;

    std::list<cJSON *>          file_list;
    std::list<Proc *>           proc_list;
    std::list<http_info_t *>    http_list;
    std::list<dns_info_t *>     dns_list;
    std::list<icmp_info_t *>    icmp_list;
    std::list<tcp_info_t *>     tcp_list;
    std::list<udp_info_t *>     udp_list;

    //  net_monitor -> push
    //  local -> pop
    pthread_spinlock_t net_monitor_lock;
    ring_buffer_t *httpes       = NULL;
    ring_buffer_t *dnses        = NULL;
    ring_buffer_t *tcpes        = NULL;
    ring_buffer_t *udpes        = NULL;
    ring_buffer_t *icmpes       = NULL;

    // RuleProcessor
    RuleProcessor *p_processor  = NULL;
    rules_info_t *p_rules       = NULL;

    // snapshot
    //  proc_monitor -> push
    //  local -> pop
    ring_buffer_t *snap_shots   = NULL;

    // ring_buffer_t *shot_records;
    time_t         snap_shot_host_time_delta    = 0;
    time_t         snap_shot_host_time_prev     = 0;
    time_t         snap_shot_proc_time_delta    = 0;
    time_t         snap_shot_proc_time_prev     = 0;
    time_t         snap_shot_net_time_delta     = 0;
    time_t         snap_shot_net_time_prev      = 0;

    ps_t         *ps_prev           = NULL;
    ps_t         *ps_curr           = NULL;
    RecordCtx     *conn_prev        = NULL;
    RecordCtx     *conn_curr        = NULL;
    system_stat_t *sys_stat_prev    = NULL;
    system_stat_t *sys_stat_curr    = NULL;

    // sys log task
    //  - interface -> push
    //  - local -> pop
    ring_buffer_t *sys_tasks        = NULL;
    ring_buffer_t *usb_actions      = NULL;
    ring_buffer_t *file_actions     = NULL;

    // action rules
    std::vector<sys_task_action_rule_t *> rules;
    std::multimap<sys_task_action_rule_t *, http_info_t *> rule_packets;

    // md5 cache
    void *md5_cache = NULL;

    // users info:  users_passwd_t is all users.  
    //              users_utmp_t is currently logged-in users
    users_passwd_t *p_users_passwd  = NULL;
    users_utmp_t   *p_users_utmp    = NULL;

    white_list      *white_list     = NULL;

    // log switch
    int enable_network_log      = 0;
    int enable_white_list_log   = 0;

    // cache historys
    // std::map<std::string, url_cache *> net_his;
    net_history net_his;
    proc_history proc_his;
    time_t cache_timeout_prev   = 0;
    time_t cache_timeout_delta  = 60*30;

    std::string local_ip;
    pthread_t   main_pid = 0;
    int         running  = 0;
    edr::edition_t edition;
};

// interface push to here
int sys_task_push(MainStatistics *st, sys_task_t *task);
int usb_action_push(MainStatistics *st, usb_action_t *action);
int file_action_push(MainStatistics *st, file_action_t *action);
void sys_task_free(void *argu);     /* sys_task_t *     */
void usb_action_free(void *argu);   /* usb_action_t *   */
void file_action_free(void *argu);  /* file_action_t *  */

// uploader pop from here
task_record *task_records_pop(MainStatistics *st);
json_file *snap_shot_pop(MainStatistics *st);
json_file *proc_action_pop(MainStatistics *st);
void task_record_free(void *argu);  /* task_record *      */
void json_file_free(void *argu);    /* json_file *    */

MainStatistics *sys_info_statis_create(module_info_t *p_m_info);
void sys_info_statis_destroy(MainStatistics *st);
int sys_info_statis_start(MainStatistics *st);
void sys_info_statis_stop(MainStatistics *st);

void white_list_set_log_switch(MainStatistics *st, int enable);
void network_set_log_switch(MainStatistics *st, int enable);
void white_list_set_config_file(MainStatistics *st, const char *file, const char *proc, const char *modu, const char *host);
void sys_info_set_statis_delta(MainStatistics *st, int sys_info_statis_delta);
void proc_action_set_statis_delta(MainStatistics *st, int proc_action_time_delta);
void snap_shot_set_statis_delta(MainStatistics *st, int snap_shot_host_time_delta, int snap_shot_proc_time_delta, int snap_shot_net_time_delta);
// void network_set_delta(MainStatistics *st, int network_time_delta);
void sys_info_statis_set_local_ip(MainStatistics *st, const char *ip);
void set_server_host_port_token(MainStatistics *p_st, const char *p_host, uint16_t port, const char *p_token);

EXTERN_C_END

#endif // __SYSTEM_INFOMATION_STATISTICS__
