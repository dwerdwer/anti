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

#include "net_monitor.h"
#include "net_monitor_plugin.h"

#include "module_data.h"
#include "module_interfaces.h"

#include "utils/utils_library.h"
#include "debug_print.h"
#include "rpcsrv_interface.h"


struct module
{
	uint32_t category;
    pthread_t thread_id;
    net_monitor_t *p_monitor;
    notify_scheduler_t pNotify;
	void * pParams;
};

static const module_data_t **module_data_ptrs_alloc_impl(uint32_t count)
{
    return new const module_data_t *[count];
}

static void http_monitor_cb_impl(http_info_t *p_info, void *p_user_data)
{
    // TODO
    http_info_free(p_info);
}

static void dns_monitor_cb_impl(dns_info_t *p_info, void *p_user_data)
{
    // TODO
    dns_info_free(p_info);
}

static void tcp_monitor_cb_impl(tcp_info_t *p_info, void *p_user_data)
{
    // TODO
    tcp_info_free(p_info);
}

static void udp_monitor_cb_impl(udp_info_t *p_info, void *p_user_data)
{
    // TODO
    udp_info_free(p_info);
}

static void icmp_monitor_cb_impl(icmp_info_t *p_info, void *p_user_data)
{
    // TODO
    icmp_info_free(p_info);
}

static const char * EventMessage[] = {
    msg_req_cmd.c_str(),
};

LIB_PUBLIC module_t * create(uint32_t category, notify_scheduler_t notifier,
                             void *p_params, uint32_t arg_count, const char **p_args)
{
    printf("net_monitor_plugin %s\n", __func__);
    module *p_module = new module;

    p_module->p_monitor = init_net_monitor();

    if (NULL == p_module->p_monitor) {
        delete p_module;
        return NULL;
    }
    net_monitor_set_http(p_module->p_monitor, http_monitor_cb_impl, p_module);
    net_monitor_set_dns(p_module->p_monitor, dns_monitor_cb_impl, p_module);
    net_monitor_set_tcp(p_module->p_monitor, tcp_monitor_cb_impl, p_module);
    net_monitor_set_udp(p_module->p_monitor, udp_monitor_cb_impl, p_module);
    net_monitor_set_icmp(p_module->p_monitor, icmp_monitor_cb_impl, p_module);

    p_module->thread_id = 0;
    p_module->pNotify = notifier;
    p_module->pParams = p_params;
    p_module->category = category;

    return p_module;
}

LIB_PUBLIC void destroy(module_t *ptr)
{
    printf("net_monitor_plugin %s\n", __func__);
    if (ptr) {
        module * p_module = (module *)ptr;

        if (p_module->thread_id != 0)
        {
            pthread_join(p_module->thread_id, NULL);
            p_module->thread_id = 0;
        }

        fin_net_monitor(p_module->p_monitor);
        p_module->p_monitor = NULL;
        delete p_module;
    }
}

LIB_PUBLIC void get_inputted_message_type(module_t *p_module,
                                          const char *** const ppp_inputted_message_types, uint32_t *p_message_type_count)
{
    *ppp_inputted_message_types = EventMessage;
    *p_message_type_count = sizeof(EventMessage) / sizeof(EventMessage[0]);
}

static void* thread_monitor_func(void *thread_param)
{
    module *p_module = (module*)thread_param;
    run_net_monitor(p_module->p_monitor);
    return NULL;
}

LIB_PUBLIC module_state_t run(module_t *ptr)
{
    printf("net_monitor_plugin %s\n", __func__);
    module_state_t result = MODULE_FATAL;
    module *p_module = (module*)ptr;
    if (p_module)
    {
        if(p_module->thread_id == 0)
        {
            pthread_create(&p_module->thread_id, NULL, thread_monitor_func, p_module);
            result = MODULE_OK;
        }
        else
        {
            result = MODULE_OK;
        }
    }
    return result;
}

LIB_PUBLIC module_state_t stop(module_t *ptr)
{
    printf("net_monitor_plugin %s\n", __func__);
    module_state_t result = MODULE_FATAL;
    module * p_module = (module *)ptr;
    if (p_module)
    {
        if(p_module->thread_id != 0)
        {
			stop_net_monitor(p_module->p_monitor);
            pthread_join(p_module->thread_id, NULL);
            p_module->thread_id = 0;
            result = MODULE_OK;
        }
        else
        {
            result = MODULE_OK;
        }
    }

    return result;
}

static void notify_scheduler_msg(module *p_module, uint32_t id, uint32_t type, const void *buff, size_t bufflen)
{
    printf("net_monitor_plugin %s\n", __func__);
    module_data_t *p_data =  create_module_data();

    set_module_data_property(p_data, g_p_message_id, msg_result_cmd.c_str(), msg_result_cmd.size());
    set_module_data_property(p_data, msg_id_key.c_str(), (const char *)&id, sizeof(id));
    set_module_data_property(p_data, msg_type_key.c_str(), (const char *)&type, sizeof(type));
    set_module_data_property(p_data, msg_payload_key.c_str(), (const char *)buff, bufflen);

    module_message_t module_msg;
    module_msg.category = (module_category_t)p_module->category;
    module_msg.p_data = p_data;

    mdh_sync_params_t sync_data;
    sync_data.is_sync = false;
    sync_data.ptrs_alloc = module_data_ptrs_alloc_impl;
    sync_data.result.pp_ptrs = NULL;
    sync_data.result.count = 0;

    p_module->pNotify(&module_msg, p_module->pParams, &sync_data);
    destroy_module_data(p_data);
}

static module_data_t *net_monitor_assign_from_rpcsrv(module *p_module, const module_data_t *p_data, bool is_sync)
{
    printf("net_monitor_plugin %s\n", __func__);
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
    case KV_ENGINE_MSG_TYPE_ENABLE_NET_MONITOR:
        debug_print("%s type %" PRIu32 " == %" PRIu32 ", continue\n",
            __func__, (*(uint32_t *)type), KV_ENGINE_MSG_TYPE_ENABLE_NET_MONITOR);
        break;
    default :
        debug_print("%s type %" PRIu32 " != %" PRIu32 ", ignore\n",
            __func__, (*(uint32_t *)type), KV_ENGINE_MSG_TYPE_ENABLE_NET_MONITOR);
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
        retv = (char)run(p_module);
    }
    else
    {
        retv = (char)stop(p_module);
    }

    notify_scheduler_msg(p_module, *(uint32_t *)id, *(uint32_t *)type, &retv, sizeof(retv));
    return NULL;
}

LIB_PUBLIC module_data_t* assign(module_t *ptr, const module_data_t *p_data, bool is_sync)
{
    printf("net_monitor_plugin %s\n", __func__);
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
        return net_monitor_assign_from_rpcsrv(p_module, p_data, is_sync);
    }
    return NULL;
}
