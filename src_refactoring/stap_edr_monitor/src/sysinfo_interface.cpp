#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "sysinfo_logger.h"
#include "sysinfo_statistics.h"
#include "sysinfo_interface.h"
#include "module_data.h"

struct module {
    module_info_t   info;
    MainStatistics *statiser;
    const char    **pp_inputted_message_types;
    uint32_t        p_message_type_count;
};

static const char *action_types[] = {
    // for token
    "receive_token",
    // for sys task
    "CheckByMD5",
    "CheckByIOC",
    "SetSnapTime",
    "CheckByConnection",
    "SetActionRule",
    // for usb monitor
    "UStorgae_Event",
    // for file monitor
    "MSG_TYPE_VIRUSSCAN",
};

SYSINFO_PUBLIC_API void destroy(module_t *p_module)
{
    if(!p_module)    return;

    if(p_module->pp_inputted_message_types) free(p_module->pp_inputted_message_types);
    if(p_module->statiser)   sys_info_statis_destroy(p_module->statiser);
    free(p_module);
}

SYSINFO_PUBLIC_API module_state_t stop(module_t *p_module)
{
    if(!p_module)    return MODULE_ERROR;

    sys_info_statis_stop(p_module->statiser);

    return MODULE_OK;
}

SYSINFO_PUBLIC_API module_state_t run(module_t *p_module)
{
    if(!p_module)    return MODULE_ERROR;

    if(sys_info_statis_start(p_module->statiser) < 0)
    {
        stop(p_module);
        return MODULE_FATAL;
    }

    return MODULE_OK;
}

static const module_data_t **module_data_ptrs_alloc_impl(uint32_t count)
{
    return new const module_data_t *[count];
}

static void async_notify_outer_module(module_t *p_module, const module_data_t **results, int result_count, const char *data, uint32_t data_len, int is_sync)
{
    // write data into handle
    module_data_t *p_data = create_module_data();
    if(!p_data)   return ;
    set_module_data_property(p_data, "CENTER_MESSAGE_TASK_RESULT", 
            data, data_len);

    // set notifier attribution
    module_message_t response_handle;
    response_handle.category = (module_category_t)(p_module->info.category | 1);
    response_handle.p_data = p_data;

    mdh_sync_params_t sync_data;
    sync_data.is_sync = is_sync;
    sync_data.ptrs_alloc = module_data_ptrs_alloc_impl;
    sync_data.result.pp_ptrs = NULL;
    sync_data.result.count = 0;

    sysinfo_log(&p_module->info, "%s result_count:%d, data_val:%d\n", __func__, result_count, *(int *)data);
    // notify other module
    p_module->info.notifier(&response_handle, p_module->info.p_params, &sync_data);
    if(sync_data.result.pp_ptrs)
        delete []sync_data.result.pp_ptrs;
    // destroy handle
    destroy_module_data(p_data);
}

