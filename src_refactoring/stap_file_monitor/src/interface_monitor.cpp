#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <inttypes.h>

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <string>
#include <ios>
#include <fstream>
#include <thread>
#include <mutex>
#include <queue>

#include "utils/utils_library.h"
#include "module_data.h"
#include "module_interfaces.h"
#include "interface_monitor.h"
#include "rpcsrv_interface.h"
#include "kv_engine_public.h"

#include "module_file_monitor_interface.h"

#include "debug_print.h"
#include "common_utils.h"
#include "json/json.h"

#define FILE_MONITOR_RULE "Monitor"
#define RULE_FILE_KEY "rule"


struct file_action
{
    int pid;
    std::string file_name;
    off_t size;
    time_t update_time;
    time_t op_time;
    int result;
    int action;
};

struct span_st
{
    enum { SPAN_ENABLE, SPAN_DISABLE, SPAN_STOP } state;
    time_t time_s;
};

struct module
{
	uint32_t category;
    pthread_t main_thread;
    file_monitor_t *p_monitor;
    notify_scheduler_t pNotify;
	void * pParams;

    bool enable_notify_avx;

    std::string configfile;
    Json::Value rule;
    std::thread config_thread;
    std::queue<span_st> span_queue;
    std::mutex span_lock;
    bool running;

    std::string cwd;
};

static bool local_to_file(const char *file, module *p_module)
{
    std::ofstream ofs(file);
    if(ofs)
    {
        std::string s = p_module->rule.toStyledString();
        ofs.write(s.c_str(), s.size());
    }
    return true;
}

static bool load_from_file(const char *file, module *p_module)
{
    Json::CharReaderBuilder crb;
    std::ifstream ifs(file);
    std::string errs;
    if(ifs)
    {
        if(parseFromStream(crb, ifs, &p_module->rule, &errs))
        {
            printf("%s(%s) -> \n%s\n",
                __func__, file, p_module->rule.toStyledString().c_str());
            return true;
        }
        debug_print("%s(%s) -> error:%s\n", __func__, file, errs.c_str());
    }
    return false;
}

static const module_data_t **module_data_ptrs_alloc_impl(uint32_t count)
{
    return new const module_data_t *[count];
}

static void notify_edr_event(module *p_module, file_action &info)
{
    char jsonBuf[2048] = { 0 };
    sprintf(jsonBuf, "{\"pid\":\"%d\",\"md5\":\" \",\"file_path\":\"%s\","
            "\"file_size\":%ld,\"updated_at\":%ld,\"action\":%d,"
            "\"result\":%d,\"op_time\":%ld}",
            info.pid, info.file_name.data(),
            info.size, info.update_time, info.action,
            info.result, info.op_time);
    printf("%s - %s\n", __func__, jsonBuf);

    module_data_t *p_data =  create_module_data();

    set_module_data_property(p_data, g_p_message_id, "MSG_TYPE_VIRUSSCAN", strlen("MSG_TYPE_VIRUSSCAN"));
    set_module_data_property(p_data, "MONITOR_MESSAGE_EVENT", (const char *)jsonBuf, strlen(jsonBuf));
    // for vc_client
    if(EVENT_NEW_FILE == info.action)
        set_module_data_property(p_data, "MONITOR_PATH_MESSAGE", info.file_name.data(), info.file_name.size());
    //set_module_data_property(p_data, "MONITOR_TYPE_MESSAGE", (const char*)info->action, sizeof(info->action));

    module_message_t module_msg;
    module_msg.category = (module_category_t)p_module->category;
    module_msg.p_data = p_data;

    mdh_sync_params_t sync_data;
    sync_data.is_sync = false;
    sync_data.ptrs_alloc = module_data_ptrs_alloc_impl;
    sync_data.result.pp_ptrs = NULL;
    sync_data.result.count = 0;

    p_module->pNotify(&module_msg, p_module->pParams, &sync_data);
    if(sync_data.result.pp_ptrs)
    {
        delete[] sync_data.result.pp_ptrs;
    }
    destroy_module_data(p_data);
}

