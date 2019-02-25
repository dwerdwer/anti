#include "cJSON.h"
#include "sys_task.h"
#include "tunnel.h"
#include "netstat.h"
#include "white_list.h"
#include "util-system.h"
#include "util-path.h"
#include "ring_buffer.h"
#include "upload_struct.h"
#include "sysinfo_statistics.h"
#include "sysinfo_logger.h"
#include "file_digest_sqlite.h"
#include "proc_monitor.h"
#include "proc_statistics.h"
#include "proc_history.h"
#include "net_monitor.h"
#include "debug_print.h"
#include "rules_processor.h"

#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <inttypes.h>
#include <map>
#include <vector>

#define CONF_EDITION            "edition"
#define CONF_WHITE_LIST_PATH    "wlist_path"
#define CONF_RULE_PATH          "rule_path"

#ifndef PRIu64
#define PRIu64        "llu"
#endif

struct utl_buf_s {
    char   *buf;
    int     siz;
    int     used;
};
static struct utl_buf_s ub = { NULL, 0, 0 };

#define FLAG_NORMAL     0
#define FLAG_LOADING    1
#define FLAG_HANGUP     2
#define FLAG_CLOSED     3

#define STRONGLY_SET_MAX_PID 100000

static char *EDR_id(time_t time_s, uint64_t pid, char *buffer, int buff_len)
{
    struct tm t;
    localtime_r(&time_s, &t);
    pid %= STRONGLY_SET_MAX_PID;
    snprintf(buffer, buff_len, "%4d%02d%02d%02d%02d%02d%05u",
            1900 + t.tm_year, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, (unsigned int)pid);
    return buffer;
}

// static uint64_t gettimeofday_ms()
// {
//     struct timeval tv;
//     gettimeofday(&tv, NULL);
//     return tv.tv_sec * 1000ULL + tv.tv_usec / 1000ULL;
// }
// 
// static bool time_to(time_t prev, time_t delta)
// {
//     return time(NULL) > prev+delta;
// }

static int conf_value_is_true(const char *value)
{
    int ret = 0;
    if (!value)
        ret = 0;
    else if (strcasestr(value, "yes"))
        ret = 1;
    else if (strcasestr(value, "enable"))
        ret = 1;
    else if (atoi(value) > 0)
        ret = 1;
    else if (strcasestr(value, "no"))
        ret = 0;
    else if (strcasestr(value, "disable"))
        ret = 0;
    else
        ret = 0;

    return ret;
}

static const char *conf_get_value(const char **p_pargs, uint32_t arg_count, const char *key)
{
    for (uint32_t i = 0; i<arg_count; i++)
    {
        if (NULL != strcasestr(p_pargs[i], key))
        {
            char *value = (char *)strchr(p_pargs[i], ':');
            if (value)
            {
                return value + 1;
            }
        }
    }
    return NULL;
}

static bool time_to(time_t curr, time_t prev, time_t delta)
{
    return curr > prev + delta;
}

static bool time_to_update_snap_shot(time_t curr, MainStatistics *p_st)
{
    return (time_to(curr, p_st->snap_shot_host_time_prev, p_st->snap_shot_host_time_delta)
            && (time_to(curr, p_st->snap_shot_proc_time_prev, p_st->snap_shot_proc_time_delta)
                || time_to(curr, p_st->snap_shot_net_time_prev, p_st->snap_shot_net_time_delta)
               )
           );
}

static bool time_to_update_proc_action(time_t curr, MainStatistics *p_st)
{
    return (time_to(curr, p_st->proc_action_time_prev, p_st->proc_action_time_delta));
}

static bool time_to_update_sysinfo(time_t curr, MainStatistics *p_st)
{
    return time_to(curr, p_st->sys_info_time_prev, p_st->sys_info_time_delta);
}

void set_server_host_port_token(MainStatistics *p_st, const char *p_host, uint16_t port, const char *p_token)
{
    if (p_st && p_st->p_upload)
    {
        p_st->p_upload->set_url(p_host, port);
        p_st->p_upload->set_url(p_token);
    }
}

void network_set_log_switch(MainStatistics *p_st, int enable)
{
    INFO_PRINT("%d\n", enable);
    p_st->enable_network_log = enable;
}

void white_list_set_log_switch(MainStatistics *p_st, int enable)
{
    INFO_PRINT("%d\n", enable);
    p_st->enable_white_list_log = enable;
}

void white_list_set_config_file(MainStatistics *p_st, const char *file, const char *proc,
        const char *modu, const char *host)
{
    p_st->white_list->set_config_file(file, proc, modu, host);
}

void sys_info_set_statis_delta(MainStatistics *p_st, int snap_shot_delta)
{
    if (p_st)
    {
        p_st->sys_info_time_delta = snap_shot_delta;
    }
}

void proc_action_set_statis_delta(MainStatistics *p_st, int proc_action_time_delta)
{
    if (p_st)
    {
        p_st->proc_action_time_delta = proc_action_time_delta;
    }
}

void snap_shot_set_statis_delta(MainStatistics *p_st, int snap_shot_host_time_delta, int snap_shot_proc_time_delta, int snap_shot_net_time_delta)
{
    if (p_st)
    {
        p_st->snap_shot_host_time_delta = snap_shot_host_time_delta;
        p_st->snap_shot_proc_time_delta = snap_shot_proc_time_delta;
        p_st->snap_shot_net_time_delta = snap_shot_net_time_delta;
    }
}

// void network_set_delta(MainStatistics *p_st, int network_time_delta)
// {
//     p_st->network_time_delta = network_time_delta;
// }

void sys_info_statis_set_local_ip(MainStatistics *p_st, const char *ip)
{
    p_st->local_ip = ip;
    INFO_PRINT("%s\n", p_st->local_ip.data());
}

void sys_task_free(void *argu)
{
    sys_task_t *task = (sys_task_t *)argu;
    if (task)
    {
        if (task->cmd)
            free(task->cmd);
        if (task->params)
            free(task->params);
        free(task);
    }
}

void file_action_free(void *argu)
{
    file_action_t *action = (file_action_t *)argu;
    if (action)
    {
        if (action->json_str)
            free(action->json_str);
        free(action);
    }
}

void usb_action_free(void *argu)
{
    usb_action_t *action = (usb_action_t *)argu;
    if (action)
    {
        free(action);
    }
}

void task_record_free(void *argu)
{
    task_record *record = (task_record *)argu;
    if (record)
    {
        delete record;
    }
}

void json_file_free(void *argu)
{
    json_file *file = (json_file *)argu;
    if (file)
    {
        delete file;
    }
}

static json_file *write_json_file(cJSON *json, const char *json_name, const char *zip_name, const char *url_type, int type)
{
    json_file *file = new json_file;
    if (file)
    {
        file->json_name = json_name;
        file->zip_name = zip_name;
        file->url_type = url_type;
        file->type = type;

        char *json_str = cJSON_Print(json);
        file->json_str = json_str;
        free(json_str);
    }
    return file;
}

static system_stat_t *sys_stat_load()
{
    system_stat_t *p_stat = (system_stat_t *)calloc(1, sizeof(system_stat_t));
    if (p_stat)
    {
        if (get_cpu_status(&p_stat->cpu_status)
                || get_logging_users(p_stat->users_login, sizeof(p_stat->users_login))
                || get_disk_status(&p_stat->disk_status)
           )
        {
            free(p_stat);
            p_stat = NULL;
        }
    }
    return p_stat;
}

static RecordCtx *conn_load()
{
    RecordCtx *connctx = RecordCtxCreate();
    if (connctx)
    {
        if (ConnRecordLoad(connctx) < 0)
        {
            RecordCtxDestroy(connctx);
            connctx = NULL;
        }
    }
    return connctx;
}

static void proc_2_md5_cache(Proc *proc, void *cache_handle)
{
    if (proc->abs_name.size())
    {
        cache_to_file_digest(cache_handle, proc->abs_name.data());
    }

    for (std::map<std::string, DynamicLibInfo>::iterator itr = proc->libs.begin();
            itr != proc->libs.end(); itr++)
    {
        cache_to_file_digest(cache_handle, itr->first.data());
    }
}

#define clear_list_easy_macro(type, l, callback)\
    do {\
    for (std::list<type *>::iterator itr = (l).begin(); itr != (l).end(); )\
    {\
        callback(*itr);\
        itr = (l).erase(itr);\
    }\
    } while (0)

static void actions_clear(MainStatistics *p_st)
{
    clear_list_easy_macro(cJSON,       p_st->file_list, cJSON_Delete);
    clear_list_easy_macro(Proc,        p_st->proc_list, proc_free);
    clear_list_easy_macro(http_info_t, p_st->http_list, http_info_free);
    clear_list_easy_macro(dns_info_t,  p_st->dns_list,  dns_info_free);
    clear_list_easy_macro(icmp_info_t, p_st->icmp_list, icmp_info_free);
    clear_list_easy_macro(tcp_info_t,  p_st->tcp_list,  tcp_info_free);
    clear_list_easy_macro(udp_info_t,  p_st->udp_list,  udp_info_free);
}

static uint64_t get_file_size(const char *name)
{
    struct stat p_st;
    if (!name || stat(name, &p_st))
        return 0;
    return p_st.st_size;
}

static time_t get_file_last_time(const char *name)
{
    struct stat p_st;
    if (!name || stat(name, &p_st))
        return 0;
    return p_st.st_mtime>p_st.st_ctime ? p_st.st_mtime : p_st.st_ctime;
}

static int pre_alloc(struct utl_buf_s *ub, int size)
{
    if (ub->siz < size)
    {
        ub->buf = (char *)realloc(ub->buf, size);
        if (!ub->buf)    return -1;
        ub->siz = size;
    }
    return 0;
}

static cJSON *snap_shot_proc_info_2_JSON(MainStatistics *p_st, ps_t *p_ps)
{
    Proc *proc;
    cJSON *obj;
    if (!p_st || !p_ps)
        return NULL;
    cJSON *proc_js_array = cJSON_CreateArray();
    if (!proc_js_array) return NULL;
    char buffer[64];
    for (size_t i = 0, size = p_ps->size; i<size; i++)
    {
        proc = p_ps->procs[i];
        obj = cJSON_CreateObject();

        // pid ->str
        EDR_id(get_sys_boot_timestamp() + proc->start_time / get_clock_ticks(), proc->pid, buffer, sizeof(buffer));
        cJSON_AddStringToObject(obj, "pid", buffer);
        // ppid ->str
        buffer[0] = 0;
        EDR_id(p_st->proc_his.get_start_time(proc->ppid), proc->ppid, buffer, sizeof(buffer));
        cJSON_AddStringToObject(obj, "parent_id", buffer);
        // proc name
        if (proc->name.size())
            cJSON_AddStringToObject(obj, "proc_name", proc->name.data());
        else
            cJSON_AddStringToObject(obj, "proc_name", proc->cmd);
        // proc path
        cJSON_AddStringToObject(obj, "proc_path", proc->abs_name.data());
        // md5 str
        strcpy(buffer, "");
        if (proc->abs_name.size() && '/' == proc->abs_name[0])
        {
            file_digest_t *ft = get_file_digest_by_path(p_st->md5_cache, proc->abs_name.data());
            if (ft)
            {
                strcpy(buffer, ft->md5);
                free(ft);
            }
        }
        cJSON_AddStringToObject(obj, "md5", buffer);
        // cmd line
        cJSON_AddStringToObject(obj, "cmd", proc->cmdline.data());
        // flag
        // cJSON_AddNumberToObject(obj, "flag", get_flag_by_process_state(proc->state));
        cJSON_AddNumberToObject(obj, "flag", FLAG_NORMAL);
        // owner
        char *p_user_name = get_user_name_by_uid(p_st->p_users_passwd, proc->ruid);
        cJSON_AddStringToObject(obj, "owner", p_user_name ? p_user_name : "");
        // cpu  = 1000.0 * time(s) / delta(s)
        uint64_t time_used = proc->utime + proc->stime;
        uint64_t time_delta_ms = 1000;
        Proc *proc_prev = get_proc_by_PID(p_st->ps_prev, proc->pid);
        if (proc_prev)
        {
            time_used -= (proc_prev->utime + proc_prev->stime);
            time_delta_ms = proc->current_time - proc_prev->current_time;
        }
        int time_used_ms = (1000.0*time_used / get_clock_ticks()) / (time_delta_ms / 1000.0);
        cJSON_AddNumberToObject(obj, "cpu", time_used_ms * 100 /*percent*/);
        // mem
        cJSON_AddNumberToObject(obj, "mem", proc->vm_rss * 1024);
        // terminal
        cJSON_AddNumberToObject(obj, "terminal", proc->tty);

        cJSON_AddItemToArray(proc_js_array, obj);
    }

    return proc_js_array;
}