SYSINFO_PUBLIC_API module_data_t *assign(module_t *p_module, const module_data_t *p_data, bool is_sync)
{
    if(!p_module)    return NULL;

    const char *buffer;
    uint32_t buffer_size;
    int err_code = -1;
    err_code = get_module_data_property(p_data, g_p_message_id, &buffer, &buffer_size);
    if(0 != err_code)
    {
        return NULL;
    }
    // sysinfo_log(&p_module->info, "%s: get_module_data_property(%s) -> %s\n", __func__, g_p_message_id, buffer);
    if(0 == strcmp("receive_token", buffer))
    {
        err_code = get_module_data_property(p_data, "TOKEN", &buffer, &buffer_size);
        if(0 == err_code)
        {
            sysinfo_log(&p_module->info, "%s: set token :%s\n", __func__, buffer);
            set_server_host_port_token(p_module->statiser, NULL, 0, buffer);
        }
        else
        {
            sysinfo_log(&p_module->info, "%s: get_module_data_property(%s) failed\n", __func__, "TOKEN");
        }
    }
    else if( 0 == strcmp("CheckByMD5", buffer)
            || 0 == strcmp("CheckByIOC", buffer)
            || 0 == strcmp("SetSnapTime", buffer)
            || 0 == strcmp("CheckByConnection", buffer)
            || 0 == strcmp("SetActionRule", buffer))
    {
        // sysinfo_log(&p_module->info, "%s: recieve cmd:%s\n", __func__, buffer);
        const char *params;
        uint32_t params_size;
        err_code = get_module_data_property(p_data, "TASK_PARAMS", &params, &params_size);
        if(0 == err_code)
        {
            // sysinfo_log(&p_module->info, "%s: recieve params:%s\n", __func__, params);
            sys_task_t *task = (sys_task_t *)calloc(1, sizeof(sys_task_t));
            if(task)
            {
                task->cmd = strdup(buffer);
                task->cmd_size = buffer_size;
                task->params = strdup(params);
                task->params_size = params_size;
                if(sys_task_push(p_module->statiser, task))
                {
                    sysinfo_log(&p_module->info, "%s: sys_task_push failed\n", __func__);
                    sys_task_free(task);
                }
            }
        }
        else
        {
            sysinfo_log(&p_module->info, "%s: get_module_data_property(%s) failed\n", __func__, "TASK_PARAMS");
        }
        if(!is_sync)
        {
            // sysinfo_log(&p_module->info, "%s: async notify the result:%d to peer\n", __func__, err_code);
            async_notify_outer_module(p_module, NULL, 1, (const char *)&err_code, sizeof(err_code), 0);
        }
        else
        {
            // sysinfo_log(&p_module->info, "%s: sync notify the result:%d to peer\n", __func__, err_code);
            module_data_t *response_data = create_module_data();
            if(!response_data) return NULL;
            set_module_data_property(response_data, "CENTER_MESSAGE_TASK_RESULT", (const char *)&err_code, sizeof(err_code));
            return response_data;
        }
    }
    else if(0 == strcmp("UStorgae_Event", buffer))
    {
        const char *num_str;
        uint32_t num_str_size;
        err_code = get_module_data_property(p_data, "UStorgae_Event_Num", &num_str, &num_str_size);
        if(0 == err_code)
        {
            int num = *(int *)num_str;
            // sysinfo_log(&p_module->info, "%s: recieve[%s] num:%d\n", __func__, "USBSTORAGE_EVENT", num);
            while(num--)
            {
                const char *path, *flag;
                uint32_t path_size, flag_size;
                err_code = get_module_data_property(p_data, "UStorage_Flag", &flag, &flag_size);
                if(0 != err_code)
                {
                    sysinfo_log(&p_module->info, "%s: get_module_data_property(%s) failed\n", __func__, "UStorage_Flag");
                    break;
                }
                err_code = get_module_data_property(p_data, "UStorage_Data", &path, &path_size);
                if(0 != err_code)
                {
                    sysinfo_log(&p_module->info, "%s: get_module_data_property(%s) failed\n", __func__, "UStorage_Data");
                    break;
                }
                // push usb action to statiser
                usb_action_t *action = (usb_action_t *)calloc(1, sizeof(usb_action_t));
                if(action)
                {
                    strncpy(action->path, path, sizeof(action->path));
                    action->state = *(int *)flag;
                    if(0 == usb_action_push(p_module->statiser, action))
                    {
                        err_code = 0;
                    }
                    else
                    {
                        err_code = 1;
                        sysinfo_log(&p_module->info, "%s: usb_action_push failed\n", __func__);
                        usb_action_free(action);
                    }
                }
            } // end while(num--)
        }
        else
        {
            sysinfo_log(&p_module->info, "%s: get_module_data_property(%s) failed\n", __func__, "TASK_PARAMS");
        }
    }
    else if(0 == strcmp("MSG_TYPE_VIRUSSCAN", buffer))
    {
        // this is a file action
        const char *json_str;
        uint32_t json_str_size;
        err_code = get_module_data_property(p_data, "MONITOR_MESSAGE_EVENT", &json_str, &json_str_size); 
        if(0 == err_code)
        {
            // push file action to statiser
            file_action_t *action = (file_action_t *)calloc(1, sizeof(file_action_t));
            if(action)
            {
                action->json_str = strdup(json_str);
                action->json_str_size = json_str_size;
                if(0 == file_action_push(p_module->statiser, action))
                {
                    err_code = 0;
                }
                else
                {
                    err_code = 1;
                    sysinfo_log(&p_module->info, "%s: file_action_push failed\n", __func__);
                    file_action_free(action);
                }
            }
        }
        else
        {
            sysinfo_log(&p_module->info, "%s: get_module_data_property(%s) failed\n", __func__, "MONITOR_MESSAGE_EVENT");
        }
    }
    else
    {
        sysinfo_log(&p_module->info, "%s: get_module_data_property(%s) -> [%s] unrecognized\n", __func__, g_p_message_id, buffer);
    }
    return NULL;
}