static void notify_avscan_event(module *p_module, file_action &info)
{
    char jsonBuf[2048] = { 0 };
    sprintf(jsonBuf, "{\"pid\":\"%d\",\"file_path\":\"%s\"}",
        info.pid, info.file_name.data());

    printf("%s - %s\n", __func__, jsonBuf);

    module_data_t *p_data =  create_module_data();

    set_module_data_property(p_data, g_p_message_id, "MSG_TYPE_AVSCAN", strlen("MSG_TYPE_AVSCAN"));
    set_module_data_property(p_data, "AVSCAN_JSON", (const char *)jsonBuf, strlen(jsonBuf));

    module_message_t module_msg;
    module_msg.category = (module_category_t)p_module->category;
    module_msg.p_data = p_data;

    mdh_sync_params_t sync_data;
    sync_data.is_sync = false;
    sync_data.ptrs_alloc = module_data_ptrs_alloc_impl;
    sync_data.result.pp_ptrs = NULL;
    sync_data.result.count = 0;

    p_module->pNotify(&module_msg, p_module->pParams, &sync_data);
    if(sync_data.result.pp_ptrs)
    {
        delete[] sync_data.result.pp_ptrs;
    }
    destroy_module_data(p_data);
}

static const char * EventMessage[] = {
    "avsdk_to_file_monitor",
	"Event_NewDirectory",
	"Event_RemoveDirectory",
	"Event_NewFile",
	"Event_RemoveFile",
	"Event_ModifyFile",
	"Event_MoveFileOrDirectofy",
    "Monitor",
    msg_req_cmd.c_str(),
};

static long get_file_size(const char *p_path)
{
    struct stat st;
    if (stat(p_path, &st) == 0)
    {
        if (S_ISREG(st.st_mode))
            return (long)st.st_size;
    }
    return 0;
}


static long get_systemtime()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return now.tv_sec;
}

static bool file_monitor_cb_impl(int32_t event_count, file_event_t *p_events, void *p_data)
{
    module *p_module = (module *)p_data;

    const char *p_target_path = NULL;

    file_action info;

    for (int32_t i = 0; i < event_count; i++)
    {
        p_target_path = p_events[i].p_dest_path;

        /* exclude current work directory*/
        if(p_module->cwd.size() && p_module->cwd.compare(0, p_module->cwd.size(), p_target_path, p_module->cwd.size()) == 0)
        {
            printf("%s file[%s] in cwd, exclude it.\n", __func__, p_target_path);
            return true;
        }

        info.action = p_events[i].type;
        info.file_name = path_format(p_target_path);
        info.pid = p_events[i].pid;
        info.size = get_file_size(p_target_path);
        info.update_time = get_systemtime();
        info.op_time = info.update_time;
        info.result = 1;

        notify_edr_event(p_module, info);
        if(p_module->enable_notify_avx)
        {
            if(info.action == EVENT_NEW_FILE || info.action == EVENT_MODIFY_FILE)
            {
                notify_avscan_event(p_module, info);
            }
        }
    }
    free_file_event(p_events);
    return true;
}

static void disable_span(module *p_module)
{
    span_st span;
    span.state = span_st::SPAN_DISABLE;
    span.time_s = 0;
    printf("file_monitor disable span\n");
    std::lock_guard<std::mutex> lock(p_module->span_lock);
    p_module->span_queue.push(span);
}

static void enable_span(module *p_module, time_t delay)
{
    span_st span;
    span.state = span_st::SPAN_ENABLE;
    span.time_s = time(NULL) + delay;
    printf("file_monitor create span at time:%lu\n", span.time_s);
    std::lock_guard<std::mutex> lock(p_module->span_lock);
    p_module->span_queue.push(span);
}

static bool enable(module *p_module, bool force)
{
    p_module->enable_notify_avx = true;
    disable_span(p_module);
    return true;
}