static cJSON *snap_shot_conn_info_2_JSON(MainStatistics *p_st, RecordCtx *connctx, ps_t *p_ps)
{
    if (!p_st || !connctx || !p_ps)
        return NULL;
    cJSON *net_js_array = cJSON_CreateArray();
    if (!net_js_array) return NULL;
    char buffer[64];
    for (size_t i = 0, used = connctx->conns_head.used; i<used; i++)
    {
        Connect *conn = connctx->conns_head.conns[i];
        if (PROTO_TCP == conn->type || PROTO_TCP6 == conn->type)
        {
            if (TCP_CLOSING == conn->state
                    || TCP_CLOSE == conn->state
                    || TCP_CLOSE_WAIT == conn->state
                    || TCP_TIME_WAIT == conn->state
                    || TCP_FIN_WAIT1 == conn->state
                    || TCP_FIN_WAIT2 == conn->state
               )
                continue;
        }
        cJSON *obj = cJSON_CreateObject();
        if (!obj)    continue;

        Proc *proc = get_proc_by_sock_ino(p_ps, conn->ino);
        // pid ->str
        buffer[0] = 0;
        if (proc)
        {
            EDR_id(get_sys_boot_timestamp() + proc->start_time / get_clock_ticks(), proc->pid, buffer, sizeof(buffer));
        }
        cJSON_AddStringToObject(obj, "pid", buffer);
        // ppid ->str
        buffer[0] = 0;
        if (proc)
        {
            EDR_id(p_st->proc_his.get_start_time(proc->ppid), proc->ppid, buffer, sizeof(buffer));
        }
        cJSON_AddStringToObject(obj, "parent_id", buffer);
        // proc name
        if (proc)
        {
            if (proc->name.size())
                cJSON_AddStringToObject(obj, "proc_name", proc->name.data());
            else
                cJSON_AddStringToObject(obj, "proc_name", proc->cmd);
        }
        else
        {
            cJSON_AddStringToObject(obj, "proc_name", "");
        }
        // proc path
        if (proc)
        {
            cJSON_AddStringToObject(obj, "proc_path", proc->abs_name.data());
        }
        else
        {
            cJSON_AddStringToObject(obj, "proc_path", "");
        }
        // md5 str
        buffer[0] = 0;
        if (proc)
        {
            if (proc->abs_name.size() && '/' == proc->abs_name[0])
            {
                file_digest_t *ft = get_file_digest_by_path(p_st->md5_cache, proc->abs_name.data());
                if (ft)
                {
                    strcpy(buffer, ft->md5);
                    free(ft);
                }
            }
        }
        cJSON_AddStringToObject(obj, "md5", buffer);
        // cmd line
        if (proc)
        {
            cJSON_AddStringToObject(obj, "cmd", proc->cmdline.data());
        }
        else
        {
            cJSON_AddStringToObject(obj, "cmd", "");
        }
        // src ip
        inet_ntop(conn->af, &conn->local.addr_data32[0], buffer, sizeof(buffer));
        cJSON_AddStringToObject(obj, "src_ip", buffer);
        // src port
        cJSON_AddNumberToObject(obj, "src_port", conn->lport);
        // dest ip
        inet_ntop(conn->af, &conn->foreign.addr_data32[0], buffer, sizeof(buffer));
        cJSON_AddStringToObject(obj, "dest_ip", buffer);
        // dest port
        cJSON_AddNumberToObject(obj, "dest_port", conn->fport);
        // protocol
        cJSON_AddStringToObject(obj, "protocol", proto2str(conn->type));
        // recv bytes
        cJSON_AddNumberToObject(obj, "recv_bytes", 0);
        // send bytes
        cJSON_AddNumberToObject(obj, "send_bytes", 0);

        cJSON_AddItemToArray(net_js_array, obj);
    }
    return net_js_array;
}

static cJSON *snap_shot_node_info_2_JSON(system_stat_t *old_sys, system_stat_t *new_sys)
{
    cJSON *sys_js = cJSON_CreateObject();
    if (!sys_js)
        return NULL;
    if (!old_sys || !new_sys)
        return sys_js;

    time_t now = time(NULL);
    cJSON_AddStringToObject(sys_js, "nodeId", "-1");
    cJSON_AddNumberToObject(sys_js, "time", now);
    cJSON_AddNumberToObject(sys_js, "cpu", calc_cpu_rate(&old_sys->cpu_status, &new_sys->cpu_status));
    cJSON_AddNumberToObject(sys_js, "disk", new_sys->disk_status.used);
    cJSON_AddNumberToObject(sys_js, "net", 0);
    cJSON_AddStringToObject(sys_js, "user", new_sys->users_login);
    return sys_js;
}

// -------------------for interface ----------------------
int sys_task_push(MainStatistics *p_st, sys_task_t *task)
{
    if (!p_st || !p_st->sys_tasks)
        return -1;
    return ring_buffer_push(p_st->sys_tasks, task);
}
int file_action_push(MainStatistics *p_st, file_action_t *action)
{
    if (!p_st || !p_st->file_actions)
        return -1;
    return ring_buffer_push(p_st->file_actions, action);
}
int usb_action_push(MainStatistics *p_st, usb_action_t *action)
{
    if (!p_st || !p_st->usb_actions)
        return -1;
    return ring_buffer_push(p_st->usb_actions, action);
}
static sys_task_t *sys_task_pop(ring_buffer_t *rb)
{
    return (sys_task_t *)ring_buffer_pop(rb);
}
static file_action_t *file_action_pop(ring_buffer_t *rb)
{
    return (file_action_t *)ring_buffer_pop(rb);
}
static usb_action_t *usb_action_pop(ring_buffer_t *rb)
{
    return (usb_action_t *)ring_buffer_pop(rb);
}

static int sysinfo_init(MainStatistics *p_st)
{
    p_st->conn_curr = NULL;
    p_st->sys_stat_curr = NULL;

    p_st->conn_prev = conn_load();
    if (NULL == p_st->conn_prev)
    {
        WARN_PRINT("conn_load failed\n");
        return -1;
    }
    p_st->sys_stat_prev = sys_stat_load();
    if (NULL == p_st->sys_stat_prev)
    {
        WARN_PRINT("sys_stat_load failed\n");
        return -1;
    }

    // update time_s
    time_t time_s = time(NULL);
    p_st->snap_shot_host_time_prev = p_st->snap_shot_proc_time_prev = p_st->snap_shot_net_time_prev = time_s;
    p_st->sys_info_time_prev = time_s;
    // p_st->network_time_prev = gettimeofday_ms();
    return 0;
}

static void load_tunnel(MainStatistics *p_st)
{
    sys_task_t *task;
    while (NULL != (task = tunnel_output()))
    {
        if (sys_task_push(p_st, task))
        {
            sysinfo_log(p_st->p_m_info, "%s: sys_task_push failed cmd[%s] params[%s]\n",
                    __func__, task->cmd, task->params);
            sys_task_free(task);
        }
    }
}

#define UPDATE_ERROR    1
#define UPDATE_AGAIN    2
#define UPDATE_SUCCESS  3

static int ps_update(MainStatistics *p_st)
{
    /* proc action */
    while (true)
    {
        Proc *proc = (Proc *)ring_buffer_pop(p_st->proc_actions);
        if (!proc)
            break;
        switch(proc->comparing_state)
        {
        case PROC_ACTION_CREATE:
            p_st->proc_his.on_proc_create(proc);
            proc_2_md5_cache(proc, p_st->md5_cache);
            break;
        case PROC_ACTION_DESTROY:
            p_st->proc_his.on_proc_destroy(proc->pid, proc->stoptime);
            break;
        default:
            fprintf(stderr, "%s an error state: %d\n", __func__, proc->comparing_state);
        }
        p_st->proc_list.push_back(proc);
    }

    /* snap shot */
    ps_t *p_ps = (ps_t *)ring_buffer_pop(p_st->snap_shots);
    if (!p_ps)
    {
        return UPDATE_AGAIN;
    }

#ifdef STATIS_DEBUG
    Proc **procs = p_ps->procs;
    for (size_t i = 0, size = p_ps->size; i<size; i++)
    {
        debug_print("snapshot   %10" PRIu64 " %s\n", procs[i]->pid, procs[i]->name.size() ? procs[i]->name.data() : procs[i]->cmd);
    }
#endif

    if (p_st->ps_prev)
    {
        ps_destroy(p_st->ps_prev);
        p_st->ps_prev = NULL;
    }
    if (p_st->ps_curr)
    {
        p_st->ps_prev = p_st->ps_curr;
        p_st->ps_curr = NULL;
    }
    p_st->ps_curr = p_ps;

    return UPDATE_SUCCESS;
}

static int packet_match_sys_task_rule(MainStatistics *p_st, http_info_t *p_info);


static int conn_update(MainStatistics *p_st)
{
    if (p_st->conn_curr)
    {
        RecordCtxDestroy(p_st->conn_prev);
        p_st->conn_prev = p_st->conn_curr;
        p_st->conn_curr = NULL;
    }
    p_st->conn_curr = conn_load();
    if (NULL == p_st->conn_curr)
    {
        WARN_PRINT("conn_load failed\n");
        return -1;
    }

#define conn_update_load_check_insert_macro(type, rb, l)     \
    do                                              \
    {                                               \
        type *p_info = (type *)ring_buffer_pop(rb); \
        if (!p_info)                                \
            break;                                  \
        packet_match_sys_task_rule(p_st, p_info);   \
        p_st->net_his.insert(p_info);               \
        l.push_back(p_info);                        \
    } while (0)

#define conn_update_load_insert_macro(type, rb, l)  \
    do                                              \
    {                                               \
        type *p_info = (type *)ring_buffer_pop(rb); \
        if (!p_info)                                \
            break;                                  \
        p_st->net_his.insert(p_info);               \
        l.push_back(p_info);                        \
    } while (0)

#define conn_update_load_macro(type, rb, l)\
    do                                              \
    {                                               \
        type *p_info = (type *)ring_buffer_pop(rb); \
        if (!p_info)                                \
            break;                                  \
        l.push_back(p_info);                        \
    } while (0)

    // packet
    conn_update_load_check_insert_macro(http_info_t, p_st->httpes, p_st->http_list);
    conn_update_load_insert_macro(dns_info_t,  p_st->dnses,  p_st->dns_list);
    conn_update_load_macro(icmp_info_t, p_st->icmpes, p_st->icmp_list);
    conn_update_load_macro(udp_info_t,  p_st->udpes,  p_st->udp_list);
    conn_update_load_macro(tcp_info_t,  p_st->tcpes,  p_st->tcp_list);

    return 0;
}

static int sys_stat_update(MainStatistics *p_st)
{
    if (p_st->sys_stat_curr)
    {
        free(p_st->sys_stat_prev);
        p_st->sys_stat_prev = p_st->sys_stat_curr;
        p_st->sys_stat_curr = NULL;
    }
    p_st->sys_stat_curr = sys_stat_load();
    if (NULL == p_st->sys_stat_curr)
    {
        WARN_PRINT("sys_stat_load failed\n");
        return -1;
    }
    return 0;
}

static int sysinfo_update(MainStatistics *p_st)
{
    if (conn_update(p_st) < 0)
        return -1;
    if (sys_stat_update(p_st) < 0)
        return -1;
    net_compare(&p_st->conn_prev->conns_head, &p_st->conn_curr->conns_head);

    // if (p_st->conn_curr && p_st->ps_curr)
    //     conn_relate_pid(p_st->conn_curr, p_st->ps_curr);
    // conn_info_update(p_st, p_st->conn_curr);
    // conn_action_update(p_st);

    p_st->sys_info_time_prev = time(NULL);
    return 0;
}

static cJSON *snap_shot_2_json(MainStatistics *p_st)
{
    // write to cjson
    cJSON *root = cJSON_CreateObject();

    if (!root) return NULL;

    time_t time_s = time(NULL);
    if (time_to(time_s, p_st->snap_shot_host_time_prev, p_st->snap_shot_host_time_delta))
    {
        p_st->snap_shot_host_time_prev = time_s;
        cJSON *sys_js = snap_shot_node_info_2_JSON(p_st->sys_stat_prev, p_st->sys_stat_curr);
        cJSON_AddItemToObject(root, "Node", sys_js);
    }
    if (time_to(time_s, p_st->snap_shot_proc_time_prev, p_st->snap_shot_proc_time_delta))
    {
        p_st->snap_shot_proc_time_prev = time_s;
        cJSON *proc_js_array = snap_shot_proc_info_2_JSON(p_st, p_st->ps_curr);
        cJSON_AddItemToObject(root, "Proc_info", proc_js_array);
    }
    if (time_to(time_s, p_st->snap_shot_net_time_prev, p_st->snap_shot_net_time_delta))
    {
        p_st->snap_shot_net_time_prev = time_s;
        cJSON *net_js_array = snap_shot_conn_info_2_JSON(p_st, p_st->conn_curr, p_st->ps_curr);
        cJSON_AddItemToObject(root, "Net_info", net_js_array);
    }

    return root;
}

enum {
    EDR_PROC_ACTION_UNKNOWN = 0,
    EDR_PROC_ACTION_FORK = 1,
    EDR_PROC_ACTION_DESTROY = 2,
    EDR_PROC_ACTION_INJECT = 3,
    EDR_PROC_ACTION_RPC = 4,
    EDR_PROC_ACTION_OPEN = 5
};

int proc_state_2_edr_state(int state)
{
    switch (state)
    {
        case PROC_ACTION_CREATE:
            state = EDR_PROC_ACTION_FORK;
            break;
        case PROC_ACTION_DESTROY:
            state = EDR_PROC_ACTION_DESTROY;
            break;
        default:
            state = EDR_PROC_ACTION_UNKNOWN;
    }
    return state;
}

static cJSON *proc_actions_proc_action_2_json(MainStatistics *p_st)
{
    cJSON *js_array = cJSON_CreateArray();
    if (!js_array)   return NULL;
    char buffer[64];
    for (std::list<Proc *>::iterator itr = p_st->proc_list.begin(); itr != p_st->proc_list.end(); itr++)
    {
        Proc *proc = *itr;

        if (!proc)
            continue;

        if (p_st->p_processor && p_st->p_processor->get_permission(p_st->p_rules, proc->pid, EPROC_ACTION) == false)
            continue;
        debug_print("%s %d get_permission ok\n", __func__, proc->pid);

        if (p_st->white_list->is_proc_in_white_list(proc->abs_name.data()))
        {
            if (p_st->enable_white_list_log)
                sysinfo_log(p_st->p_m_info, "[%s] whitelist [%s] touch\n", __func__, proc->abs_name.data());
            continue;
        }

        cJSON *obj = cJSON_CreateObject();
        if (!obj) continue;

        // ppid destroyed pid, so write ppid at first, write pid at dest_pid

        // ppid ->str
        buffer[0] = 0;
        EDR_id(p_st->proc_his.get_start_time(proc->ppid), proc->ppid, buffer, sizeof(buffer));
        cJSON_AddStringToObject(obj, "pid", buffer);
        // action
        cJSON_AddNumberToObject(obj, "action", proc_state_2_edr_state(proc->comparing_state));
        // result
        cJSON_AddNumberToObject(obj, "result", 0);
        // pid ->str
        EDR_id(get_sys_boot_timestamp() + proc->start_time / get_clock_ticks(), proc->pid, buffer, sizeof(buffer));
        cJSON_AddStringToObject(obj, "dest_pid", buffer);
        // op time
        cJSON_AddNumberToObject(obj, "op_time", time(NULL));

        cJSON_AddItemToArray(js_array, obj);
        // debug_print("%10d--%10s-->%10lu    %s\n", proc->ppid, (proc->comparing_state==PROC_STATE_START)?"fork":"destroy", proc->pid, proc->name.size() ? proc->name.data() : proc->cmd);
    }
    return js_array;
}

