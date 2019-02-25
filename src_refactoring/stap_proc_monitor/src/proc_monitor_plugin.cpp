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

#include "proc_monitor.h"
#include "proc_monitor_plugin.h"

#include "module_data.h"
#include "module_interfaces.h"

#include "utils/utils_library.h"
#include "debug_print.h"
#include "rpcsrv_interface.h"

struct module
{
	uint32_t category;
    pthread_t thread_id;
    proc_monitor_t *p_monitor;
    notify_scheduler_t pNotify;
	void * pParams;
};

static void proc_monitor_callback_impl(proc_event_type_t event_type, size_t nmemb, Proc **procs, void *p_user_data)
{
    // TODO
    for(size_t i = 0; i < nmemb; i++)
    {
        proc_free(procs[i]);
    }
}

static const char * EventMessage[] = {
    msg_req_cmd.c_str(),
};

static const module_data_t **module_data_ptrs_alloc_impl(uint32_t count)
{
    return new const module_data_t *[count];
}

LIB_PUBLIC module_t * create(uint32_t category, notify_scheduler_t notifier,
                             void *p_params, uint32_t arg_count, const char **p_args)
{
    printf("proc_monitor_plugin %s\n", __func__);
    module *p_module = new module;

    p_module->p_monitor = init_proc_monitor(proc_monitor_callback_impl, p_module, 0, NULL);

    if (NULL == p_module->p_monitor) {
        delete p_module;
        return NULL;
    }

    p_module->thread_id = 0;
    p_module->pNotify = notifier;
    p_module->pParams = p_params;
    p_module->category = category;

    return p_module;
}

LIB_PUBLIC void destroy(module_t *ptr)
{
    printf("proc_monitor_plugin %s\n", __func__);
    if (ptr) {
        module * p_module = (module *)ptr;

        if (p_module->thread_id != 0)
        {
            pthread_join(p_module->thread_id, NULL);
            p_module->thread_id = 0;
        }

        fin_proc_monitor(p_module->p_monitor);
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
    run_proc_monitor(p_module->p_monitor);
    return NULL;
}

LIB_PUBLIC module_state_t run(module_t *ptr)
{
    printf("proc_monitor_plugin %s\n", __func__);
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
    printf("proc_monitor_plugin %s\n", __func__);
    module_state_t result = MODULE_FATAL;
    module * p_module = (module *)ptr;
    if (p_module)
    {
        if(p_module->thread_id != 0)
        {
			stop_proc_monitor(p_module->p_monitor);
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
    printf("proc_monitor_plugin %s\n", __func__);
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

static module_data_t *proc_monitor_assign_from_rpcsrv(module *p_module, const module_data_t *p_data, bool is_sync)
{
    printf("proc_monitor_plugin %s\n", __func__);
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
    case KV_ENGINE_MSG_TYPE_ENABLE_PROC_MONITOR:
        debug_print("%s type %" PRIu32 " == %" PRIu32 ", continue\n",
            __func__, (*(uint32_t *)type), KV_ENGINE_MSG_TYPE_ENABLE_PROC_MONITOR);
        break;
    default :
        debug_print("%s type %" PRIu32 " != %" PRIu32 ", ignore\n",
            __func__, (*(uint32_t *)type), KV_ENGINE_MSG_TYPE_ENABLE_PROC_MONITOR);
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
    printf("proc_monitor_plugin %s\n", __func__);
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
        return proc_monitor_assign_from_rpcsrv(p_module, p_data, is_sync);
    }
    return NULL;
}