static bool disable(module *p_module, bool force)
{
    if(force)
    {
        p_module->enable_notify_avx = false;
        return true;
    }
    if(p_module->rule.isNull())
    {
        p_module->enable_notify_avx = false;
        return true;
    }
    else
    {
        Json::Value v = p_module->rule["DisableSpan"];
        if(!v.isString())
        {
            printf("file_monitor DisableSpan is not a string , Json format error\n");
            return false;
        }
        /* Unit : minute */
        int var = std::stoi(v.asString()) * 60;
        if(var == 0)
        {
            /* forbidden from disable by other user */
            printf("file_monitor forbidden from disable by other user\n");
            return false;
        }
        else if(var == -1)
        {
            p_module->enable_notify_avx = false;
            disable_span(p_module);
            return true;
        }
        else if(var > 0)
        {
            p_module->enable_notify_avx = false;
            enable_span(p_module, (time_t)var);
            return true;
        }
    }
    return false;
}

static void span_thread_loop(module *p_module)
{
    time_t enable_time = 0;
    while(true)
    {
        usleep(50000);
        if(enable_time && time(NULL) >= enable_time)
        {
            printf("%s trigger time:%lu enable file_monitor\n", __func__, enable_time);
            enable(p_module, true);
            enable_time = 0;
        }

        std::lock_guard<std::mutex> lock(p_module->span_lock);
        if(p_module->span_queue.size() == 0)
        {
            continue;
        }
        span_st span = p_module->span_queue.front();
        p_module->span_queue.pop();
        if(span.state == span_st::SPAN_STOP)
        {
            break;
        }
        else if(span.state == span_st::SPAN_ENABLE)
        {
            enable_time = span.time_s;
        }
        else if(span.state == span_st::SPAN_DISABLE)
        {
            enable_time = 0;
        }
    }
    return ;
}

LIB_PUBLIC module_t * create(uint32_t category, notify_scheduler_t notifier,
                             void *p_params, uint32_t arg_count, const char **pp_args)
{
    printf("file_monitor_plugin %s\n", __func__);
    module *p_module = new module;

    uint32_t i = 0;
    const char *(*p_dirs)[2] = NULL;
    size_t dir_count = 0;
    if(arg_count)
    {
        p_dirs = new const char *[arg_count][2];
        for(i = 0; i < arg_count; i++) {
            printf("file_monitor_plugin %s\n", pp_args[i]);
            if(tok_key(pp_args[i], ":").compare(RULE_FILE_KEY) == 0)
            {
                p_module->configfile = tok_value(pp_args[i], ":");
                if(p_module->configfile.find_last_of("/") != std::string::npos)
                {
                    makedir_p(p_module->configfile.substr(0, p_module->configfile.find_last_of("/")), 0777);
                }
                load_from_file(p_module->configfile.c_str(), p_module);
            }
            else
            {
                p_dirs[dir_count][0] = "monitor_directory";
                p_dirs[dir_count][1] = pp_args[i];
                dir_count++;
            }
        }
    }

    p_module->p_monitor = init_file_monitor(file_monitor_cb_impl, (void*)p_module, dir_count, p_dirs);

    if(p_dirs)
    {
        delete [] p_dirs;
        p_dirs = NULL;
    }

    if (NULL == p_module->p_monitor) {
        delete p_module;
        return NULL;
    }
    p_module->main_thread = 0;
    // void span_thread_loop(module *);
    p_module->config_thread = std::thread (span_thread_loop, p_module);
    p_module->pNotify = notifier;
    p_module->pParams = p_params;
    p_module->category = category;
    p_module->enable_notify_avx = true;

    /* exclude self */
    char cwd[1024] = {0};
    getcwd(cwd, sizeof(cwd));
    if(strlen(cwd) > 1) /* cwd is not "/" */
    {
        p_module->cwd = path_format(strcat(cwd, "/../"));
        printf("%s file_monitor exclude cwd[%s]\n", __func__, p_module->cwd.c_str());
    }

    return p_module;
}