static cJSON *proc_actions_proc_base_info_2_json(MainStatistics *p_st)
{
    cJSON *js_array = cJSON_CreateArray();
    if (!js_array)   return NULL;
    char buffer[64];
    for (std::list<Proc *>::iterator itr = p_st->proc_list.begin(); itr != p_st->proc_list.end(); itr++)
    {
        Proc *proc = *itr;
        if (!proc)
            continue;

        if (p_st->p_processor && p_st->p_processor->get_permission(p_st->p_rules, proc->pid, EPROC_INFO) == false)
            continue;

        debug_print("%s %d get_permission ok\n", __func__, proc->pid);
        if (p_st->white_list->is_proc_in_white_list(proc->abs_name.data()))
        {
            if (p_st->enable_white_list_log)
                sysinfo_log(p_st->p_m_info, "[%s] whitelist [%s] touch\n", __func__, proc->abs_name.data());
            continue;
        }

        cJSON *obj = cJSON_CreateObject();
        if (!obj)    continue;

        // pid ->str
        EDR_id(get_sys_boot_timestamp() + proc->start_time / get_clock_ticks(), proc->pid, buffer, sizeof(buffer));
        cJSON_AddStringToObject(obj, "Pid", buffer);
        // ppid ->str
        buffer[0] = 0;
        EDR_id(p_st->proc_his.get_start_time(proc->ppid), proc->ppid, buffer, sizeof(buffer));
        cJSON_AddStringToObject(obj, "parent_id", buffer);
        // proc name
        if (proc->name.size())
            cJSON_AddStringToObject(obj, "proc_name", proc->name.data());
        else
            cJSON_AddStringToObject(obj, "proc_name", proc->cmd);
        // proc path
        cJSON_AddStringToObject(obj, "proc_path", proc->abs_name.data());
        // cmd
        cJSON_AddStringToObject(obj, "cmd", proc->cmdline.data());
        // flag
        // cJSON_AddNumberToObject(obj, "flag", get_flag_by_process_state(proc->state));
        cJSON_AddNumberToObject(obj, "flag", (PROC_STATE_CLOSED == proc->comparing_state) ? FLAG_CLOSED : FLAG_NORMAL);
        // user
        char *p_user_name = get_user_name_by_uid(p_st->p_users_passwd, proc->ruid);
        cJSON_AddStringToObject(obj, "user", p_user_name ? p_user_name : "");
        // created at
        cJSON_AddNumberToObject(obj, "created_at", get_sys_boot_timestamp() + proc->start_time / get_clock_ticks());
        // stop at
        cJSON_AddNumberToObject(obj, "stop_at", proc->stoptime);

        cJSON_AddItemToArray(js_array, obj);
    }
    if (!p_st->ps_curr)
        return js_array;
    if (p_st->edition == edr::edition_t::EDITION_HUAWEI)
        return js_array;
    for (size_t i = 0, size = p_st->ps_curr->size; i<size; i++)
    {
        Proc *proc = p_st->ps_curr->procs[i];
        if (!proc)
            continue;

        if (p_st->white_list->is_proc_in_white_list(proc->abs_name.data()))
        {
            if (p_st->enable_white_list_log)
                sysinfo_log(p_st->p_m_info, "[%s] whitelist [%s] touch\n", __func__, proc->abs_name.data());
            continue;
        }

        cJSON *obj = cJSON_CreateObject();
        if (!obj)    continue;

        // pid ->str
        EDR_id(get_sys_boot_timestamp() + proc->start_time / get_clock_ticks(), proc->pid, buffer, sizeof(buffer));
        cJSON_AddStringToObject(obj, "Pid", buffer);
        // ppid ->str
        buffer[0] = 0;
        EDR_id(p_st->proc_his.get_start_time(proc->ppid), proc->ppid, buffer, sizeof(buffer));
        cJSON_AddStringToObject(obj, "parent_id", buffer);
        // proc name
        if (proc->name.size())
            cJSON_AddStringToObject(obj, "proc_name", proc->name.data());
        else
            cJSON_AddStringToObject(obj, "proc_name", proc->cmd);
        // proc path
        cJSON_AddStringToObject(obj, "proc_path", proc->abs_name.data());
        // cmd
        cJSON_AddStringToObject(obj, "cmd", proc->cmdline.data());
        // flag
        // cJSON_AddNumberToObject(obj, "flag", get_flag_by_process_state(proc->state));
        cJSON_AddNumberToObject(obj, "flag", (PROC_STATE_CLOSED == proc->comparing_state) ? FLAG_CLOSED : FLAG_NORMAL);
        // user
        char *p_user_name = get_user_name_by_uid(p_st->p_users_passwd, proc->ruid);
        cJSON_AddStringToObject(obj, "user", p_user_name ? p_user_name : "");
        // created at
        cJSON_AddNumberToObject(obj, "created_at", get_sys_boot_timestamp() + proc->start_time / get_clock_ticks());
        // stop at
        cJSON_AddNumberToObject(obj, "stop_at", proc->stoptime);

        cJSON_AddItemToArray(js_array, obj);
    }
    return js_array;
}

static cJSON *proc_actions_proc_module_info_2_json(MainStatistics *p_st)
{
    cJSON *js_array = cJSON_CreateArray();
    if (!js_array)    return NULL;
    char buffer[64];
    struct stat file_stat;
    std::vector<std::string> writed_pids;
    for (std::list<Proc *>::iterator itr = p_st->proc_list.begin(); itr != p_st->proc_list.end(); itr++)
    {
        Proc *proc = *itr;

        if (!proc)
            continue;

        if (p_st->p_processor && p_st->p_processor->get_permission(p_st->p_rules, proc->pid, EPROC_MOD) == false)
            continue;

        debug_print("%s %d get_permission ok\n", __func__, proc->pid);
        if (p_st->white_list->is_proc_in_white_list(proc->abs_name.data()))
        {
            if (p_st->enable_white_list_log)
                sysinfo_log(p_st->p_m_info, "[%s] whitelist [%s] touch\n", __func__, proc->abs_name.data());
            continue;
        }

        buffer[0] = 0;
        EDR_id(get_sys_boot_timestamp() + proc->start_time / get_clock_ticks(), proc->pid, buffer, sizeof(buffer));

        for (std::vector<std::string>::iterator itr = writed_pids.begin();
                itr != writed_pids.end(); itr++)
        {
            // this PID module info is already write to json
            if (0 == strcmp(itr->data(), buffer))
                continue;
        }

        writed_pids.push_back(buffer);

        for (std::map<std::string, DynamicLibInfo>::iterator itr = proc->libs.begin();
                itr != proc->libs.end(); itr++)
        {

            DynamicLibInfo &info = itr->second;

            if (p_st->white_list->is_modu_in_white_list(info.abs_name.data()))
            {
                if (p_st->enable_white_list_log)
                    sysinfo_log(p_st->p_m_info, "[%s] whitelist [%s] touch\n", __func__, info.abs_name.data());
                continue;
            }

            if (stat(info.abs_name.data(), &file_stat))
                continue;

            cJSON *obj = cJSON_CreateObject();
            if (!obj)    continue;

            // pid->str
            EDR_id(get_sys_boot_timestamp() + proc->start_time / get_clock_ticks(), proc->pid, buffer, sizeof(buffer));
            cJSON_AddStringToObject(obj, "pid", buffer);
            // md5str
            buffer[0] = 0;
            if (info.abs_name.size() && '/' == info.abs_name[0])
            {
                file_digest_t *ft = get_file_digest_by_path(p_st->md5_cache, info.abs_name.data());
                if (ft)
                {
                    strcpy(buffer, ft->md5);
                    free(ft);
                }
            }
            cJSON_AddStringToObject(obj, "md5", buffer);
            // name
            cJSON_AddStringToObject(obj, "file_name", info.abs_name.data());
            // file size
            cJSON_AddNumberToObject(obj, "file_size", get_file_size(info.abs_name.data()));
            // file time
            cJSON_AddNumberToObject(obj, "file_time", get_file_last_time(info.abs_name.data()));
            // flag
            cJSON_AddNumberToObject(obj, "flag", 0);
            // op time
            cJSON_AddNumberToObject(obj, "op_time", get_sys_boot_timestamp() + proc->start_time / get_clock_ticks());
            cJSON_AddItemToArray(js_array, obj);
        }
    }
    return js_array;
}

#if 0
{
    Json::CharReaderBuilder readerbuilder;
    Json::CharReader *reader = readerbuilder.newCharReader();
    JSONCPP_STRING errs;

    Json::Value root;
    if (false == reader->parse(task->params, task->params + strlen(task->params), &root, &errs))
    {
        debug_print(" parse [%s] [%s] failed\n", task->cmd, task->params);
        return -1;
    }
    delete reader;
}
#endif

static cJSON *file_action_fill_info(MainStatistics *p_st, cJSON *src)
{
    if (!src)
        return NULL;
    cJSON *dst = cJSON_CreateObject();
    if (!dst)
        return NULL;
    char buffer[64];
    do {
        cJSON *item = cJSON_GetObjectItem(src, "file_path");
        if (item == NULL || item->valuestring == NULL)
        {
            break;
        }
        // pid
        item = cJSON_GetObjectItem(src, "pid");
        if (item == NULL || item->valuestring == NULL)
        {
            break;
        }

        buffer[0] = 0;
        time_t start_time = p_st->proc_his.get_start_time(atoi(item->valuestring));
        EDR_id(start_time, atoi(item->valuestring), buffer, sizeof(buffer));
        cJSON_AddStringToObject(dst, "pid", buffer);

        // md5
        file_digest_t *md5_info = get_file_digest_by_path(p_st->md5_cache, item->valuestring);
        if (md5_info)
        {
            cJSON_AddStringToObject(dst, "md5", md5_info->md5);
            free(md5_info);
        }
        else
        {
            cache_to_file_digest(p_st->md5_cache, item->valuestring);
            md5_info = get_file_digest_by_path(p_st->md5_cache, item->valuestring);
            if (md5_info)
            {
                cJSON_AddStringToObject(dst, "md5", md5_info->md5);
                free(md5_info);
            }
            else
            {
                cJSON_AddStringToObject(dst, "md5", "");
            }
        }
        // file_path
        item = cJSON_GetObjectItem(src, "file_path");
        if (item == NULL || item->valuestring == NULL)
        {
            break;
        }
        cJSON_AddStringToObject(dst, "file_path", item->valuestring);
        // file size
        item = cJSON_GetObjectItem(src, "file_size");
        if (item == NULL)
        {
            break;
        }
        cJSON_AddNumberToObject(dst, "file_size", item->valuedouble);
        // create time
        cJSON_AddNumberToObject(dst, "created_at", 0);
        // update time
        item = cJSON_GetObjectItem(src, "updated_at");
        if (item == NULL)
        {
            break;
        }
        cJSON_AddNumberToObject(dst, "updated_at", item->valuedouble);
        // action 
        item = cJSON_GetObjectItem(src, "action");
        if (item == NULL)
        {
            break;
        }
        cJSON_AddNumberToObject(dst, "action", item->valuedouble);
        // result
        item = cJSON_GetObjectItem(src, "result");
        if (item == NULL)
        {
            break;
        }
        cJSON_AddNumberToObject(dst, "result", item->valuedouble);
        // op time
        item = cJSON_GetObjectItem(src, "op_time");
        if (item == NULL)
        {
            break;
        }
        cJSON_AddNumberToObject(dst, "op_time", item->valuedouble);

        cJSON_Delete(src);
        return dst;

    } while (0);

    cJSON_Delete(src);
    cJSON_Delete(dst);
    return NULL;
}

static cJSON *proc_actions_file_action_2_json(MainStatistics *p_st)
{
    cJSON *js_array = cJSON_CreateArray();
    if (!js_array) return NULL;
    for (std::list<cJSON *>::iterator itr = p_st->file_list.begin();
            itr != p_st->file_list.end();)
    {
        cJSON *file_js = *itr;
        if (!file_js)
            continue;

        cJSON *item = cJSON_GetObjectItem(file_js, "file_path");
        if (item && item->valuestring)
        {
            if (p_st->white_list->is_file_in_white_list(item->valuestring))
            {
                if (p_st->enable_white_list_log)
                    sysinfo_log(p_st->p_m_info, "[%s] whitelist [%s] touch\n", __func__, item->valuestring);
                // this file is in white list, ignore this action
                continue;
            }
        }

        item = cJSON_GetObjectItem(file_js, "pid");
        if (item && item->valuestring)
        {
            if (p_st->p_processor && p_st->p_processor->get_permission(p_st->p_rules, atoi(item->valuestring), EPROC_FILE) == false)
                continue;
            debug_print("%s %d get_permission ok\n", __func__, atoi(item->valuestring));
        }
        else
        {
            continue;
        }

        cJSON_AddItemToArray(js_array, file_js);
#ifdef DEBUG
            char *debug_str = cJSON_Print(file_js);
            sysinfo_log(p_st->p_m_info, "%s %s", __func__, debug_str);
            free(debug_str);
#endif /* DEBUG */

        itr = p_st->file_list.erase(itr);
    }
    return js_array;
}

