#pragma once

extern "C" {

#include <stdint.h>

typedef struct kv_engine kv_engine_t;

// return: non-NULL if success.
kv_engine_t *initialize_engine(void);

// p_engine: p_engine is returned by initialize_engine, otherwise program may crash.
// if it is NULL, program definitely crash.
void finalize_engine(kv_engine_t *p_engine);

typedef enum monitor_type
{
    MONITOR_TYPE_FILE = 1 << 0,
    MONITOR_TYPE_NETWORK = 1 << 1,
    MONITOR_TYPE_PROCESS = 1 << 2,
}monitor_type_t;

// p_engine: p_engine is returned by initialize_engine, otherwise program may crash.
// if it is NULL, program definitely crash.
// type: or-bitwise of monitor_type_t
// return: 0 if all operations success, otherwise return the or-bitwise of failed monitor_type_t.
int32_t enable_monitor(kv_engine_t *p_engine, monitor_type_t type);

// similar to enable_monitor
int32_t disable_monitor(kv_engine_t *p_engine, monitor_type_t type);

// p_engine: p_engine is returned by initialize_engine, otherwise program may crash.
// if it is NULL, program definitely crash.
// p_count: out param, the number of asset_t array.
// pp_assets: out param, asset_t array
// return: 0 if success; otherwise non-0.
typedef struct asset
{
	const char *p_name;
	const char *p_description;
}asset_t;
int32_t get_assets_info(kv_engine_t *p_engine, uint32_t *p_count, const asset_t **pp_assets);

// p_engine: p_engine is returned by initialize_engine, otherwise program may crash.
// if it is NULL, program definitely crash.
// notifier: Callback function, if any message arrive, it will be called.
// p_param: Use parameter of callback function
// return: 0 if success; otherwise non-0.
typedef void (*notifier_t)(const char *data, size_t datalen, void *p_param);
int32_t set_notifier(kv_engine_t *p_engine, notifier_t notifier, void *p_param);


typedef enum
{
    UPDATE_TYPE_ALL = 1 << 0,
    UPDATE_TYPE_VIRUS_LIB = 1 << 1,
    UPDATE_TYPE_PROGRAM = 1 << 2,
}update_type_t;

int upgrade_daemon(kv_engine_t *p_engine,
                    update_type_t type, uint32_t file_count, const char **pp_files);

}
