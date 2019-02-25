#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#include "utils/utils_library.h"
#include "module_data.h"
#include "module_interfaces.h"
#include "interface_monitor.h"

#include "debug_print.h"
#include "file_system_monitor.h"

struct module{
	uint32_t uCategory;
	file_system_monitor *pMonitor;
	notify_scheduler_t pNotify;
	void * pParams;

    std::string current_path;
    char current_dir[256];
};

const char * EventMessage[] = {
	"Event_NewDirectory",
	"Event_RemoveDirectory",
	"Event_NewFile",
	"Event_RemoveFile",
	"Event_ModifyFile",
	"Event_MoveFileOrDirectofy"
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

void event_cb(int event_type, int argc, const char *args[], void *p_data)
{
    module *p_module = (module *)p_data;
    const char *p_target_path = NULL;
    int action_type = -1;
    
    switch(event_type)
    {
    case (IN_CREATE | IN_ISDIR):
        {
            action_type = EVENT_NEW_DIR;
            p_target_path = args[0];
            debug_print("EVENT_NEW_DIR: %s\n", p_target_path);
            break;
        }
    case (IN_DELETE | IN_ISDIR):
        {
            action_type = EVENT_REMOVE_DIR;
            p_target_path = args[0];
            debug_print("EVENT_REMOVE_DIR: %s\n", p_target_path);
            break;
        }
    case IN_CREATE:
        {
            action_type = EVENT_NEW_FILE;
            p_target_path = args[0];
            debug_print("EVENT_NEW_FILE: %s\n", p_target_path);
            break;
        }
    case IN_DELETE:
        {
            action_type = EVENT_REMOVE_FILE;
            p_target_path = args[0];
            debug_print("EVENT_REMOVE_FILE: %s\n", p_target_path);
            break;
        }
    case IN_MODIFY:
        {
            action_type = EVENT_MODIFY_FILE;
            p_target_path = args[0];
            debug_print("EVENT_MODIFY_FILE: %s\n", p_target_path);
            break;
        }
    default:
        break;
    }
    /* exclude continuous change file */
    if(p_target_path && 0 != strcmp(p_module->current_path.c_str(), p_target_path))
        p_module->current_path = p_target_path;
    else 
        return;

    /* exclude current directory*/
    if(strstr(p_target_path, p_module->current_dir) != NULL)
        return;

    char jsonBuf[2048] = { 0 };

    long file_size = get_file_size(p_target_path);
    long update_time = get_systemtime();

    sprintf(jsonBuf, "{\"pid\":\" \",\"md5\":\" \",\"file_path\":\"%s\",\"file_size\":%ld,\"updated_at\":%ld,\"action\":%d,\"result\":%d,\"op_time\":%ld}",
            p_target_path, file_size, update_time, action_type, 1, update_time);

    if (p_data)
    {
        char tmp_cmd[] = "MSG_TYPE_VIRUSSCAN";

        module_data_t *p_data =  create_module_data();

        set_module_data_property(p_data, g_p_message_id, tmp_cmd, strlen(tmp_cmd));
        set_module_data_property(p_data, "MONITOR_MESSAGE_EVENT", (const char *)jsonBuf, strlen(jsonBuf));
        // for vc_client
        if(EVENT_NEW_FILE == action_type)
            set_module_data_property(p_data, "MONITOR_PATH_MESSAGE", p_target_path, strlen(p_target_path));
        //set_module_data_property(p_data, "MONITOR_TYPE_MESSAGE", (const char*)action_type, sizeof(action_type));

        module_message_t module_msg;
        module_msg.category = (module_category_t)p_module->uCategory;
        module_msg.p_data = p_data;

        mdh_sync_params_t syncData;
        syncData.is_sync = false;
        p_module->pNotify(&module_msg, p_module->pParams, &syncData);
        destroy_module_data(p_data);
    }
}

LIB_PUBLIC module_t * create(uint32_t category, notify_scheduler_t notifier, 
                             void *p_params, uint32_t arg_count, const char **p_args)
{
    module * p_module = new module;
    assert(p_module);
    p_module->pMonitor = new file_system_monitor;
    p_module->pNotify = notifier;
    p_module->pParams = p_params;
    p_module->uCategory = category;
    p_module->current_path = "";
    getcwd(p_module->current_dir, sizeof(p_module->current_dir));

    p_module->pMonitor->begin_monitor(arg_count, (char **)p_args, event_cb, (void *)p_module);

    return p_module;
}

LIB_PUBLIC void destroy(module_t *ptr)
{
    if (ptr) {
        module * p_module = (module *)ptr;
        delete p_module->pMonitor;
        p_module->pMonitor = NULL;
        delete p_module;
        p_module = NULL;
    }
}

LIB_PUBLIC void get_inputted_message_type(module_t *p_module, 
                const char *** const ppp_inputted_message_types, uint32_t *p_message_type_count)
{
    // Nothing to do
}

LIB_PUBLIC module_state_t run(module_t *ptr)
{
    module_state_t tmpState = MODULE_FATAL;
    if (ptr) {
        tmpState = MODULE_OK;
    }
    return tmpState;
}

LIB_PUBLIC module_state_t stop(module_t *ptr)
{
    module_state_t tmpState = MODULE_FATAL;
    if (ptr) {
        module * p_module = (module *)ptr;
        p_module->pMonitor->end_monitor();
        tmpState = MODULE_OK;
    }
    return tmpState;
}

LIB_PUBLIC module_data_t* assign(module_t *ptr, const module_data_t *p_data, bool is_sync)
{
    // Nothing to do
    return NULL;
}