static cJSON *proc_actions_net_2_json_huawei(MainStatistics *p_st)
{
    cJSON *obj;
    cJSON *js_array = cJSON_CreateArray();
    if (!js_array) return NULL;
    char buffer[64];

    for (std::list<tcp_info_t *>::iterator itr = p_st->tcp_list.begin(); itr != p_st->tcp_list.end(); itr++)
    {
        tcp_info_t *p_info = *itr;

        if (p_st->p_processor && p_st->p_processor->get_permission(p_st->p_rules, p_info->pid, EPROC_NET) == false)
            continue;
        debug_print("%s %d get_permission ok\n", __func__, p_info->pid);

        buffer[0] = 0;
        inet_ntop(AF_INET, &p_info->ip_hdr.source_address, buffer, sizeof(buffer));

        obj = cJSON_CreateObject();
        if (!obj) continue;

        // pid ->str
        buffer[0] = 0;
        EDR_id(p_st->proc_his.get_start_time(p_info->pid), p_info->pid, buffer, sizeof(buffer));
        cJSON_AddStringToObject(obj, "pid", buffer);
        // src ip
        buffer[0] = 0;
        inet_ntop(AF_INET, &p_info->ip_hdr.source_address, buffer, sizeof(buffer));
        cJSON_AddStringToObject(obj, "src_ip", buffer);
        // src port
        cJSON_AddNumberToObject(obj, "src_port", ntohs(p_info->tcp_hdr.source_port));
        // dst ip
        buffer[0] = 0;
        inet_ntop(AF_INET, &p_info->ip_hdr.destination_address, buffer, sizeof(buffer));
        cJSON_AddStringToObject(obj, "dest_ip", buffer);
        // dst port
        cJSON_AddNumberToObject(obj, "dest_port", ntohs(p_info->tcp_hdr.destination_port));
        // protocol
        cJSON_AddStringToObject(obj, "protocol", "TCP");
        // url
        cJSON_AddStringToObject(obj, "url", "");
        // result
        cJSON_AddNumberToObject(obj, "result", 0);
        // op time
        cJSON_AddNumberToObject(obj, "op_time", p_info->time);

        cJSON_AddItemToArray(js_array, obj);
    }


    for (std::list<udp_info_t *>::iterator itr = p_st->udp_list.begin(); itr != p_st->udp_list.end(); itr++)
    {
        udp_info_t *p_info = *itr;

        if (p_st->p_processor && p_st->p_processor->get_permission(p_st->p_rules, p_info->pid, EPROC_NET) == false)
            continue;
        debug_print("%s %d get_permission ok\n", __func__, p_info->pid);

        obj = cJSON_CreateObject();
        if (!obj) continue;

        // pid ->str
        buffer[0] = 0;
        EDR_id(p_st->proc_his.get_start_time(p_info->pid), p_info->pid, buffer, sizeof(buffer));
        cJSON_AddStringToObject(obj, "pid", buffer);
        // src ip
        buffer[0] = 0;
        inet_ntop(AF_INET, &p_info->ip_hdr.source_address, buffer, sizeof(buffer));
        cJSON_AddStringToObject(obj, "src_ip", buffer);
        // src port
        cJSON_AddNumberToObject(obj, "src_port", ntohs(p_info->udp_hdr.source_port));
        // dst ip
        buffer[0] = 0;
        inet_ntop(AF_INET, &p_info->ip_hdr.destination_address, buffer, sizeof(buffer));
        cJSON_AddStringToObject(obj, "dest_ip", buffer);
        // dst port
        cJSON_AddNumberToObject(obj, "dest_port", ntohs(p_info->udp_hdr.destination_port));
        // protocol
        cJSON_AddStringToObject(obj, "protocol", "UDP");
        // url
        cJSON_AddStringToObject(obj, "url", "");
        // result
        cJSON_AddNumberToObject(obj, "result", 0);
        // op time
        cJSON_AddNumberToObject(obj, "op_time", p_info->time);

        cJSON_AddItemToArray(js_array, obj);
    }


    for (std::list<icmp_info_t *>::iterator itr = p_st->icmp_list.begin(); itr != p_st->icmp_list.end(); itr++)
    {
        icmp_info_t *p_info = *itr;

        if (p_st->p_processor && p_st->p_processor->get_permission(p_st->p_rules, p_info->pid, EPROC_NET) == false)
            continue;
        debug_print("%s %d get_permission ok\n", __func__, p_info->pid);

        obj = cJSON_CreateObject();
        if (!obj) continue;

        // pid ->str
        buffer[0] = 0;
        EDR_id(p_st->proc_his.get_start_time(p_info->pid), p_info->pid, buffer, sizeof(buffer));
        cJSON_AddStringToObject(obj, "pid", buffer);
        // src ip
        buffer[0] = 0;
        inet_ntop(AF_INET, &p_info->ip_hdr.source_address, buffer, sizeof(buffer));
        cJSON_AddStringToObject(obj, "src_ip", buffer);
        // src port
        cJSON_AddNumberToObject(obj, "src_port", 0);
        // dst ip
        buffer[0] = 0;
        inet_ntop(AF_INET, &p_info->ip_hdr.destination_address, buffer, sizeof(buffer));
        cJSON_AddStringToObject(obj, "dest_ip", buffer);
        // dst port
        cJSON_AddNumberToObject(obj, "dest_port", 0);
        // protocol
        cJSON_AddStringToObject(obj, "protocol", "ICMP");
        // url
        cJSON_AddStringToObject(obj, "url", "");
        // result
        cJSON_AddNumberToObject(obj, "result", 0);
        // op time
        cJSON_AddNumberToObject(obj, "op_time", p_info->time);

        cJSON_AddItemToArray(js_array, obj);
    }

    return js_array;
}


static cJSON *proc_actions_http_2_json_edr(MainStatistics *p_st)
{
    cJSON *obj;
    cJSON *js_array = cJSON_CreateArray();
    if (!js_array) return NULL;
    char buffer[64];

    for (size_t i = 0, total = p_st->conn_curr->conns_head.used; i<total; i++)
    {
        Connect *conn = p_st->conn_curr->conns_head.conns[i];
        if (!conn)   continue;

        if (!conn->ino)
            continue;
        if (53 == conn->fport || 53 == conn->lport)
            continue;

        buffer[0] = 0;
        inet_ntop(conn->af, &conn->foreign.addr_data32[0], buffer, sizeof(buffer));
        if (p_st->white_list->is_host_in_white_list(buffer))
        {
            if (p_st->enable_white_list_log)
                sysinfo_log(p_st->p_m_info, "[%s] whitelist [%s] touch\n", __func__, buffer);
            continue;
        }

        obj = cJSON_CreateObject();
        if (!obj) continue;

        cJSON_AddStringToObject(obj, "pid", conn->edr_pid);
        buffer[0] = 0;
        inet_ntop(conn->af, &conn->local.addr_data32[0], buffer, sizeof(buffer));
        cJSON_AddStringToObject(obj, "src_ip", buffer);
        cJSON_AddNumberToObject(obj, "src_port", conn->lport);
        buffer[0] = 0;
        inet_ntop(conn->af, &conn->foreign.addr_data32[0], buffer, sizeof(buffer));
        cJSON_AddStringToObject(obj, "dest_ip", buffer);
        cJSON_AddNumberToObject(obj, "dest_port", conn->fport);
        cJSON_AddStringToObject(obj, "protocol", proto2str(conn->type));
        cJSON_AddStringToObject(obj, "url", "");
        cJSON_AddNumberToObject(obj, "result", 0);
        cJSON_AddNumberToObject(obj, "op_time", time(NULL));

        cJSON_AddItemToArray(js_array, obj);

        if (p_st->enable_network_log)
        {
            sysinfo_log(p_st->p_m_info, "[%s] read from /proc/net/tcp\n", __func__);
        }
    }

    for (std::list<http_info_t *>::iterator itr = p_st->http_list.begin(); itr != p_st->http_list.end(); itr++)
    {
        http_info_t *ninfo = *itr;

        if (p_st->white_list->is_host_in_white_list(buffer))
        {
            if (p_st->enable_white_list_log)
                sysinfo_log(p_st->p_m_info, "[%s] whitelist [%s] touch\n", __func__, buffer);
            continue;
        }

        obj = cJSON_CreateObject();
        if (!obj) continue;

        // pid ->str
        buffer[0] = 0;
        EDR_id(p_st->proc_his.get_start_time(ninfo->pid), ninfo->pid, buffer, sizeof(buffer));
        cJSON_AddStringToObject(obj, "pid", buffer);
        // src ip
        buffer[0] = 0;
        inet_ntop(AF_INET, &ninfo->ip_hdr.source_address, buffer, sizeof(buffer));
        cJSON_AddStringToObject(obj, "src_ip", buffer);
        // src port
        cJSON_AddNumberToObject(obj, "src_port", ntohs(ninfo->tcp_hdr.source_port));
        // dst ip
        buffer[0] = 0;
        inet_ntop(AF_INET, &ninfo->ip_hdr.destination_address, buffer, sizeof(buffer));
        cJSON_AddStringToObject(obj, "dest_ip", buffer);
        // dst port
        cJSON_AddNumberToObject(obj, "dest_port", ntohs(ninfo->tcp_hdr.destination_port));
        // protocol
        cJSON_AddStringToObject(obj, "protocol", "HTTP");
        // url
        if (ninfo->p_url)
            cJSON_AddStringToObject(obj, "url", ninfo->p_url);
        else
            cJSON_AddStringToObject(obj, "url", "");
        // result
        cJSON_AddNumberToObject(obj, "result", 0);
        // op time
        cJSON_AddNumberToObject(obj, "op_time", ninfo->time);

        cJSON_AddItemToArray(js_array, obj);

        if (p_st->enable_network_log)
        {
            sysinfo_log(p_st->p_m_info, "[%s] url:[%s]\n", __func__, ninfo->p_url);
        }
    }

    return js_array;
}

static cJSON *proc_actions_dns_2_json(MainStatistics *p_st)
{
    cJSON *obj;
    cJSON *js_array = cJSON_CreateArray();
    if (!js_array) return NULL;
    char buffer[64];

    for (std::list<dns_info_t *>::iterator itr = p_st->dns_list.begin(); itr != p_st->dns_list.end(); itr++)
    {
        dns_info_t *dinfo = *itr;
        if (!dinfo->p_dns_query)
            continue;
        obj = cJSON_CreateObject();
        if (!obj) continue;

        if (p_st->p_processor && p_st->p_processor->get_permission(p_st->p_rules, dinfo->pid, EPROC_DNS) == false)
            continue;
        debug_print("%s %d get_permission ok\n", __func__, dinfo->pid);

        buffer[0] = 0;
        EDR_id(p_st->proc_his.get_start_time(dinfo->pid), dinfo->pid, buffer, sizeof(buffer));
        cJSON_AddStringToObject(obj, "pid", buffer);
        cJSON_AddStringToObject(obj, "dns_query", dinfo->p_dns_query);
        if (p_st->edition == edr::edition_t::EDITION_HUAWEI)
        {
            buffer[0] = 0;
            inet_ntop(AF_INET, &dinfo->ip_hdr.destination_address, buffer, sizeof(buffer));
            cJSON_AddStringToObject(obj, "dns_srvip", buffer);
        }
        buffer[0] = 0;
        inet_ntop(AF_INET, &dinfo->response_address, buffer, sizeof(buffer));
        cJSON_AddStringToObject(obj, "dns_resovle", buffer);
        cJSON_AddNumberToObject(obj, "op_time", dinfo->time);

        cJSON_AddItemToArray(js_array, obj);

        if (p_st->enable_network_log)
        {
            sysinfo_log(p_st->p_m_info, "[%s] url:[%s]\n", __func__, dinfo->p_dns_query);
        }
    }

    return js_array;
}

static cJSON *proc_action_node_2_json()
{
    cJSON *node_js = cJSON_CreateObject();
    if (!node_js) return NULL;

    cJSON_AddStringToObject(node_js, "nodeId", "-1");
    cJSON_AddNumberToObject(node_js, "time", time(NULL));
    return node_js;
}

static cJSON *proc_action_2_json(MainStatistics *p_st)
{
    cJSON *root = cJSON_CreateObject();
    if (!root) return NULL;

    cJSON *node_js = proc_action_node_2_json();
    cJSON *base_js_arr = proc_actions_proc_base_info_2_json(p_st);
    cJSON *module_js_arr = proc_actions_proc_module_info_2_json(p_st);
    cJSON *action_js_arr = proc_actions_proc_action_2_json(p_st);
    cJSON *file_js_arr = proc_actions_file_action_2_json(p_st);
    //Reg

    cJSON *net_action_js_arr = NULL;
    if (p_st->edition == edr::edition_t::EDITION_HUAWEI)
    {
        net_action_js_arr = proc_actions_net_2_json_huawei(p_st);
    }
    else if (p_st->edition == edr::edition_t::EDITION_EDR)
    {
        net_action_js_arr = proc_actions_http_2_json_edr(p_st);
    }
    cJSON *dns_action_js_arr = proc_actions_dns_2_json(p_st);

    cJSON_AddItemToObject(root, "Node", node_js);
    cJSON_AddItemToObject(root, "Proc_info", base_js_arr);
    cJSON_AddItemToObject(root, "Proc_module", module_js_arr);
    cJSON_AddItemToObject(root, "Proc_action", action_js_arr);
    cJSON_AddItemToObject(root, "File", file_js_arr);
    // Reg
    cJSON_AddItemToObject(root, "Net_action", net_action_js_arr);
    cJSON_AddItemToObject(root, "Dns_action", dns_action_js_arr);

    debug_print("base:%d, module:%d, action:%d, file:%d, net:%d, dns:%d\n",
            cJSON_GetArraySize(base_js_arr), cJSON_GetArraySize(module_js_arr), cJSON_GetArraySize(action_js_arr),
            cJSON_GetArraySize(file_js_arr), cJSON_GetArraySize(net_action_js_arr), cJSON_GetArraySize(dns_action_js_arr));

    return root;
}

static std::string write_action_rule_log_data(MainStatistics *p_st, time_t time_s, const char *tag, int msgid, int ruleid, const char *objects, int action, const char *result)
{
    char buffer[1024] = { 0 };
    struct tm tmnow;
    localtime_r(&time_s, &tmnow);
    snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "%d-%d-%dT%d:%d:%dZ ",
            1900 + tmnow.tm_year, tmnow.tm_mon + 1, tmnow.tm_mday, tmnow.tm_hour, tmnow.tm_min, tmnow.tm_sec);

    snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer),
            "%s %s "
            "%d %d "
            "%s %d %s",
            p_st->local_ip.data(), tag,
            msgid, ruleid,
            objects, action, result);
    std::string ret(buffer);
    return ret;
}

static int isolate_file(const char *file)
{
#define ISOLATE_DIRECTORY   "isolate_dir"
    if (path_exists(ISOLATE_DIRECTORY))
    {
        if (!path_isdir(ISOLATE_DIRECTORY))
        {
            remove(ISOLATE_DIRECTORY);
            debug_print("[%s] exists, but not dir, remove it\n", ISOLATE_DIRECTORY);
        }
    }
    if (!path_exists(ISOLATE_DIRECTORY))
    {
        mkdir(ISOLATE_DIRECTORY, 0777);
        debug_print("[%s] un-exists, make it\n", ISOLATE_DIRECTORY);
    }
    std::string src, dest;
    src = file;
    dest = dest + ISOLATE_DIRECTORY + '/';
    if (std::string::npos != src.find_last_of('/'))
    {
        dest = dest + src.substr(src.find_last_of('/') + 1);
    }
    else
    {
        dest = dest + src;
    }

    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm t;
    localtime_r(&tv.tv_sec, &t);
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%4d_%02d_%02d_%02d_%02d_%02d_%06ld",
            1900 + t.tm_year, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, tv.tv_usec);
    dest = dest + '.' + buffer;

    debug_print("isolate [%s] -> [%s]\n", file, dest.data());

    return move_file(file, dest.data());
}