static void stop_span(module *p_module)
{
    span_st span;
    span.state = span_st::SPAN_STOP;
    printf("file_monitor stop span\n");
    std::lock_guard<std::mutex> lock(p_module->span_lock);
    p_module->span_queue.push(span);
}

LIB_PUBLIC void destroy(module_t *ptr)
{
    printf("file_monitor_plugin %s\n", __func__);
    if (ptr)
    {
        module * p_module = (module *)ptr;

        stop_span(p_module);
        p_module->config_thread.join();

        if (p_module->main_thread != 0)
        {
            pthread_cancel(p_module->main_thread);
            p_module->main_thread = 0;
        }

        destroy_file_monitor(p_module->p_monitor);
        p_module->p_monitor = NULL;

        delete p_module;
    }
}

LIB_PUBLIC void get_inputted_message_type(module_t *p_module,
                                          const char *** const ppp_inputted_message_types, uint32_t *p_message_type_count)
{
    printf("file_monitor_plugin %s\n", __func__);
    *ppp_inputted_message_types = EventMessage;
    *p_message_type_count = sizeof(EventMessage) / sizeof(EventMessage[0]);
    // Nothing to do
}

static void* thread_monitor_func(void *thread_param)
{
    module *p_module = (module*)thread_param;
    run_file_monitor(p_module->p_monitor);
    return NULL;
}

LIB_PUBLIC module_state_t run(module_t *ptr)
{
    printf("file_monitor_plugin %s\n", __func__);
    module_state_t result = MODULE_FATAL;
    module *p_module = (module*)ptr;
    if (p_module)
    {
        if(p_module->main_thread == 0)
        {
            printf("%s ok\n", __func__);
            pthread_create(&p_module->main_thread, NULL, thread_monitor_func, p_module);
            result = MODULE_OK;
        }
        else
        {
            printf("%s re-run\n", __func__);
            result = MODULE_OK;
        }
        enable(p_module, true);
    }
    return result;
}

LIB_PUBLIC module_state_t stop(module_t *ptr)
{
    printf("file_monitor_plugin %s\n", __func__);
    module_state_t result = MODULE_FATAL;
    module * p_module = (module *)ptr;
    if (p_module)
    {
        if(p_module->main_thread != 0)
        {
            stop_file_monitor(p_module->p_monitor);
            // pthread_join(p_module->main_thread, NULL);
            printf("file_monitor_plugin %s ok\n", __func__);
            p_module->main_thread = 0;
            result = MODULE_OK;
        }
        else
        {
            printf("file_monitor_plugin %s re-stop\n", __func__);
            result = MODULE_OK;
        }
        disable(p_module, true);
    }

    return result;
}

static void notify_rpcsrv(module *p_module, uint32_t id, uint32_t type, const void *buff, size_t bufflen)
{
    printf("file_monitor_plugin %s\n", __func__);
    module_data_t *p_data =  create_module_data();

    set_module_data_property(p_data, g_p_message_id, msg_result_cmd.c_str(), msg_result_cmd.size());
    set_module_data_property(p_data, msg_id_key.c_str(), (const char *)&id, sizeof(id));
    set_module_data_property(p_data, msg_type_key.c_str(), (const char *)&type, sizeof(type));
    set_module_data_property(p_data, msg_payload_key.c_str(), (const char *)buff, bufflen);

    module_message_t module_msg;
    module_msg.category = (module_category_t)(p_module->category | 1);
    module_msg.p_data = p_data;

    mdh_sync_params_t sync_data;
    sync_data.is_sync = false;
    sync_data.ptrs_alloc = module_data_ptrs_alloc_impl;
    sync_data.result.pp_ptrs = NULL;
    sync_data.result.count = 0;

    p_module->pNotify(&module_msg, p_module->pParams, &sync_data);
    if(sync_data.result.pp_ptrs)
    {
        delete[] sync_data.result.pp_ptrs;
    }
    destroy_module_data(p_data);
}

