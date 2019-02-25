#ifndef __EDR_UPLOAD_STRUCT_H__
#define __EDR_UPLOAD_STRUCT_H__

#include <stdint.h>
#include <string>
#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SYS_UPLOAD_LOG_TYPE_SNAP_SHOT   1
#define SYS_UPLOAD_LOG_TYPE_PROC_ACTION 2

#define SYS_UPLOAD_LOG_TYPE_MD5_LOG     1
#define SYS_UPLOAD_LOG_TYPE_PROC_FILE   2
#define SYS_UPLOAD_LOG_TYPE_MD5_FILE    3
#define SYS_UPLOAD_LOG_TYPE_ACTION_RULE 4

struct json_file {
    std::string json_name;
    std::string zip_name;
    std::string json_str;
    std::string url_type;
    int type;
};

struct task_record {
    // text is a line endof '\n', and all elements separate by ' '
    std::string data;
    uint32_t    data_size;

    // attribute
    std::string type;
    int         upload_type;    // SYS_UPLOAD_LOG_TYPE_xxx
    // int        msg_id;
};

#ifdef __cplusplus
}
#endif

#endif // __EDR_UPLOAD_STRUCT_H__