static const char *exec_action(const char *objects, int pid, int action)
{
    if (!objects || !objects[0])
        return "Fail";
    int ret = -1;
    switch (action)
    {
        case SYS_TASK_ACTION_RULE_ACTION_WARNING:
            ret = 0;
            break;
        case SYS_TASK_ACTION_RULE_ACTION_FINISH_CONN:
            // TODO, it seems a little force
            ret = kill_process(pid, 9);
            ret = 0;
            break;
        case SYS_TASK_ACTION_RULE_ACTION_KILL_PROCESS:
            ret = kill_process(pid, 9);
            break;
        case SYS_TASK_ACTION_RULE_ACTION_DELETE_FILE:
            ret = delete_file(objects);
            break;
        case SYS_TASK_ACTION_RULE_ACTION_ISOLATE_FILE:
            ret = isolate_file(objects);
            break;
    }
    return ret ? "Fail" : "Success";
}

static task_record *task_record_create(const char *data, uint32_t data_size, const char *type, int upload_type)
{
    task_record *record = new task_record;
    if (!record) return NULL;

    record->data = data;
    record->data_size = data_size;
    record->type = type;
    record->upload_type = upload_type;
    return record;
}

static void action_rule_generate_log(MainStatistics *p_st)
{
    if (!p_st)
        return;
    for (std::multimap<sys_task_action_rule_t *, http_info_t *>::iterator itr = p_st->rule_packets.begin(); itr != p_st->rule_packets.end();)
    {
        sys_task_action_rule_t *rule = itr->first;
        http_info_t *ninfo = itr->second;
        time_t op_time = ninfo->time;
        std::string data = write_action_rule_log_data(p_st, op_time, "jmedr", 20,
                rule->rule_id, rule->objects.data(), rule->action, exec_action(rule->objects.data(), ninfo->pid, rule->action));
        task_record *record = task_record_create(data.data(), data.size(), "actionlog", SYS_UPLOAD_LOG_TYPE_ACTION_RULE);
        if (record)
        {
            if (p_st->p_upload)
            {
                p_st->p_upload->push_task(record);
            }
            task_record_free(record);
        }
        // now free http_info_t *
        http_info_free(ninfo);
        itr = p_st->rule_packets.erase(itr);
    }
}

static void action_rule_clear_timeout(MainStatistics *p_st)
{
    time_t now = time(NULL);
    for (std::vector<sys_task_action_rule_t *>::iterator itr = p_st->rules.begin(); itr != p_st->rules.end();)
    {
        if ((*itr)->finish_time == 0)
        {
            itr++;
        }
        else if ((*itr)->finish_time < now)
        {
            debug_print("rule_id:%d, finish_time:%lu < now:%lu, delete it\n",
                    (*itr)->rule_id, (*itr)->finish_time, now);
            delete (*itr);
            itr = p_st->rules.erase(itr);
        }
        else
        {
            itr++;
        }
    }
    // debug_print("total rules num [%d]\n", (int)p_st->rules.size());
}

static int packet_match_sys_task_rule(MainStatistics *p_st, http_info_t *p_info)
{
    sys_task_action_rule_t *rule;
    for (std::vector<sys_task_action_rule_t *>::iterator itr = p_st->rules.begin(); itr != p_st->rules.end(); itr++)
    {
        rule = *itr;
        if (p_info->time < rule->last_report_time + rule->report_delta)
            continue;

        if (SYS_TASK_ACTION_RULE_OBJECT_TYPE_URL == rule->object_type)
        {
            if (strstr(p_info->p_url, rule->objects.data()))
            {
                p_st->rule_packets.insert(std::pair<sys_task_action_rule_t *, http_info_t *>(rule, http_info_deep_copy(p_info)));
                rule->last_report_time = p_info->time;
                debug_print("rule:%d %s match url:%s\n", rule->rule_id, rule->objects.data(), p_info->p_url);
            }
        }
        else if (SYS_TASK_ACTION_RULE_OBJECT_TYPE_DOMAIN == rule->object_type)
        {
            if (strstr(p_info->p_url, rule->objects.data()))
            {
                p_st->rule_packets.insert(std::pair<sys_task_action_rule_t *, http_info_t *>(rule, http_info_deep_copy(p_info)));
                rule->last_report_time = p_info->time;
                debug_print("rule:%d %s match domain:%s\n", rule->rule_id, rule->objects.data(), p_info->p_url);
            }
        }
        else if (SYS_TASK_ACTION_RULE_OBJECT_TYPE_IP == rule->object_type)
        {
            char src_ip_str[16]; src_ip_str[0] = '\0';
            inet_ntop(AF_INET, &p_info->ip_hdr.source_address, src_ip_str, sizeof(src_ip_str));
            if (0 == strcmp(src_ip_str, rule->objects.data()))
            {
                p_st->rule_packets.insert(std::pair<sys_task_action_rule_t *, http_info_t *>(rule, http_info_deep_copy(p_info)));
                rule->last_report_time = p_info->time;
                debug_print("rule:%d %s match ip_str:%s\n", rule->rule_id, rule->objects.data(), src_ip_str);
            }
        }
    }
    return 0;
}

static void action_rule_proc_statis(MainStatistics *p_st)
{
    if (!p_st)
        return;
    time_t time_s = time(NULL);
    for (std::vector<sys_task_action_rule_t *>::iterator itr = p_st->rules.begin(); itr != p_st->rules.end(); ++itr)
    {
        sys_task_action_rule_t *rule = *itr;
        if (!rule)
        {
            continue;
        }
        if (time_s < rule->last_report_time + rule->report_delta)
        {
            continue;
        }
        if (rule->object_type == SYS_TASK_ACTION_RULE_OBJECT_TYPE_MD5)
        {
            file_digest_t *md5_info = get_file_digest_by_md5(p_st->md5_cache, rule->objects.data());
            if (!md5_info)
            {
                continue;
            }
            if (!path_exists(md5_info->file_path))
            {
                continue;
            }
            task_record *record = NULL;
            if (rule->action == SYS_TASK_ACTION_RULE_ACTION_ISOLATE_FILE || rule->action == SYS_TASK_ACTION_RULE_ACTION_DELETE_FILE || rule->action == SYS_TASK_ACTION_RULE_ACTION_WARNING)
            {
                std::string data = write_action_rule_log_data(p_st, time(NULL), "jmedr", 20,
                        rule->rule_id, rule->objects.data(), rule->action, exec_action(md5_info->file_path, 0, rule->action));
                record = task_record_create(data.data(), data.size(), "actionlog", SYS_UPLOAD_LOG_TYPE_ACTION_RULE);
                rule->last_report_time = time_s;
            }
            else
            {
                Proc *proc = get_proc_by_path(p_st->ps_curr, md5_info->file_path);
                if (proc)
                {
                    std::string data = write_action_rule_log_data(p_st, time(NULL), "jmedr", 20,
                            rule->rule_id, rule->objects.data(), rule->action, exec_action(rule->objects.data(), proc->pid, rule->action));
                    record = task_record_create(data.data(), data.size(), "actionlog", SYS_UPLOAD_LOG_TYPE_ACTION_RULE);
                    rule->last_report_time = time_s;
                }
            }
            if (record)
            {
                if (p_st->p_upload)
                {
                    p_st->p_upload->push_task(record);
                }
                task_record_free(record);
            }
            free(md5_info);
        }
        else if (rule->object_type == SYS_TASK_ACTION_RULE_OBJECT_TYPE_IP)
        {
            uint32_t addr = inet_addr(rule->objects.data());
            RecordCtx *ctx = p_st->conn_curr;
            if (!ctx)
                continue;
            Connect *conn;
            for (size_t i = 0, total = ctx->conns_head.used; i<total; i++)
            {
                conn = ctx->conns_head.conns[i];
                if (!conn || !conn->ino)
                    continue;
                if (addr == conn->local.addr_data32[0] ||
                        addr == conn->foreign.addr_data32[0])
                {
                    debug_print("find conn %s, pid:%s\n", rule->objects.data(), conn->edr_pid);
                    if (strlen(conn->edr_pid) > 5)
                    {
                        uint64_t pid = atoi(&conn->edr_pid[strlen(conn->edr_pid) - 5]);
                        std::string data = write_action_rule_log_data(p_st, time(NULL), "jmedr", 20,
                                rule->rule_id, rule->objects.data(), rule->action, exec_action(rule->objects.data(), pid, rule->action));
                        task_record *record = task_record_create(data.data(), data.size(), "actionlog", SYS_UPLOAD_LOG_TYPE_ACTION_RULE);
                        rule->last_report_time = time_s;
                        if (record)
                        {
                            if (p_st && p_st->p_upload)
                            {
                                p_st->p_upload->push_task(record);
                            }
                            task_record_free(record);
                        }

                    }
                }
            }
        }
        else
        {
            // in packet module
        }
    }
}

static void action_rule_statis(MainStatistics *p_st)
{
    // remove other module rules if  timeout
    action_rule_proc_statis(p_st);
    action_rule_generate_log(p_st);

    // before delete rules of timeout,  first delete it from other module;
    action_rule_clear_timeout(p_st);
}

static void snap_shot_statis(MainStatistics *p_st)
{
    if (!p_st)
        return;
    // write snapshot to json
    cJSON *shot_json = snap_shot_2_json(p_st);
    if (!shot_json)
    {
        INFO_PRINT("snap_shot_2_json failed\n");
        return;
    }

    // push into uploader ringbuffer
    json_file *shot_file = write_json_file(shot_json, "file.json", "snap.zip", "edrsnap", SYS_UPLOAD_LOG_TYPE_SNAP_SHOT);
    if (shot_file)
    {
        if (p_st->p_upload)
        {
            p_st->p_upload->push_task(shot_file);
        }
        json_file_free(shot_file);
    }
}

static void proc_action_statis(MainStatistics *p_st)
{
    cJSON *proc_json = proc_action_2_json(p_st);
    if (!proc_json)
    {
        INFO_PRINT("proc_action_2_json failed\n");
        return;
    }

    // push into uploader ringbuffer
    json_file *proc_file = write_json_file(proc_json, "file.json", "action.zip", "edraction", SYS_UPLOAD_LOG_TYPE_PROC_ACTION);
    if (proc_file)
    {
        if (p_st && p_st->p_upload)
        {
            p_st->p_upload->push_task(proc_file);
        }
        json_file_free(proc_file);
    }
}

static int sys_task_md5_parse(sys_task_t *task, sys_task_md5_t *result)
{
    cJSON *params_js = cJSON_Parse(task->params);
    if (!params_js)
    {
        INFO_PRINT("cJSON_Parse failed, params is:%s \n", task->params);
        return -1;
    }
    cJSON *item_js;
    item_js = cJSON_GetObjectItem(params_js, "md5");
    if (item_js)
    {
        strncpy(result->md5_str, item_js->valuestring, sizeof(result->md5_str));
    }
    else
    {
        INFO_PRINT("cJSON_GetObjectItem(%s) failed, params is:%s \n", "md5", task->params);
        cJSON_Delete(params_js);
        return -1;
    }
    item_js = cJSON_GetObjectItem(params_js, "finish_time");
    if (item_js)
    {
        result->finish_time = (time_t)item_js->valuedouble;
    }
    else
    {
        INFO_PRINT("cJSON_GetObjectItem(%s) failed, params is:%s \n", "finish_time", task->params);
        cJSON_Delete(params_js);
        return -1;
    }

    debug_print("parse result[md5:%s, finish_time:%lu]\n", result->md5_str, result->finish_time);

    cJSON_Delete(params_js);
    return 0;
}

static task_record *write_sys_md5_log(MainStatistics *p_st, sys_task_md5_t *task)
{
    Proc *proc = NULL;
    const char *name, *path, *cmd, *proc_md5 = NULL, *check_md5 = NULL, *check_path = NULL;
    int pid, ppid;
    char buffer[1024] = { 0 };

    // md5 ---get---> file && pid
    file_digest_t *ft_check_file = get_file_digest_by_md5(p_st->md5_cache, task->md5_str);
    if (ft_check_file)
    {
        check_md5 = ft_check_file->md5;
        check_path = ft_check_file->file_path;

        proc = get_proc_by_path(p_st->ps_curr, ft_check_file->file_path);
        if (!proc)
        {
            debug_print("get_proc_by_path(%s) find NULL\n", ft_check_file->file_path);
        }
        struct tm tmnow;
        localtime_r(&ft_check_file->created_at, &tmnow);
        snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "%d-%d-%dT%d:%d:%dZ ",
                1900 + tmnow.tm_year, tmnow.tm_mon + 1, tmnow.tm_mday, tmnow.tm_hour, tmnow.tm_min, tmnow.tm_sec);
    }
    else
    {
        check_md5 = "-";
        check_path = "-";

        debug_print("get_file_digest_by_md5(%s) find NULL\n", task->md5_str);
        strncat(buffer, "1970-1-1T00:00:00Z", sizeof(buffer) - strlen(buffer));
    }

    pid = 0; ppid = 0; name = path = cmd = "-";
    if (proc)
    {
        pid = proc->pid; ppid = proc->ppid;
        if (proc->name.size())
            name = proc->name.data();
        if (proc->abs_name.size())
            path = proc->abs_name.data();
        if (proc->cmdline.size())
            cmd = proc->cmdline.data();
    }

    file_digest_t *ft_proc_file = NULL;
    if (path && '/' == path[0])
    {
        ft_proc_file = get_file_digest_by_path(p_st->md5_cache, path);
        if (ft_proc_file)
            proc_md5 = ft_proc_file->md5;
    }
    if (!proc_md5)
        proc_md5 = "-";

    snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer),
            "%s %s %s "
            "\"%s\" "
            "%d %d "
            "%s \"%s\" "
            "%s %s \"%s\"",
            p_st->local_ip.data(), "jmedr", "10",
            name,
            pid, ppid,
            path, cmd,
            proc_md5, check_md5, check_path);
    task_record *record = task_record_create(buffer, strlen(buffer), "md5log", SYS_UPLOAD_LOG_TYPE_MD5_LOG);

    free(ft_check_file);
    if (ft_proc_file)
        free(ft_proc_file);
    return record;
}

