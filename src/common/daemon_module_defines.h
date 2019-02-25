
#ifndef DAEMON_MODULE_DEFINES_HHH
#define DAEMON_MODULE_DEFINES_HHH

#include <stdint.h>

// 'dm' is short for "daemon module".
typedef enum
{
    DAEMON_MODULE_OK = 0,
    DAEMON_MODULE_ERROR = -1,
}dm_state_t;

typedef struct daemon_module daemon_module_t;

typedef daemon_module_t *(*dm_creator_t)(uint32_t arg_count, const char **p_args);
typedef void (*dm_destroyer_t)(daemon_module_t *p_daemon_module);

typedef dm_state_t (*dm_runner_t)(daemon_module_t *p_daemon_module);
typedef dm_state_t (*dm_stopper_t)(daemon_module_t *p_daemon_module);

#endif

