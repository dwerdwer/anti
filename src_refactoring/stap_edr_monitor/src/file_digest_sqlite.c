
#include "file_digest_sqlite.h"
#include "sqlite_cmd.h"
#include "debug_print.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <openssl/md5.h>
#include <sys/stat.h>
#include <unistd.h>

#define READ_SECTION_LEN 512 

#define TAB_NAME "file_digest_tab"

#define TAB_ELEMENTS "\
    id INTEGER PRIMARY KEY AUTOINCREMENT,\
    md5 varchar(32) DEFAULT NULL,\
    file_path varchar(255) DEFAULT NULL,\
    file_size int(11) DEFAULT NULL,\
    created_at time DEFAULT NULL" 

typedef struct
{
    sqlite_t *p_sqlite;
    file_digest_t file_digest;
    const char *tab_name;

}cache_data_t;

static int get_element_cb(void *cb_param, int arg_count, char **p_pargs, char **p_propertys)
{
    if(arg_count > 0)
    {
        file_digest_t **pp_file_digest = (file_digest_t**)cb_param;

        *pp_file_digest = (file_digest_t*)malloc(sizeof(file_digest_t));
        if (NULL == *pp_file_digest)
            return -1;

        memset(*pp_file_digest, 0, sizeof(file_digest_t));

        int i = 0;
        for (i = 0; i < arg_count; i++)
        {
            if(!strcmp("md5", p_propertys[i]) && (NULL != p_pargs[i]))
            {
                strncpy((*pp_file_digest)->md5, p_pargs[i], strlen(p_pargs[i]));
                continue;
            }
            if(!strcmp("file_path", p_propertys[i]) && (NULL != p_pargs[i]))
            {
                strncpy((*pp_file_digest)->file_path, p_pargs[i], strlen(p_pargs[i]));
                continue;
            }
            if(!strcmp("file_size", p_propertys[i]) && (NULL != p_pargs[i]))
            {
                (*pp_file_digest)->file_size = atoi(p_pargs[i]);
                continue;
            }
            if(!strcmp("created_at", p_propertys[i]) && (NULL != p_pargs[i]))
            {
                (*pp_file_digest)->created_at = atol(p_pargs[i]);
                continue;
            }
        }
        return 0;
    }
    return -1;
}

static int get_file_md5(const char *p_file_path, char *p_md5_str)
{
    FILE* rfp = NULL;

    if ((rfp = fopen(p_file_path, "rb")) == NULL)
    {
        return -1;
    }
    int read_len = READ_SECTION_LEN;
    char read_buf[READ_SECTION_LEN] = { 0 };
    char *tmp_buf = read_buf;
    unsigned char md5_digest[16] = { 0 };

    MD5_CTX md5_ctx;

    memset(&md5_ctx, 0, sizeof(MD5_CTX));

    if (MD5_Init(&md5_ctx) != 1)
    {
        fclose(rfp);
        return -1;
    }

    while(read_len > 0)
    {
        if ((read_len = fread(tmp_buf, 1, READ_SECTION_LEN, rfp)) == 0)
            break;

        if (MD5_Update(&md5_ctx, tmp_buf, read_len) != 1)
        {
            fclose(rfp);
            return -1;
        }
    }
    MD5_Final(md5_digest, &md5_ctx);

    /* p_md5_str must more than 32, if not,'\0' will cover the next member */ 
    uint8_t i = 0;
    for (i = 0; i < 16; i++)
        snprintf(p_md5_str + i * 2, 3,"%2.2x", md5_digest[i]);

    fclose(rfp);

    return 0;
}

/* return: succ p_cache_data fail NULL */
void *create_file_digest_cache(const char *p_db_path)
{
    cache_data_t *p_cache_data = (cache_data_t*)malloc(sizeof(cache_data_t));

    if(NULL == p_cache_data)
        return NULL;
    memset(p_cache_data, 0, sizeof(cache_data_t));
    p_cache_data->tab_name = TAB_NAME;

    sqlite_t *p_sqlite = NULL;
    if((p_sqlite = create_sqlite_connect
        (p_db_path, p_cache_data->tab_name, get_element_cb)) == NULL)
    {
        DEBUG_PRINT("create_sqlite_connect error\n");
        return NULL;
    }
    else
    {
        p_cache_data->p_sqlite = p_sqlite;

        /* set auto release disk space */
        sqlite_step_command(p_cache_data->p_sqlite, "PRAGMA auto_vacuum = 1;");
        
        char cmd_buf[MAX_CMD_LEN] = { 0 };
        
        snprintf(cmd_buf, MAX_CMD_LEN, "CREATE TABLE IF NOT EXISTS '%s' (%s);",
                 p_cache_data->tab_name, TAB_ELEMENTS);
        if (sqlite_command(p_sqlite, cmd_buf) != true)
            return NULL;
    }
    return (void*)p_cache_data;
}

static int access_sqlite(cache_data_t *p_cache, file_digest_t *p_file_digest)
{
    char cmd_buf[MAX_CMD_LEN] = { 0 };

    snprintf(cmd_buf, MAX_CMD_LEN, 
             "INSERT INTO '%s'(md5, file_path, file_size, created_at) \
             VALUES ('%s','%s', %lu, %ld);", 
             p_cache->tab_name, p_file_digest->md5, p_file_digest->file_path, 
             p_file_digest->file_size, p_file_digest->created_at);

    if (sqlite_command(p_cache->p_sqlite, cmd_buf) != true)
        return -1;

    return 0;
}

