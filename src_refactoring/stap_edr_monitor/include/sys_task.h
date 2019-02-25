#ifndef __EDR_SYS_TASK_H__
#define __EDR_SYS_TASK_H__

#include "cJSON.h"
#include <time.h>
#include <string>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sys_task_ {
    // task(js string)
    char *cmd;
    uint32_t cmd_size;
    char *params;
    uint32_t params_size;
} sys_task_t;

typedef struct sys_task_md5_ {
    char md5_str[64];
    time_t finish_time;
} sys_task_md5_t;

typedef struct sys_task_snap_shot_delta_ {
    int report_proc;
    int report_net;
    int report_host;
} sys_task_snap_shot_delta_t;

struct sys_task_check_proc_by_connection_t {
    uint64_t edr_pid    = 0;
    int      pid        = 0;
    uint32_t dest_ip    = 0;
    uint16_t dest_port  = 0;
    time_t   op_time    = 0;
    std::string domain;
} ;

#define SYS_TASK_ACTION_RULE_OBJECT_TYPE_IP     1
#define SYS_TASK_ACTION_RULE_OBJECT_TYPE_DOMAIN 2
#define SYS_TASK_ACTION_RULE_OBJECT_TYPE_URL    3
#define SYS_TASK_ACTION_RULE_OBJECT_TYPE_MD5    4

#define SYS_TASK_ACTION_RULE_ACTION_WARNING        1
#define SYS_TASK_ACTION_RULE_ACTION_ISOLATE_FILE   2
#define SYS_TASK_ACTION_RULE_ACTION_DELETE_FILE    3
#define SYS_TASK_ACTION_RULE_ACTION_KILL_PROCESS   4
#define SYS_TASK_ACTION_RULE_ACTION_FINISH_CONN    5

struct sys_task_action_rule_t {
    int rule_id                 = 0;
    int object_type             = 0;
    int action                  = 0;
    time_t finish_time          = 0;
    int check_delta             = 0;
    int report_delta            = 0;
    time_t last_report_time     = 0;
    time_t last_check_time      = 0;
    std::string objects;
};

#ifdef __cplusplus
}
#endif

#endif // __EDR_SYS_TASK_H__
