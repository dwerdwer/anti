#ifndef __EDR_MODULE_ACTIONS_H__
#define __EDR_MODULE_ACTIONS_H__

#include "defs.h"
#include "util-queue.h"
#include <stdint.h>

// #include "USBMonitorDef.h"

typedef struct file_action_ {
    char *json_str;
    uint32_t json_str_size;
} file_action_t;

typedef struct usb_action_ {
    uint64_t pid;
    char proc_name[MAX_LEN_ABSOLUTE_PATH_X_FILE_NAME];
    time_t pid_time;

    char path[MAX_LEN_ABSOLUTE_PATH_X_FILE_NAME];
    int state;
    TAILQ_ENTRY(usb_action_) next;
} usb_action_t;

#endif // __EDR_MODULE_ACTIONS_H__