/* return: exist 1  not found 0   error -1*/
static int judge_eleinfo_state(cache_data_t *p_cache, const char *p_file_path) 
{
    if (NULL == p_cache || NULL == p_file_path)
        return -1;

    file_digest_t *p_file_digest = NULL;
    
    if (get_element_by_str(p_cache->p_sqlite, &p_file_digest, "file_path", p_file_path) != true)
        return 0;
    if(p_file_digest != NULL)
    {
        free(p_file_digest);
        return 1;
    }
    return 0;
}

int cache_to_file_digest(void *p_cache_data, const char *p_file_path)
{
    if (NULL == p_cache_data || NULL == p_file_path)
        return -1;
    int result = 0;

    cache_data_t *p_cache = (cache_data_t*)p_cache_data;

    if(judge_eleinfo_state(p_cache, p_file_path) == 1)
        return result;

    strncpy(p_cache->file_digest.file_path, p_file_path, strlen(p_file_path) + 1);

    struct stat file_stat = { 0 };

    if (stat(p_cache->file_digest.file_path, &file_stat) == -1)
    {
        DEBUG_PRINT("stat error\n");
        result = -1;
    }
    p_cache->file_digest.file_size = file_stat.st_size;
    p_cache->file_digest.created_at = file_stat.st_ctime; /* time of last status change */ 

    if (get_file_md5(p_cache->file_digest.file_path, p_cache->file_digest.md5) != 0)
        result = -1;

    if (access_sqlite(p_cache, &p_cache->file_digest) != 0)
        result = -1;
    return result;
}

file_digest_t *get_cache_ele(void *p_cache_data, int id)
{
    if (NULL == p_cache_data)
        return NULL;

    cache_data_t *p_cache = (cache_data_t*)p_cache_data;

    file_digest_t *p_file_digest = NULL;

    if (get_element_by_id(p_cache->p_sqlite, &p_file_digest, id) != true)
    {
        return NULL;
    }
    return p_file_digest;
}

file_digest_t *get_file_digest_by_md5(void *p_cache_data, const char *md5_str)
{
    if (NULL == p_cache_data || NULL == md5_str)
        return NULL;

    cache_data_t *p_cache = (cache_data_t*)p_cache_data;

    file_digest_t *p_file_digest = NULL;

    if (get_element_by_str(p_cache->p_sqlite, &p_file_digest, "md5", md5_str) != true)
    {
        return NULL;
    }
    return p_file_digest;
}

file_digest_t *get_file_digest_by_path(void *p_cache_data, const char *p_file_path)
{
    if (NULL == p_cache_data || NULL == p_file_path)
        return NULL;

    cache_data_t *p_cache = (cache_data_t*)p_cache_data;
    
    file_digest_t *p_file_digest = NULL;

    if (get_element_by_str(p_cache->p_sqlite, &p_file_digest, "file_path", p_file_path) != true)
    {
        return NULL;
    }
    return p_file_digest;
}

int delete_file_digest_by_md5(void *p_cache_data, const char *md5_str)
{
    if (NULL == p_cache_data || NULL == md5_str)
        return -1;

    cache_data_t *p_cache = (cache_data_t*)p_cache_data;

    if (delete_element_by_str(p_cache->p_sqlite, "md5", md5_str) != true)
        return -1;

    return 0;
}

int delete_file_digest_by_path(void *p_cache_data, const char *p_file_path)
{
    if (NULL == p_cache_data || NULL == p_file_path)
        return -1;

    cache_data_t *p_cache = (cache_data_t*)p_cache_data;

    if (delete_element_by_str(p_cache->p_sqlite, "file_path", p_file_path) != true)
        return -1;

    return 0;
}

int get_cache_file_digest_sum(void *p_cache_data)
{
    if (NULL == p_cache_data)
        return -1;

    cache_data_t *p_cache = (cache_data_t*)p_cache_data;

    int element_sum = 0;
    char cmd_buf[MAX_CMD_LEN] = { 0 };

    snprintf(cmd_buf, MAX_CMD_LEN, "SELECT * FROM '%s';", p_cache->tab_name);

    if ((element_sum = get_tab_element_num(p_cache->p_sqlite, cmd_buf)) == -1)
    {
        return -1;
    }
    return element_sum;
}

int get_file_digest_top_id(void *p_cache_data)
{
    if (NULL == p_cache_data)
        return -1;

    cache_data_t *p_cache = (cache_data_t*)p_cache_data;

    int top_id = 0;

    if ((top_id = (int)get_top_element_id(p_cache->p_sqlite)) == -1)
        return -1;

    return top_id;
}

void destroy_file_digest_cache(void *p_cache_data)
{
    if (p_cache_data)
    {
        cache_data_t *p_cache = (cache_data_t*)p_cache_data;
        destroy_sqlite_connect(p_cache->p_sqlite);

        free(p_cache_data);
        p_cache_data = NULL;
    }
}

