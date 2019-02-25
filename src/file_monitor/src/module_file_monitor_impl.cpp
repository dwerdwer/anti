#include <unistd.h>
#include <pthread.h>
#include <sys/inotify.h>

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <map>

#include "module_file_monitor_interface.h"
#include "file_system_monitor.h"
#include "debug_print.h"

#define PUBLIC_LIB __attribute__ ((visibility ("default")))

#define MAX_OPTSTR_LEN 256


struct file_monitor
{
    void *p_data;
    event_callback_t event_cb;

    file_system_monitor *p_monitor;
};

void event_proc(int event_type, int argc, const char *args[], void *p_data)
{
    if (NULL == p_data) {
        debug_print("p_data is empty\n");
        return;
    }
    file_monitor *p_file_monitor = (file_monitor*)p_data;
    if (p_file_monitor->event_cb == NULL) {
        debug_print("event_cb is empty\n");
        return;
    }
 
    file_event_t event;
    memset(&event, 0, sizeof(file_event_t));

    int flag = 0;
    
    switch(event_type)
    {
    case (IN_CREATE | IN_ISDIR):
            event.type = EVENT_NEW_DIR;
            event.p_dest_path = args[0];
            //event.p_src_path = "";
            flag = 1;
        break;
    case (IN_DELETE | IN_ISDIR):
            event.type = EVENT_REMOVE_DIR;
            event.p_dest_path = args[0];
            //event.p_src_path = "";
            flag = 1;
        break;
    case IN_MOVED_TO:
            //event.type = EVENT_MOVE_FILE_OR_DIRECTORY;
            //event.p_dest_path = args[0];
            //event.p_src_path = args[1];
            //flag = 1;
        break;
    case IN_CREATE:
            event.type = EVENT_NEW_FILE;
            event.p_dest_path = args[0];
            //event.p_src_path = "";
            flag = 1;
        break;
    case IN_DELETE:
            event.type = EVENT_REMOVE_FILE;
            event.p_dest_path = args[0];
            //event.p_src_path = "";
            flag = 1;
        break;
    case IN_MODIFY:
            event.type = EVENT_MODIFY_FILE;
            event.p_dest_path = args[0];
            //event.p_src_path = "";
            flag = 1;
        break;
    default:
        break;
    }
    if(event.p_dest_path == NULL) {
        return ;
    }

    if (flag && NULL != p_file_monitor->event_cb) {
        p_file_monitor->event_cb(1, &event, p_file_monitor->p_data);
    }
}

PUBLIC_LIB file_monitor_t *init_file_monitor(event_callback_t callback, 
                                void *p_user_data, uint32_t option_count, const char *(*pp_options)[2])
{
    if (NULL == pp_options || NULL == callback)
        return NULL;
    
    file_monitor *p_file_monitor = new file_monitor;
    assert(p_file_monitor);

    p_file_monitor->p_monitor = new file_system_monitor;
    assert(p_file_monitor->p_monitor);
    p_file_monitor->p_data = p_user_data;
    p_file_monitor->event_cb = callback;
   
    char **monitor_lists = (char**)calloc(option_count, sizeof(char*));
    size_t current_len = 0;

    for (uint32_t i = 0; i < option_count; i++)
    {
        monitor_lists[i] = (char*)calloc(MAX_OPTSTR_LEN, sizeof(char));

        if (NULL == pp_options[i][0] 
           || NULL == pp_options[i][1] || '/' != pp_options[i][1][0]) 
        {
            for (uint32_t j = 0; j <= i; j++)
                free(monitor_lists[j]);

            free(monitor_lists);
            goto ErrEnd; 
        }
        current_len = strlen(pp_options[i][1]);

        if ((strcmp(pp_options[i][0], "monitor_directory") == 0) && current_len < MAX_OPTSTR_LEN) 
        {
            if ( '/' == pp_options[i][1][current_len - 1] )
                strncpy(monitor_lists[i], pp_options[i][1], current_len - 1);
            else
                strncpy(monitor_lists[i], pp_options[i][1], current_len);
        }
    }
    p_file_monitor->p_monitor->begin_monitor(option_count, monitor_lists,
                                            event_proc, (void*)p_file_monitor);
    for (uint32_t i = 0; i < option_count; i++) {
        free(monitor_lists[i]);
    }

    free(monitor_lists);
    monitor_lists = NULL;

    debug_print("%s success\n", __func__);
    return p_file_monitor;
ErrEnd:
    debug_print("%s fail\n", __func__);
    uninit_file_monitor(p_file_monitor);
    return NULL;
}

PUBLIC_LIB bool add_dir_to_watcher(file_monitor_t *p_file_monitor, const char *p_directory)
{
    if (p_file_monitor && p_file_monitor->p_monitor && p_directory)
    {
        debug_print(">>> add directory to monitor!\n");
        if (0 == p_file_monitor->p_monitor->add_storage(p_directory, strlen(p_directory))) {
            debug_print(">>> add directory to monitor done!\n");
            return true;
        }
    }
    return false;
}

PUBLIC_LIB bool remove_dir_from_watcher(file_monitor_t *p_file_monitor,const char *p_directory)
{
    if (p_file_monitor && p_file_monitor->p_monitor && p_directory)
    {
        debug_print(">>> remove directory from monitor!\n");
        if (0 == p_file_monitor->p_monitor->remove_storage(p_directory, strlen(p_directory))) {
            debug_print(">>> remove directory from monitor done!\n");
            return true;
        }
    }
    return false;
}

PUBLIC_LIB bool add_to_white_list(file_monitor_t * p_file_monitor,const char * p_path)
{
    if (p_file_monitor && p_file_monitor->p_monitor && p_path)
    {
        debug_print(">>> add file or directory to white list!\n");
        if (0 == p_file_monitor->p_monitor->add_to_whitelist(p_path, strlen(p_path))) {
            debug_print(">>> add file or directory to white list done!\n");
            return true;
        }
    }
    return false;
}

PUBLIC_LIB bool remove_from_white_list(file_monitor_t *p_file_monitor, const char *p_path)
{
    if (p_file_monitor && p_file_monitor->p_monitor && p_path)
    {
        debug_print(">>> remove file or directory from white list!\n");
        if ( 0 == p_file_monitor->p_monitor->remove_from_whitelist(p_path, strlen(p_path))) {
            debug_print(">>> remove file or directory from white list done!\n");
            return true;
        }
    }
    return false;
}

PUBLIC_LIB void uninit_file_monitor(file_monitor_t *p_file_monitor)
{
    if (p_file_monitor) {
        if (p_file_monitor->p_monitor) {
            p_file_monitor->p_monitor->end_monitor();
            delete p_file_monitor->p_monitor;
        }
        delete p_file_monitor;
    }
}