static void process_md5_log(MainStatistics *p_st, sys_task_t *task)
{
    if (!p_st)
        return;
    if (!task)
        return;
    sys_task_md5_t result;
    memset(&result, 0, sizeof(result));
    if (sys_task_md5_parse(task, &result) < 0)
    {
        sysinfo_log(p_st->p_m_info, "%s sys_task_md5_parse failed", __func__);
        return;
    }

    if (result.finish_time && result.finish_time < time(NULL))
    {
        sysinfo_log(p_st->p_m_info, "%s time is no effect, ignore, finish_time:%lu, now:%lu\n",
                __func__, result.finish_time, time(NULL));
        return;
    }

    task_record *record = write_sys_md5_log(p_st, &result);
    if (record)
    {
        if (p_st->p_upload)
        {
            p_st->p_upload->push_task(record);
        }
        task_record_free(record);
    }
}

static int sys_task_snap_shot_parse(sys_task_t *task, sys_task_snap_shot_delta_t *result)
{
    cJSON *params_js = cJSON_Parse(task->params);
    if (!params_js)
    {
        debug_print("cJSON_Parse failed, params is:%s \n", task->params);
        return -1;
    }
    cJSON *item_js;
    item_js = cJSON_GetObjectItem(params_js, "report_proc");
    if (item_js)
    {
        result->report_proc = (int)item_js->valuedouble;
    }
    else
    {
        debug_print("cJSON_GetObjectItem(%s) failed, params is:%s\n",
                "report_proc", task->params);
        cJSON_Delete(params_js);
        return -1;
    }
    item_js = cJSON_GetObjectItem(params_js, "report_net");
    if (item_js)
    {
        result->report_net = (int)item_js->valuedouble;
    }
    else
    {
        debug_print("cJSON_GetObjectItem(%s) failed, params is:%s\n",
                "report_net", task->params);
        cJSON_Delete(params_js);
        return -1;
    }
    item_js = cJSON_GetObjectItem(params_js, "report_host");
    if (item_js)
    {
        result->report_host = (int)item_js->valuedouble;
    }
    else
    {
        debug_print("cJSON_GetObjectItem(%s) failed, params is:%s\n",
                "report_host", task->params);
        cJSON_Delete(params_js);
        return -1;
    }
    cJSON_Delete(params_js);
    return 0;
}

static void process_snap_shot_delta(MainStatistics *p_st, sys_task_t *task)
{
    sys_task_snap_shot_delta_t result;
    memset(&result, 0, sizeof(result));
    if (sys_task_snap_shot_parse(task, &result) < 0)
    {
        sysinfo_log(p_st->p_m_info, "%s sys_task_snap_shot_parse failed", __func__);
    }
    else
    {
        snap_shot_set_statis_delta(p_st, result.report_host, result.report_proc, result.report_net);
        sysinfo_log(p_st->p_m_info, "%s set snapshot host_delta:%d, proc_delta:%d, net_delta:%d",
                __func__, result.report_host, result.report_proc, result.report_net);
    }
}

#if 0
static int sys_task_check_proc_by_connection_parse(sys_task_t *task, sys_task_check_proc_by_connection_t *result)
{
    cJSON *params_js = cJSON_Parse(task->params);
    if (!params_js)
    {
        debug_print("cJSON_Parse failed, params is:%s \n", task->params);
        return -1;
    }
    cJSON *item_js;
    item_js = cJSON_GetObjectItem(params_js, "pid");
    if (item_js)
    {
        result->pid = (int)item_js->valuedouble;
    }
    else
    {
        result->pid = 0;
    }

    item_js = cJSON_GetObjectItem(params_js, "destip");
    if (item_js)
    {
        result->dest_ip = ntohl(item_js->valuedouble);
    }
    else
    {
        result->dest_ip = 0;
    }

    item_js = cJSON_GetObjectItem(params_js, "port");
    if (item_js)
    {
        result->dest_port = (int)item_js->valuedouble;
    }
    else
    {
        result->dest_port = 0;
    }

    item_js = cJSON_GetObjectItem(params_js, "domain");
    if (item_js && item_js->valuestring)
    {
        result->domain = strdup(item_js->valuestring);
    }
    else
    {
        result->domain = NULL;
    }

    item_js = cJSON_GetObjectItem(params_js, "time");
    if (item_js)
    {
        result->op_time = (int)item_js->valuedouble;
    }
    else
    {
        result->op_time = 0;
    }

    cJSON_Delete(params_js);
    return 0;
}

#else
#include "json/json.h"
static int sys_task_check_proc_by_connection_parse(sys_task_t *task, sys_task_check_proc_by_connection_t *result)
{
    //  virtual bool parse(   
    //                     273       char const* beginDoc, char const* endDoc,
    //                     274       Value* root, JSONCPP_STRING* errs) = 0;

    Json::CharReaderBuilder readerbuilder;
    Json::CharReader *reader = readerbuilder.newCharReader();
    JSONCPP_STRING errs;

    Json::Value root;
    if (false == reader->parse(task->params, task->params + strlen(task->params), &root, &errs))
    {
        debug_print(" parse [%s] [%s] failed\n", task->cmd, task->params);
        return -1;
    }
    delete reader;

    if (root["pid"].isUInt64())
    {
        result->edr_pid = root["pid"].asUInt64();
        result->pid = result->edr_pid % STRONGLY_SET_MAX_PID;
    }
    if (root["destip"].isUInt())
    {
        result->dest_ip = ntohl(root["destip"].asUInt());
    }
    if (root["port"].isInt())
    {
        result->dest_port = (uint16_t)root["port"].asInt();
    }
    if (root["domain"].isString())
    {
        result->domain = root["domain"].asString();
    }
    if (root["time"].isUInt())
    {
        result->op_time = root["time"].asUInt();
    }
    return 0;
}
#endif

static task_record *write_sys_file_log_by_pid(MainStatistics *p_st, uint64_t pid, const char *url_type)
{
    Proc *proc = get_proc_by_PID(p_st->ps_curr, pid);
    if (!proc)
    {
        debug_print("%s: proc(pid:%d) is not running, ignore\n",
                url_type, (int)pid);
        return NULL;
    }
    if (!proc->abs_name.size() || proc->abs_name[0] != '/')
    {
        debug_print("%s: can't read proc(pid:%d) main module, ignore\n",
                url_type, (int)pid);
        return NULL;
    }

    task_record *record = task_record_create(proc->abs_name.data(), proc->abs_name.size(), url_type, SYS_UPLOAD_LOG_TYPE_PROC_FILE);
    return record;
}

static void process_check_proc_by_connection(MainStatistics *p_st, sys_task_t *task)
{
    if (!p_st || !task)
        return;
    sys_task_check_proc_by_connection_t result;
    if (sys_task_check_proc_by_connection_parse(task, &result) < 0)
    {
        sysinfo_log(p_st->p_m_info, "%s sys_task_check_proc_by_connection_parse failed", __func__);
        return;
    }
    if (result.op_time && result.op_time < time(NULL))
    {
        sysinfo_log(p_st->p_m_info, "%s time < now, ignore it", __func__);
        return;
    }

    std::string fname;
    if (0 == result.pid)
    {
        if (result.dest_ip && result.dest_port)
        {
            Connect *conn = SearchConnByDest(p_st->conn_curr, result.dest_ip, result.dest_port);
            if (conn)
            {
                Proc *proc = get_proc_by_sock_ino(p_st->ps_curr, conn->ino);
                if (proc)
                {
                    result.pid = (int)proc->pid;
                }
                else
                {
                    sysinfo_log(p_st->p_m_info, "[%s] get_proc_by_sock_ino(%llu) --> NULL\n", __func__, (unsigned long long)conn->ino);
                }
            }
            else
            {
                sysinfo_log(p_st->p_m_info, "[%s] SearchConnByDest(%d:%d) --> NULL\n", __func__, result.dest_ip, result.dest_port);
            }
        }
        else if (false == result.domain.empty())
        {
            url_cache url_cache = p_st->net_his.search(result.domain.data());
            if (url_cache.pid)
            {
                result.pid = url_cache.pid;
                fname = p_st->proc_his.get_name(url_cache.pid);
            }
            else
            {
                sysinfo_log(p_st->p_m_info, "[%s] net_history_search(%s) --> NULL\n", __func__, result.domain.data());
            }
        }
    }

    if (result.pid)
    {
        task_record *record = NULL;
        if (false == fname.empty())
            record = task_record_create(fname.data(), fname.size(), "edrsuspicious", SYS_UPLOAD_LOG_TYPE_PROC_FILE);
        else
            record = write_sys_file_log_by_pid(p_st, result.pid, "edrsuspicious");
        if (record)
        {
            if (p_st->p_upload)
            {
                p_st->p_upload->push_task(record);
            }
            task_record_free(record);
        }
        else
        {
            sysinfo_log(p_st->p_m_info, "%s write_sys_file_log_by_pid(%llu) failed, pid is exit\n",
                    __func__, result.pid);
        }
    }

}

static int sys_task_action_rule_parse(sys_task_t *task, std::vector<sys_task_action_rule_t *> &results)
{
    cJSON *params_js = cJSON_Parse(task->params);
    if (!params_js)
    {
        INFO_PRINT("cJSON_Parse failed, params is:%s \n", task->params);
        return -1;
    }

    cJSON *item_js;
    item_js = cJSON_GetObjectItem(params_js, "interval_time");
    int check_delta = 0;
    if (item_js)
    {
        check_delta = (int)item_js->valuedouble;
    }

    item_js = cJSON_GetObjectItem(params_js, "report_host");
    int report_delta = 0;
    if (item_js)
    {
        report_delta = (int)item_js->valuedouble;
    }
    report_delta = MAX(report_delta, check_delta);

    item_js = cJSON_GetObjectItem(params_js, "rules");
    if (!cJSON_IsArray(item_js))
    {
        INFO_PRINT("get(%s) failed, I need an array\n", "rules");
        cJSON_Delete(params_js);
        return -1;
    }
    int rules_num = cJSON_GetArraySize(item_js);
    if (0 == rules_num)
    {
        INFO_PRINT("cJSON_GetArraySize is 0, I need > 0\n");
        cJSON_Delete(params_js);
        return -1;
    }
    for (int i = 0; i<rules_num; i++)
    {
        cJSON *rule_js = cJSON_GetArrayItem(item_js, i);
        if (!rule_js)
            continue;
        sys_task_action_rule_t result;
        cJSON *it_js = cJSON_GetObjectItem(rule_js, "rule_id");
        if (it_js && it_js->valuestring)
        {
            result.rule_id = (int)atoi(it_js->valuestring);
        }
        it_js = cJSON_GetObjectItem(rule_js, "object_type");
        if (it_js && it_js->valuestring)
        {
            result.object_type = (int)atoi(it_js->valuestring);
        }
        it_js = cJSON_GetObjectItem(rule_js, "action");
        if (it_js && it_js->valuestring)
        {
            result.action = (int)atoi(it_js->valuestring);
        }
        it_js = cJSON_GetObjectItem(rule_js, "finish_time");
        if (it_js)
        {
            result.finish_time = (int)it_js->valuedouble;
        }
        it_js = cJSON_GetObjectItem(rule_js, "objects");
        if (!it_js)
        {
            INFO_PRINT("%d get(%s) failed, format error\n", i, "objects");
            continue;
        }
        if (!cJSON_IsArray(it_js))
        {
            INFO_PRINT("%d get(%s) failed, I need an array\n", i, "objects");
            continue;
        }
        int objects_num = cJSON_GetArraySize(it_js);
        if (objects_num < 1)
        {
            INFO_PRINT("cJSON_GetArraySize(%d) , I need > 0\n", objects_num);
            continue;
        }
        for (int j = 0; j<objects_num; j++)
        {
            cJSON *i_js = cJSON_GetArrayItem(it_js, j);
            if (!i_js || !i_js->valuestring) continue;
            sys_task_action_rule_t *res = new sys_task_action_rule_t;
            res->rule_id = result.rule_id;
            res->object_type = result.object_type;
            res->action = result.action;
            res->finish_time = result.finish_time;
            res->check_delta = check_delta;
            res->report_delta = report_delta;
            res->objects = i_js->valuestring;
            results.push_back(res);
        }
    }
    cJSON_Delete(params_js);
    return 0;
}

static void process_action_rule(MainStatistics *p_st, sys_task_t *task)
{
    std::vector<sys_task_action_rule_t *> results;
    if (sys_task_action_rule_parse(task, results) < 0)
    {
        sysinfo_log(p_st->p_m_info, "%s sys_task_action_rule_parse failed", __func__);
        return;
    }

    // clear all_about old rules
    action_rule_statis(p_st);
    for (std::multimap<sys_task_action_rule_t *, http_info_t *>::iterator itr = p_st->rule_packets.begin(); itr != p_st->rule_packets.end();)
    {
        http_info_free(itr->second);
        itr = p_st->rule_packets.erase(itr);
    }
    // delete memory safety
    for (std::vector<sys_task_action_rule_t *>::iterator itr = p_st->rules.begin(); itr != p_st->rules.end();)
    {
        delete *itr;
        itr = p_st->rules.erase(itr);
    }

    // send rules to other module
    for (std::vector<sys_task_action_rule_t *>::iterator itr = results.begin(); itr != results.end(); ++itr)
    {

        std::string &s = (*itr)->objects;
        if ((*itr)->object_type == SYS_TASK_ACTION_RULE_OBJECT_TYPE_MD5)
        {
            for (int i = 0, len = s.size(); i<len; i++)
            {
                s[i] = tolower(s[i]);
            }
        }
        else if ((*itr)->object_type == SYS_TASK_ACTION_RULE_OBJECT_TYPE_URL)
        {
            if (0 == strncmp(s.data(), "http://", strlen("http://")))
            {
                s = s.substr(strlen("http://"));
            }
        }

        sysinfo_log(p_st->p_m_info, "%s [rule_id:%d, object_type:%d, objects:%s, action:%d, time:%lu]\n",
                __func__, (*itr)->rule_id, (*itr)->object_type, (*itr)->objects.data(), (*itr)->action, (*itr)->finish_time);

        p_st->rules.push_back(*itr);
    }
}

