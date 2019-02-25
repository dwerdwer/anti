#ifndef _MODULE_INTERFACE_
#define _MODULE_INTERFACE_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

#include <time.h>

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

    int          pid;
    time_t       time;
} file_event_t;

typedef struct file_monitor file_monitor_t;

typedef bool (*event_callback_t)(int32_t event_count, file_event_t *p_events, void *p_usr_data);

void free_file_event(file_event_t *p_event);

file_monitor_t* init_file_monitor(event_callback_t callback, void *p_user_data,
    uint32_t option_count, const char *(*pp_options)[2]);

/* If success function is blocked */
bool run_file_monitor(file_monitor_t *p_file_monitor);

void stop_file_monitor(file_monitor_t *p_file_monitor);

void destroy_file_monitor(file_monitor_t *p_file_monitor);

/* fuck */
bool change_flag(file_monitor_t *p_file_monitor);


bool add_dir_to_watcher(file_monitor_t * p_file_monitor,const char * p_directory);

bool remove_dir_from_watcher(file_monitor_t * p_file_monitor,const char * p_directory);

bool add_to_white_list(file_monitor_t * p_file_monitor,const char * p_path);

bool remove_from_white_list(file_monitor_t * p_file_monitor,const char * p_path);

#ifdef __cplusplus
}
#endif
#endif
