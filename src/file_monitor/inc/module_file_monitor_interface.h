#ifndef _MODULE_INTERFACE_
#define _MODULE_INTERFACE_

// #define STAP_CODE

#include <stdint.h>

#ifdef STAP_CODE
#include <time.h>
#endif /* STAP_CODE */

typedef enum 
{
    EVENT_NEW_DIR = 0,
	EVENT_REMOVE_DIR = 1,
	EVENT_NEW_FILE = 2,
	EVENT_REMOVE_FILE = 3,
	EVENT_MODIFY_FILE = 4,
} event_type_t;

typedef struct file_event
{
	event_type_t type;
	const char  *p_dest_path;

#ifdef STAP_CODE
    int          pid;
    time_t       time;
#endif /* STAP_CODE */
} file_event_t;

typedef struct file_monitor file_monitor_t;

typedef bool (*event_callback_t)(int32_t event_count, file_event_t *p_events, void *p_usr_data);

#ifdef __cplusplus
extern "C"
{
#endif
file_monitor_t * init_file_monitor(event_callback_t callback, void *p_user_data, 
                                   uint32_t option_count, const char *(*pp_options)[2]);

void uninit_file_monitor(file_monitor_t * p_file_monitor);

bool add_dir_to_watcher(file_monitor_t * p_file_monitor,const char * p_directory);

bool remove_dir_from_watcher(file_monitor_t * p_file_monitor,const char * p_directory);

bool add_to_white_list(file_monitor_t * p_file_monitor,const char * p_path);

bool remove_from_white_list(file_monitor_t * p_file_monitor,const char * p_path);

#ifdef __cplusplus
}
#endif
#endif