static int get_local_ip(const char *host, int port, uint32_t *ip)
{
    int socketfd;
    struct sockaddr_in sockaddr;

    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0)
    {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        char szbuf[200] = {0};
        sprintf(szbuf, "create socket error: %s (errno: %d)\n", strerror(errno), errno);
        return -1;
    }
    int mode = fcntl(socketfd, F_GETFD);
    fcntl(socketfd, F_SETFD, mode|O_NONBLOCK);
    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    inet_pton(AF_INET, host, &sockaddr.sin_addr);
    int tmout = 1000;
    setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, &tmout, sizeof(tmout));
    setsockopt(socketfd, SOL_SOCKET, SO_SNDTIMEO, &tmout, sizeof(tmout));
    if ((connect(socketfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr))) < 0)
    {
        printf("connect error: %s(errno: %d)\n", strerror(errno), errno);
        char szbuf[200] = {0};
        sprintf(szbuf, "create socket error: %s (errno: %d)\n", strerror(errno), errno);
        close(socketfd);
        return -1;
    }

    sockaddr_in my_addr;
    socklen_t addr_len = sizeof(sockaddr_in);
    getsockname(socketfd, (struct sockaddr*)&my_addr,&addr_len);

    *ip = my_addr.sin_addr.s_addr; 

    close(socketfd);  
    return 0;  
}

static int p_pargs_value_is_true(const char *value)
{
    int ret = 0;
    if(!value)
        ret = 0;
    else if(strcasestr(value, "yes"))
        ret = 1;
    else if(strcasestr(value, "enable"))
        ret = 1;
    else if(atoi(value) > 0)
        ret = 1;
    else if(strcasestr(value, "no"))
        ret = 0;
    else if(strcasestr(value, "disable"))
        ret = 0;
    else 
        ret = 0;

    return ret;
}
static char *p_pargs_get_value_by_key(const char **p_pargs, uint32_t arg_count, const char *key)
{
    for(uint32_t i=0; i<arg_count; i++)
    {
        if(NULL != strcasestr(p_pargs[i], key))
        {
            char *value = (char *)strchr(p_pargs[i], ':');
            if(value)
            {
                return value+1;
            }
        }
    }
    return NULL;
}