static void process_sys_task(MainStatistics *p_st)
{
    sys_task_t *task;
    while (NULL != (task = sys_task_pop(p_st->sys_tasks)))
    {
        debug_print("cmd[%s], params:[%s]\n", task->cmd, task->params);
        tunnel_input(task);
        if (task->cmd && task->params)
        {
            if (0 == strcmp(task->cmd, "CheckByMD5"))
            {
                process_md5_log(p_st, task);
            }
            else if (0 == strcmp(task->cmd, "CheckByIOC"))
            {
                // TODO:
                INFO_PRINT("CheckByIOC unimplement\n");
            }
            else if (0 == strcmp(task->cmd, "SetSnapTime"))
            {
                process_snap_shot_delta(p_st, task);
            }
            else if (0 == strcmp(task->cmd, "CheckByConnection"))
            {
                process_check_proc_by_connection(p_st, task);
            }
            else if (0 == strcmp(task->cmd, "SetActionRule"))
            {
                process_action_rule(p_st, task);
            }
        }
        sys_task_free(task);
    }
}

static void usb_file_action(MainStatistics *p_st, usb_action_t *action)
{
    task_record *record = task_record_create(action->path, strlen(action->path), "edrudiskfile", SYS_UPLOAD_LOG_TYPE_PROC_FILE);
    if (record)
    {
        if (p_st && p_st->p_upload)
        {
            p_st->p_upload->push_task(record);
        }
        task_record_free(record);
    }
}

static void process_usb_action(MainStatistics *p_st)
{
    if (!p_st)
        return;
    usb_action_t *p_action;
    while (NULL != (p_action = usb_action_pop(p_st->usb_actions)))
    {
        if (atoi(p_action->path) > 0)
        {
            // this is a pid
            task_record *record = write_sys_file_log_by_pid(p_st, atoi(p_action->path), "edrudiskfile");
            if (record)
            {
                if (p_st->p_upload)
                {
                    p_st->p_upload->push_task(record);
                }
                task_record_free(record);
            }
            free(p_action);
        }
        else
        {
            // this is a path action
            usb_file_action(p_st, p_action);
        }
    }
    return;
}

static void process_file_action(MainStatistics *p_st)
{
    file_action_t *action;
    while (NULL != (action = file_action_pop(p_st->file_actions)))
    {
        cJSON *src_file_js = cJSON_Parse(action->json_str);
        if (src_file_js)
        {
            cJSON *file_js = file_action_fill_info(p_st, src_file_js);
            if (file_js)
            {
                p_st->file_list.push_back(file_js);
            }
        }
        else
        {
            sysinfo_log(p_st->p_m_info, "file action parse failed %s\n", action->json_str);
        }
        file_action_free(action);
    }
}

#ifdef EDR_DEBUG

static void fake_sys_task(MainStatistics *p_st)
{
    usb_action_t *action = (usb_action_t *)calloc(1, sizeof(usb_action_t));
    if (action)
    {
        strcpy(action->path, "/home/lidawei/a");
        action->state = 1;
        if (usb_action_push(p_st, action))
        {
            usb_action_free(action);
        }
    }

    static bool sys_task_done = false;
    if (true == sys_task_done)
        return;
    sys_task_done = true;

    struct fake_task {
        const char *cmd;
        const char *params;
    };
    static fake_task fake_tasks[] = {
        { "CheckByConnection",   "{\"destip\":\"\",\"domain\":\"www.baidu.com\",\"pid\":\"0\",\"port\":0,\"time\":88}" },
        { "SetActionRule",       "{\"interval_time\":10,\"report_host\":5,\"rules\":[{\"action\":\"1\",\"finish_time\":0,\"object_type\":\"2\",\"objects\":[\"www.baidu.com\",\"www.163.com\"],\"rule_id\":\"1232\"},{\"action\":\"1\",\"finish_time\":0,\"object_type\":\"4\",\"objects\":[\"885acc6870b8ba98983e88e578179a2c\",\"acf8da07c687dbfb0579af4a6dd31871\"],\"rule_id\":\"1232\"}]}" },
        { "CheckByMD5",          "{\"md5\":\"1ca402ca438ed74b69acce1a710537f3\",\"finish_time\":0,\"mode\":0}" },
        { "SetSnapTime",         "{\"report_proc\":\"2\",\"report_net\":3,\"report_host\":1}" }
    };

    sys_task_t *task = NULL;
    for (size_t i = 0; i<sizeof(fake_tasks) / sizeof(fake_tasks[0]); i++)
    {
        task = (sys_task_t *)calloc(1, sizeof(sys_task_t));
        if (task)
        {
            task->cmd = strdup(fake_tasks[i].cmd);
            task->cmd_size = strlen(fake_tasks[i].cmd);
            task->params = strdup(fake_tasks[i].params);
            task->params_size = strlen(fake_tasks[i].params);
            if (sys_task_push(p_st, task))
            {
                INFO_PRINT("sys_task_push failed  fake cmd[%s], params[%s]\n",
                        task->cmd, task->params);
                sys_task_free(task);
            }
        }
    }
}
#endif

static void *sys_info_statis_loop(void *argu)
{
    MainStatistics *p_st = (MainStatistics *)argu;

    if (sysinfo_init(p_st) < 0)
        return NULL;

    if (pre_alloc(&ub, 1024))
        return NULL;

    load_tunnel(p_st);

    time_t finish_time = 0;
    if (p_st->p_processor)
    {
        finish_time = p_st->p_processor->get_finish_time(p_st->p_rules);
    }

    while (p_st->running)
    {
        // uint64_t time_ms = gettimeofday_ms();
        // time_t time_s = time_ms/1000;
        time_t time_s = time(NULL);

        if (p_st->p_upload && p_st->p_upload->isstop())
        {
            // sys_info_statis_stop(p_st);
            // sys_info_statis_destroy(p_st);
            sysinfo_log(p_st->p_m_info, "uploader is stop\n");
            exit(0);
        }

        if (finish_time && time_s >= finish_time)
        {
            // sys_info_statis_stop(p_st);
            // sys_info_statis_destroy(p_st);
            sysinfo_log(p_st->p_m_info, "HWRule is to finish time %lu>=%lu\n", time_s, finish_time);
            p_st->p_upload->flush();
			goto sys_info_statis_once_end;
        }

        if (UPDATE_SUCCESS != ps_update(p_st) && !time_to_update_sysinfo(time_s, p_st))
        {
            goto sys_info_statis_once_end;
        }

        if (sysinfo_update(p_st) < 0)
        {
            sysinfo_log(p_st->p_m_info, "sysinfo_update failed\n");
            goto sys_info_statis_once_end;
        }

        // TODO: write md5 to actions
#if 1
        // fill some infomation(pid, ...) in time
        process_usb_action(p_st);
        process_file_action(p_st);
#endif

        // action rule
        action_rule_statis(p_st);
        // recv sys task
        process_sys_task(p_st);

        if (time_to(time_s, p_st->cache_timeout_prev, p_st->cache_timeout_delta))
        {
            p_st->net_his.clear_timeout(p_st->cache_timeout_delta);
            p_st->proc_his.clear_timeout(p_st->cache_timeout_delta);
            p_st->cache_timeout_prev = time_s;
        }

        if (p_st->edition != edr::edition_t::EDITION_HUAWEI)
        {
            if (time_to_update_snap_shot(time_s, p_st))
            {
                snap_shot_statis(p_st);
            }
        }

        if (time_to_update_proc_action(time_s, p_st))
        {
            p_st->white_list->load_config();

            proc_action_statis(p_st);

            actions_clear(p_st);
            p_st->proc_his.clear_timeout(p_st->proc_action_time_delta);
            p_st->proc_action_time_prev = time_s;
        }

#ifdef EDR_DEBUG
        fake_sys_task(p_st);
#endif


sys_info_statis_once_end:
        sleep(1);
        // usleep(500 * 1000);
    }
    free(ub.buf);
    return NULL;
}

void sys_info_statis_stop(MainStatistics *p_st)
{
    if (!p_st)
        return;
    p_st->running = 0;
    if (p_st && p_st->main_pid)
    {
        pthread_join(p_st->main_pid, NULL);
        p_st->main_pid = 0;
    }
    if (p_st && p_st->proc_monitor_pid)
    {
        stop_proc_monitor(p_st->p_proc_monitor);
        pthread_join(p_st->proc_monitor_pid, NULL);
        p_st->proc_monitor_pid = 0;
    }
    if (p_st && p_st->net_monitor_pid)
    {
        stop_net_monitor(p_st->p_net_monitor);
        pthread_join(p_st->net_monitor_pid, NULL);
        p_st->net_monitor_pid = 0;
    }
    if (p_st && p_st->upload_pid)
    {
        if (p_st && p_st->p_upload)
        {
            p_st->p_upload->stop();
        }
        pthread_join(p_st->upload_pid, NULL);
        p_st->upload_pid = 0;
    }
}

void sys_info_statis_destroy(MainStatistics *p_st)
{
    if (p_st)
    {
        fin_proc_monitor(p_st->p_proc_monitor);
        fin_net_monitor(p_st->p_net_monitor);

        actions_clear(p_st);
        pthread_spin_destroy(&p_st->net_monitor_lock);
        // TODO implement cb
        if (p_st->httpes)     ring_buffer_clear_destroy(p_st->httpes, NULL);
        if (p_st->dnses)    ring_buffer_clear_destroy(p_st->dnses, NULL);
        if (p_st->icmpes)     ring_buffer_clear_destroy(p_st->icmpes, NULL);
        if (p_st->tcpes)     ring_buffer_clear_destroy(p_st->tcpes, NULL);
        if (p_st->udpes)     ring_buffer_clear_destroy(p_st->udpes, NULL);
        if (p_st->p_rules)     delete p_st->p_rules;
        if (p_st->p_processor)        delete p_st->p_processor;
        if (p_st->snap_shots)      ring_buffer_clear_destroy(p_st->snap_shots, NULL);
        if (p_st->proc_actions)      ring_buffer_clear_destroy(p_st->proc_actions, NULL);

        //        if (p_st->shot_records)    ring_buffer_clear_destroy(p_st->shot_records, json_file_free);
        //        if (p_st->action_records)  ring_buffer_clear_destroy(p_st->action_records, json_file_free);
        //        if (p_st->task_records)    ring_buffer_clear_destroy(p_st->task_records, task_record_free);
        if (p_st->sys_tasks)       ring_buffer_clear_destroy(p_st->sys_tasks, sys_task_free);
        if (p_st->usb_actions)     ring_buffer_clear_destroy(p_st->usb_actions, usb_action_free);
        if (p_st->file_actions)    ring_buffer_clear_destroy(p_st->file_actions, file_action_free);
        if (p_st->ps_prev)         ps_destroy(p_st->ps_prev);
        if (p_st->sys_stat_prev)   free(p_st->sys_stat_prev);
        if (p_st->p_users_passwd)  users_passwd_free(p_st->p_users_passwd);
        if (p_st->p_upload) delete p_st->p_upload;
        delete p_st;
    }
}

static bool match_rule(MainStatistics *p_st, object_type_t type, void *p_info)
{
    if (p_st == NULL || p_st->p_processor == NULL || p_st->p_rules == NULL || p_info == NULL)
        return false;
    return p_st->p_processor->match(p_st->p_rules, type, p_info);
}

static int link2name(const char *link, char *name, size_t nameLen)
{
    int n = readlink(link, name, nameLen);
    if (n < 0)
    {
        name[0] = '\0';
        return n;
    }
    if (n > (int)nameLen)
        n = nameLen;
    name[n] = '\0';
    return n;
}

static void huawei_push_task_file(MainStatistics *p_st, int pid)
{
    if (pid == 0)
    {
        sysinfo_log(p_st->p_m_info, "%s pid is 0\n", __func__);
        return;
    }
    char link[1024], absn[1024];
    snprintf(link, sizeof(link), "/proc/%d/exe", pid);
    if (link2name(link, absn, sizeof(absn)) <= 0)
    {
        sysinfo_log(p_st->p_m_info, "%s readlink(%s) failed\n", __func__, link);
        return;
    }
    task_record task;
    task.data = absn;
    sysinfo_log(p_st->p_m_info, "%s readlink(%s) -> (%s), push to upload\n", __func__, link, absn);
    p_st->p_upload->push_task(&task);
}

static void on_http(http_info_t *p_info, void *p_user_data)
{
    MainStatistics *p_st = (MainStatistics *)p_user_data;

    debug_print("%s %s\n", __func__, p_info->p_url);

    if (p_st->edition == edr::edition_t::EDITION_HUAWEI
        || p_st->edition == edr::edition_t::EDITION_EDR)
    {
        pthread_spin_lock(&p_st->net_monitor_lock);
        if (ring_buffer_push(p_st->httpes, p_info))
        {
            http_info_free(p_info);
        }
        pthread_spin_unlock(&p_st->net_monitor_lock);
    }
}

static void on_dns(dns_info_t *p_info, void *p_user_data)
{
    MainStatistics *p_st = (MainStatistics *)p_user_data;

    debug_print("%s %s\n", __func__, p_info->p_dns_query);

    if (p_st->edition == edr::edition_t::EDITION_HUAWEI
        || p_st->edition == edr::edition_t::EDITION_EDR)
    {
        if (p_st->edition == edr::edition_t::EDITION_HUAWEI)
        {
            if (match_rule(p_st, OBJ_DNS, p_info) == false)
                return;
            sysinfo_log(p_st->p_m_info, "%s [%s] match rule\n", __func__, p_info->p_dns_query);
            if (p_st->p_processor->upload_file(p_info->pid))
            {
                huawei_push_task_file(p_st, p_info->pid);
            }
        }
        pthread_spin_lock(&p_st->net_monitor_lock);
        if (ring_buffer_push(p_st->dnses, p_info))
        {
            dns_info_free(p_info);
        }
        pthread_spin_unlock(&p_st->net_monitor_lock);
    }
}

static void on_icmp(icmp_info_t *p_info, void *p_user_data)
{
    MainStatistics *p_st = (MainStatistics *)p_user_data;

    if (p_st->edition == edr::edition_t::EDITION_HUAWEI)
    {
        if (match_rule(p_st, OBJ_ICMP, p_info) == false)
            return;
        sysinfo_log(p_st->p_m_info, "%s ICMP match rule\n", __func__);
        if (p_st->p_processor->upload_file(p_info->pid))
        {
            huawei_push_task_file(p_st, p_info->pid);
        }

        pthread_spin_lock(&p_st->net_monitor_lock);
        if (ring_buffer_push(p_st->icmpes, p_info))
        {
            icmp_info_free(p_info);
        }
        pthread_spin_unlock(&p_st->net_monitor_lock);
    }
}