static module_data_t *assign_from_rpcsrv(module *p_module, const module_data_t *p_data, bool is_sync)
{
    printf("file_monitor_plugin %s\n", __func__);
    const char *type = NULL;
    uint32_t typesz = 0;
    int err = get_module_data_property(p_data, msg_type_key.c_str(), &type, &typesz);
    if(err || type == NULL)
    {
        debug_print("%s get type error\n",
            __func__);
        return NULL;
    }

    /* check type */
    switch(*(uint32_t *)type)
    {
    case KV_ENGINE_MSG_TYPE_ENABLE_FILE_MONITOR:
        debug_print("%s type %" PRIu32 " == %" PRIu32 ", continue\n",
            __func__, (*(uint32_t *)type), KV_ENGINE_MSG_TYPE_ENABLE_FILE_MONITOR);
        break;
    default :
        debug_print("%s type %" PRIu32 " != %" PRIu32 ", ignore\n",
            __func__, (*(uint32_t *)type), KV_ENGINE_MSG_TYPE_ENABLE_FILE_MONITOR);
        return NULL;
    }

    const char *id = NULL;
    uint32_t idsz = 0;
    err = get_module_data_property(p_data, msg_id_key.c_str(), &id, &idsz);
    if(err || id == NULL)
    {
        debug_print("%s type %" PRIu32 ", get id error\n",
            __func__, (*(uint32_t *)type));
        return NULL;
    }

    const char *payload = NULL;
    uint32_t payloadsz = 0;
    err = get_module_data_property(p_data, msg_payload_key.c_str(), &payload, &payloadsz);
    if(err)
    {
        return NULL;
    }

    char retv = 0;
    if(*payload)
    {
        retv = enable(p_module, false) ? 0 : -1;
    }
    else
    {
        retv = disable(p_module, false) ? 0 : -1;
    }

    notify_rpcsrv(p_module, *(uint32_t *)id, *(uint32_t *)type, &retv, sizeof(retv));
    printf("file_monitor_plugin %s end\n", __func__);
    return NULL;
}

static module_data_t *assign_from_center_agent(module_t *p_module, const module_data_t *p_data, bool is_sync)
{
    printf(__func__);
    const char *params = NULL;
    uint32_t params_sz = 0;
    int err = get_module_data_property(p_data, "TASK_PARAMS", &params, &params_sz);
    if(err != 0)
    {
        return NULL;
    }
    printf("%s receive task :\n%s\n", __func__, params);

    Json::CharReaderBuilder builder;
    Json::CharReader *reader = builder.newCharReader();
    Json::Value root;
    std::string errs;
    if(!reader->parse(params, params+params_sz, &root, &errs))
    {
        printf("%s receive task :\n%s\n parse error:%s", __func__, params, errs.c_str());
        return NULL;
    }
    p_module->rule = root;
    local_to_file(p_module->configfile.c_str(), p_module);

    Json::Value val = p_module->rule["File"];
    if(val.isString())
    {
        if(val.asString().compare("true") == 0
            || val.asString().compare("True") == 0
            || val.asString().compare("TRUE") == 0)
        {
            enable(p_module, false);
        }
        else
        {
            disable(p_module, false);
        }
    }

    delete reader;
    return NULL;
}

LIB_PUBLIC module_data_t* assign(module_t *ptr, const module_data_t *p_data, bool is_sync)
{
    printf("file_monitor_plugin %s\n", __func__);
    module *p_module = (module *)ptr;
    if(p_module == NULL)
    {
        return NULL;
    }
    const char *cmd = NULL;
    uint32_t cmdsz = 0;
    int err = get_module_data_property(p_data, g_p_message_id, &cmd, &cmdsz);
    if(err != 0)
    {
        return NULL;
    }
    if(strcmp(cmd, msg_req_cmd.c_str()) == 0)
    {
        return assign_from_rpcsrv(p_module, p_data, is_sync);
    }
    else if(strncmp(cmd, FILE_MONITOR_RULE, cmdsz) == 0)
    {
        return assign_from_center_agent(p_module, p_data, is_sync);
    }
    return NULL;
}