SYSINFO_PUBLIC_API module_t *create(uint32_t category, notify_scheduler_t notifier, 
        void *p_params, uint32_t arg_count, const char** p_pargs)
{
    module_t *p_module = (module_t *)calloc(1, sizeof(module_t));
    if(!p_module)   return NULL;
    p_module->pp_inputted_message_types = (const char **)calloc(sizeof(action_types)/sizeof(action_types[0]), sizeof(const char *));
    if(!p_module->pp_inputted_message_types)
    {
        destroy(p_module);
        return NULL;
    }

    p_module->p_message_type_count = sizeof(action_types)/sizeof(action_types[0]);
    for(uint32_t i=0; i<p_module->p_message_type_count; i++)
    {
        p_module->pp_inputted_message_types[i] = action_types[i];
    }

    p_module->info.category = category;
    p_module->info.notifier = notifier;
    p_module->info.p_params = p_params;
    p_module->info.arg_count= arg_count;
    p_module->info.p_pargs  = p_pargs;

    p_module->statiser = sys_info_statis_create(&p_module->info);
    if(!p_module->statiser)
    {
        destroy(p_module);
        return NULL;
    }

    const char *host = p_pargs_get_value_by_key(p_pargs, arg_count, "host");
    const char *port = p_pargs_get_value_by_key(p_pargs, arg_count, "port");
    if(host && port)
    {
        set_server_host_port_token(p_module->statiser, host, (uint16_t)atoi(port), NULL);
        INFO_PRINT("set_server_host_port(%s:%d)\n", host, atoi(port));
        uint32_t local_ip = 0;
        // if(0 == get_local_ip(host, atoi(port), &local_ip))
        // {
        //     char ip_str[32] = {0};
        //     inet_ntop(AF_INET, &local_ip, ip_str, sizeof(ip_str));
        //     sys_info_statis_set_local_ip(p_module->statiser, ip_str);
        // }
    }

    const char *sysinfo_delta = p_pargs_get_value_by_key(p_pargs, arg_count, "sysinfo_delta");
    if(sysinfo_delta)
    {
        sys_info_set_statis_delta(p_module->statiser, atoi(sysinfo_delta));
        INFO_PRINT("sys_info_set_statis_delta(%d)\n", atoi(sysinfo_delta));
    }
    const char *proc_action_delta = p_pargs_get_value_by_key(p_pargs, arg_count, "proc_action_delta");
    if(proc_action_delta)
    {
        proc_action_set_statis_delta(p_module->statiser, atoi(proc_action_delta));
        INFO_PRINT("proc_action_set_statis_delta(%d)\n", atoi(proc_action_delta));
    }
    // const char *network_delta = p_pargs_get_value_by_key(p_pargs, arg_count, "network_delta");
    // if(network_delta)
    // {
    //     network_set_delta(p_module->statiser, atoi(network_delta));
    //     INFO_PRINT("network_set_delta(%d)\n", atoi(network_delta));
    // }
    const char *file_white = p_pargs_get_value_by_key(p_pargs, arg_count, "file_white");
    const char *process_white = p_pargs_get_value_by_key(p_pargs, arg_count, "process_white");
    const char *module_white = p_pargs_get_value_by_key(p_pargs, arg_count, "module_white");
    const char *host_white = p_pargs_get_value_by_key(p_pargs, arg_count, "host_white");
    white_list_set_config_file(p_module->statiser, file_white, process_white, module_white, host_white);

    const char *enable_json_file = p_pargs_get_value_by_key(p_pargs, arg_count, "enable_json_file");
    // TODO : enable upload store json file

    const char *enable_white_list_log = p_pargs_get_value_by_key(p_pargs, arg_count, "enable_white_list_log");
    white_list_set_log_switch(p_module->statiser, p_pargs_value_is_true(enable_white_list_log));

    const char *enable_network_log = p_pargs_get_value_by_key(p_pargs, arg_count, "enable_network_log");
    network_set_log_switch(p_module->statiser, p_pargs_value_is_true(enable_network_log));

    sysinfo_log(&p_module->info, "interface %s: end\n", __func__);

    return p_module;
}

SYSINFO_PUBLIC_API void get_inputted_message_type(module_t *p_module, 
        const char *** const ppp_inputted_message_types, uint32_t *p_message_type_count)
{
    *ppp_inputted_message_types = p_module->pp_inputted_message_types;
    *p_message_type_count = p_module->p_message_type_count;
}

