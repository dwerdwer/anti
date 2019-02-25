
#include "cache_to_sqlite.h"
#include "sqlite_cmd.h"
#include "debug_print.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <sys/stat.h>
#include <unistd.h>
#include <string>

#define TAB_NAME "json_str_tab"

#define TAB_ELEMENTS "\
    id INTEGER PRIMARY KEY AUTOINCREMENT,\
    json_str TEXT DEFAULT NULL"

typedef struct
{
    char *json_str;
    size_t json_len;

}json_info_t;

struct json_cache 
{
    sqlite_t *p_sqlite;
    json_info_t json_info;
    std::string db_path;
    std::string tab_name;
};

static int get_element_cb(void *cb_param, int arg_count, char **p_pargs, char **p_propertys)
{
    if (arg_count > 0 && cb_param)
    {
        json_info_t *p_json_info = (json_info_t*)cb_param;
        
        p_json_info->json_len = 0;

        int i = 0;
        for (i = 0; i < arg_count; i++)
        {
            if(!strcmp("json_str", p_propertys[i]) && (NULL != p_pargs[i]))
            {
                p_json_info->json_len = strlen(p_pargs[i]);
                p_json_info->json_str = (char*)malloc(p_json_info->json_len + 1);

                memset(p_json_info->json_str, 0, p_json_info->json_len + 1);
                
                if(NULL == p_json_info->json_str)
                    break;
                strncpy(p_json_info->json_str, p_pargs[i], p_json_info->json_len);

                break;
            }
        }
        return 0;
    }
    return -1;
}


json_cache_t *create_jsonstr_cache(const char *p_db_path)
{
    json_cache_t *p_cache = new json_cache_t;

    if(NULL == p_cache)
        return NULL;
    
    p_cache->tab_name = TAB_NAME;
    p_cache->db_path = p_db_path;

    if((p_cache->p_sqlite = create_sqlite_connect
        (p_db_path, p_cache->tab_name.c_str(), get_element_cb)) == NULL)
    {
        goto ErrEnd;
    }
    else {
        /* set auto release disk space */
        sqlite_step_command(p_cache->p_sqlite, "PRAGMA auto_vacuum = 1;");

        char cmd_buf[MAX_CMD_LEN] = { 0 };

        snprintf(cmd_buf, MAX_CMD_LEN, "CREATE TABLE IF NOT EXISTS '%s' (%s);",
                 p_cache->tab_name.c_str(), TAB_ELEMENTS);
        if (sqlite_command(p_cache->p_sqlite, cmd_buf) != true)
            goto ErrEnd;
    }
    return p_cache;
ErrEnd:
    destroy_jsonstr_cache(p_cache);
    return NULL;
}

int set_jsonstr_to_cache(json_cache_t *p_cache, const char *json_str)
{
    if (insert_text_to_tab(p_cache->p_sqlite, "json_str", json_str) != true)
        return -1;

    return 0;
}

char *get_jsonstr_by_id(json_cache_t *p_cache, int64_t id, size_t *json_len)
{
    if (NULL == p_cache || id <= 0)
        return NULL;

    *json_len = 0;

    if (get_element_by_id(p_cache->p_sqlite, &p_cache->json_info, id) != true)
        return NULL;

    if(0 == p_cache->json_info.json_len)
    {
        if(p_cache->json_info.json_str)
        {
            free(p_cache->json_info.json_str);
            p_cache->json_info.json_str = NULL; 
        }
    }
    *json_len = p_cache->json_info.json_len;

    return p_cache->json_info.json_str;
}

int delete_jsonstr_by_id(json_cache_t *p_cache, int64_t id)
{
    if (NULL == p_cache || id <= 0)
        return -1;

    if (delete_element_by_id(p_cache->p_sqlite, id) != true)
        return -1;

    return 0;
}

int get_cache_ele_sum(json_cache_t *p_cache )
{
    if (NULL == p_cache)
        return -1;

    char cmd_buf[MAX_CMD_LEN] = { 0 };

    snprintf(cmd_buf, MAX_CMD_LEN, "SELECT * FROM '%s';", p_cache->tab_name.c_str());

    return get_tab_element_num(p_cache->p_sqlite, cmd_buf);
}

int64_t get_jsonstr_cache_top_id(json_cache_t *p_cache)
{
    if (NULL == p_cache)
        return -1;

    return get_top_element_id(p_cache->p_sqlite);
}

void destroy_jsonstr_cache(json_cache_t *p_cache)
{
    if (p_cache)
    {
        destroy_sqlite_connect(p_cache->p_sqlite);
       
        remove(p_cache->db_path.c_str());
        delete p_cache;
        p_cache = NULL;
    }
}