static void on_tcp(tcp_info_t *p_info, void *p_user_data)
{
    MainStatistics *p_st = (MainStatistics *)p_user_data;

    if (p_st->edition == edr::edition_t::EDITION_HUAWEI)
    {
#ifdef DEBUG
        char srcaddr[16] = {0};
        char dstaddr[16] = {0};
        inet_ntop(AF_INET, &p_info->ip_hdr.source_address, srcaddr, sizeof(srcaddr));
        inet_ntop(AF_INET, &p_info->ip_hdr.destination_address, dstaddr, sizeof(dstaddr));
        debug_print("on_TCP %s:%d -> %s:%d\n", srcaddr, ntohs(p_info->tcp_hdr.source_port),
            dstaddr, ntohs(p_info->tcp_hdr.destination_port));
#endif

        if (match_rule(p_st, OBJ_TCP, p_info) == false)
            return;
        sysinfo_log(p_st->p_m_info, "%s TCP match rule\n", __func__);
        if (p_st->p_processor->upload_file(p_info->pid))
        {
            huawei_push_task_file(p_st, p_info->pid);
        }

        pthread_spin_lock(&p_st->net_monitor_lock);
        if (ring_buffer_push(p_st->tcpes, p_info))
        {
            tcp_info_free(p_info);
        }
        pthread_spin_unlock(&p_st->net_monitor_lock);
    }
}

static void on_udp(udp_info_t *p_info, void *p_user_data)
{
    MainStatistics *p_st = (MainStatistics *)p_user_data;

    if (p_st->edition == edr::edition_t::EDITION_HUAWEI)
    {
        if (match_rule(p_st, OBJ_UDP, p_info) == false)
            return;
        sysinfo_log(p_st->p_m_info, "%s UDP match rule\n", __func__);
        if (p_st->p_processor->upload_file(p_info->pid))
        {
            huawei_push_task_file(p_st, p_info->pid);
        }

        pthread_spin_lock(&p_st->net_monitor_lock);
        if (ring_buffer_push(p_st->udpes, p_info))
        {
            udp_info_free(p_info);
        }
        pthread_spin_unlock(&p_st->net_monitor_lock);
    }
}

static void on_proc_action(proc_event_type_t event, size_t nmemb, Proc **procs, void *p_user_data)
{
    MainStatistics *p_st = (MainStatistics *)p_user_data;

    for (size_t i = 0; i<nmemb; i++)
    {
        Proc *proc = procs[i];
        if (proc == NULL)
        {
            continue;
        }

        proc->comparing_state = event;
        if (ring_buffer_push(p_st->proc_actions, proc))
        {
            INFO_PRINT("ring_buffer_push failed\n");
            proc_free(proc);
        }
        debug_print("%s %10d --%8s--> %10d %s\n", 
                __func__, 
                (int)proc->ppid, 
                event == PROC_ACTION_CREATE ? "create" : "destroy", 
                proc->pid, 
                proc->abs_name.size() ? proc->abs_name.data() : proc->cmd);
    }
}

static void on_snap_shot(size_t nmemb, Proc **procs, void *p_user_data)
{
    MainStatistics *p_st = (MainStatistics *)p_user_data;

    ps_t *p_ps = ps_load(nmemb, procs);
    if (!p_ps)
    {
        INFO_PRINT("ps_load failed\n");
        return;
    }

    if (ring_buffer_push(p_st->snap_shots, p_ps))
    {
        INFO_PRINT("ring_buffer_push failed\n");
        ps_destroy(p_ps);
    }

    debug_print("%s %lu process\n", __func__, nmemb);
}

static void on_proc_monitor_action(proc_event_type_t event_type, size_t nmemb, Proc **procs, void *p_user_data)
{
    switch (event_type)
    {
        case PROC_SNAPSHOT:
            on_snap_shot(nmemb, procs, p_user_data);
            break;
        case PROC_ACTION_CREATE:
        case PROC_ACTION_DESTROY:
            on_proc_action(event_type, nmemb, procs, p_user_data);
            break;
        default:
            for (size_t i = 0; i<nmemb; i++)
            {
                proc_free(procs[i]);
            }
            break;
    }
}

static edr::edition_t get_edition(module_info_t *p_m_info)
{
    const char *val = conf_get_value(p_m_info->p_pargs, p_m_info->arg_count, CONF_EDITION);
    assert(val != NULL);

    if (strcasecmp(val, "EDR") == 0)
    {
        return edr::edition_t::EDITION_EDR;
    }
    else if (strcasecmp(val, "HUAWEI") == 0)
    {
        return edr::edition_t::EDITION_HUAWEI;
    }
    assert(0);
    return edr::edition_t::EDITION_NO;
}

int load_configuration(MainStatistics *p_st, module_info_t *p_m_info)
{
    // TODO: load statis config here, not in interface
    p_st->edition = get_edition(p_m_info);
    return 0;
}

int create_rule(MainStatistics *p_st)
{
    p_st->p_processor = ProcessorFactory().create_processor(p_st->edition);
    if (p_st->edition == edr::edition_t::EDITION_HUAWEI)
    {

        const char *wlist_path = conf_get_value(p_st->p_m_info->p_pargs, p_st->p_m_info->arg_count, CONF_WHITE_LIST_PATH);
        const char *rule_path = conf_get_value(p_st->p_m_info->p_pargs, p_st->p_m_info->arg_count, CONF_RULE_PATH);
        p_st->p_rules = p_st->p_processor->load_rules(wlist_path, rule_path);
        if (p_st->p_rules == NULL)
        {
            sysinfo_log(p_st->p_m_info, "%s load_rules failed\n", __func__);
            return -1;
        }
    }
    else
    {
        p_st->p_rules = NULL;
    }
    return 0;
}

int create_uploader(MainStatistics *p_st)
{
    edr::UploaderConf conf;

    conf.p_m_info = p_st->p_m_info;
    if (p_st->edition == edr::edition_t::EDITION_HUAWEI)
    {
        if (p_st->p_processor)
        {
            conf.log_path = p_st->p_processor->get_log_path(p_st->p_rules);
            p_st->p_processor->get_ftp_login(p_st->p_rules, conf.ip, conf.user, conf.passwd);
        }
        conf.port = 21;
    }
    else if (p_st->edition == edr::edition_t::EDITION_EDR)
    {
    }

    p_st->p_upload = edr::UploaderFactory().create_uploader(p_st->edition, &conf);

    return 0;
}

MainStatistics *sys_info_statis_create(module_info_t *p_m_info)
{
    MainStatistics *p_st = new MainStatistics;
    if (!p_st) return NULL;

    p_st->p_m_info = p_m_info;

    load_configuration(p_st, p_m_info);


    if (create_rule(p_st) < 0)
    {
        sys_info_statis_destroy(p_st);
        return NULL;
    }
    create_uploader(p_st);

    p_st->ps_prev = NULL;
    p_st->ps_curr = NULL;
    p_st->conn_prev = NULL;
    p_st->conn_curr = NULL;
    p_st->sys_stat_prev = NULL;
    p_st->sys_stat_curr = NULL;

    p_st->snap_shots = ring_buffer_create(4096);
    if (!p_st->snap_shots)
    {
        sys_info_statis_destroy(p_st);
        return NULL;
    }

    sys_info_set_statis_delta(p_st, 3);
    snap_shot_set_statis_delta(p_st, 3, 3, 3);

    p_st->proc_actions = ring_buffer_create(4096);
    if (!p_st->proc_actions)
    {
        sys_info_statis_destroy(p_st);
        return NULL;
    }

    proc_action_set_statis_delta(p_st, 30);

    p_st->sys_tasks = ring_buffer_create(4096);
    if (!p_st->sys_tasks)
    {
        sys_info_statis_destroy(p_st);
        return NULL;
    }
    p_st->usb_actions = ring_buffer_create(4096);
    if (!p_st->usb_actions)
    {
        sys_info_statis_destroy(p_st);
        return NULL;
    }
    p_st->file_actions = ring_buffer_create(4096);
    if (!p_st->file_actions)
    {
        sys_info_statis_destroy(p_st);
        return NULL;
    }

    pthread_spin_init(&p_st->net_monitor_lock, PTHREAD_PROCESS_PRIVATE);
    p_st->httpes = ring_buffer_create(4096);
    if (!p_st->httpes)
    {
        sys_info_statis_destroy(p_st);
        return NULL;
    }
    p_st->dnses = ring_buffer_create(4096);
    if (!p_st->dnses)
    {
        sys_info_statis_destroy(p_st);
        return NULL;
    }
    p_st->icmpes = ring_buffer_create(4096);
    if (!p_st->icmpes)
    {
        sys_info_statis_destroy(p_st);
        return NULL;
    }
    p_st->tcpes = ring_buffer_create(4096);
    if (!p_st->tcpes)
    {
        sys_info_statis_destroy(p_st);
        return NULL;
    }
    p_st->udpes = ring_buffer_create(4096);
    if (!p_st->udpes)
    {
        sys_info_statis_destroy(p_st);
        return NULL;
    }
    p_st->md5_cache = create_file_digest_cache("./file_md5_cache.db");
    if (!p_st->md5_cache)
    {
        sys_info_statis_destroy(p_st);
        return NULL;
    }

    p_st->p_users_passwd = get_users_passwd();
    if (!p_st->p_users_passwd)
    {
        sys_info_statis_destroy(p_st);
        return NULL;
    }

    const char *val;
    val = conf_get_value(p_m_info->p_pargs, p_m_info->arg_count, "enable_proc_monitor");
    int enable_proc_monitor = conf_value_is_true(val);
    if (val == NULL)
    {
        enable_proc_monitor = 1;
    }
    val = conf_get_value(p_m_info->p_pargs, p_m_info->arg_count, "enable_net_monitor");
    int enable_net_monitor = conf_value_is_true(val);
    if (val == NULL)
    {
        enable_net_monitor = 1;
    }

    if (enable_proc_monitor)
    {
        int32_t enable_snapshot = 1;
        if (p_st->edition == edr::edition_t::EDITION_HUAWEI)
        {
            enable_snapshot = 0;
        }
        size_t snapshot_delta = 3;
        const char *proc_options[][2] = { 
            { "snapshot_delta", (const char *)&snapshot_delta },
            { "enable_snapshot", (const char *)&enable_snapshot}};
        p_st->p_proc_monitor = init_proc_monitor(on_proc_monitor_action, p_st, sizeof(proc_options) / sizeof(proc_options[0]), proc_options);
        if (!p_st->p_proc_monitor)
        {
            sys_info_statis_destroy(p_st);
            return NULL;
        }
    }
    else
    {
        p_st->p_proc_monitor = NULL;
    }

    if (enable_net_monitor)
    {
        // typedef void (*http_monitor_cb_t)(http_info_t *p_info, void *cb_param);
        // typedef void (*dns_monitor_cb_t)(dns_info_t *p_info, void *cb_param);
        // typedef void (*tcp_monitor_cb_t)(tcp_info_t *p_info, void *cb_param);
        // typedef void (*udp_monitor_cb_t)(udp_info_t *p_info, void *cb_param);
        // typedef void (*icmp_monitor_cb_t)(icmp_info_t *p_info, void *cb_param);

        // int net_monitor_set_http(net_monitor_t *p_monitor, http_monitor_cb_t cb, void *cb_param);
        // int net_monitor_set_dns(net_monitor_t *p_monitor, dns_monitor_cb_t cb, void *cb_param);
        // int net_monitor_set_tcp(net_monitor_t *p_monitor, tcp_monitor_cb_t cb, void *cb_param);
        // int net_monitor_set_udp(net_monitor_t *p_monitor, udp_monitor_cb_t cb, void *cb_param);
        // int net_monitor_set_icmp(net_monitor_t *p_monitor, icmp_monitor_cb_t cb, void *cb_param);
        p_st->p_net_monitor = init_net_monitor();
        if (!p_st->p_net_monitor)
        {
            sys_info_statis_destroy(p_st);
            return NULL;
        }
        if (p_st->edition == edr::edition_t::EDITION_HUAWEI
            || p_st->edition == edr::edition_t::EDITION_EDR)
        {
            net_monitor_set_http(p_st->p_net_monitor, on_http, p_st);
            net_monitor_set_dns(p_st->p_net_monitor, on_dns, p_st);
        }
        if (p_st->edition == edr::edition_t::EDITION_HUAWEI)
        {
            net_monitor_set_icmp(p_st->p_net_monitor, on_icmp, p_st);
            net_monitor_set_tcp(p_st->p_net_monitor, on_tcp, p_st);
            net_monitor_set_udp(p_st->p_net_monitor, on_udp, p_st);
        }
    }
    else
    {
        p_st->p_net_monitor = NULL;
    }

    p_st->white_list = white_list::get_instance();

    sys_info_statis_set_local_ip(p_st, "0.0.0.0");
    white_list_set_log_switch(p_st, 0);
    network_set_log_switch(p_st, 0);
    return p_st;
}

static void *proc_monitor_loop(void *argu)
{
    MainStatistics *p_st = (MainStatistics *)argu;
    run_proc_monitor(p_st->p_proc_monitor);
    return NULL;
}

static void *net_monitor_loop(void *argu)
{
    MainStatistics *p_st = (MainStatistics *)argu;
    run_net_monitor(p_st->p_net_monitor);
    return NULL;
}

static void *upload_loop(void *argu)
{
    MainStatistics *p_st = (MainStatistics *)argu;
    if (p_st && p_st->p_upload)
    {
        p_st->p_upload->run();
    }
    return NULL;
}

int sys_info_statis_start(MainStatistics *p_st)
{
    p_st->running = 1;

    p_st->snap_shot_host_time_prev = p_st->snap_shot_proc_time_prev = p_st->snap_shot_net_time_prev = 0;
    p_st->proc_action_time_prev = 0;
    p_st->sys_info_time_prev = 0;
    // p_st->network_time_prev = 0;
    p_st->ps_curr = p_st->ps_prev = NULL;
    p_st->conn_curr = p_st->conn_prev = NULL;

    int err = 0;
    err = pthread_create(&p_st->net_monitor_pid, NULL, net_monitor_loop, p_st);
    if (err)
    {
        return -1;
    }

    err = pthread_create(&p_st->proc_monitor_pid, NULL, proc_monitor_loop, p_st);
    if (err)
    {
        return -1;
    }

    err = pthread_create(&p_st->upload_pid, NULL, upload_loop, p_st);
    if (err)
    {
        return -1;
    }

    err = pthread_create(&p_st->main_pid, NULL, sys_info_statis_loop, p_st);
    if (err)
    {
        return -1;
    }
    return 0;
}
