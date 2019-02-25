
#include "sqlite_cmd.h" 
#include "debug_print.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

typedef struct sqlite_context_
{
    sqlite3 *context;
    sqlite_cb_t sqlite_cb;
    char cmd_buf[MAX_CMD_LEN];
    char tab_name[MAX_CMD_LEN / 2];

}sqlite_context_t;

void *create_sqlite_connect(const char *p_db_path, sqlite_cb_t sqlite_cb, const char *tab_name)
{
    sqlite_context_t *sqlite_context = (sqlite_context_t*)malloc(sizeof(sqlite_context_t));

    if (NULL == sqlite_context)
    {
        DEBUG_PRINT("malloc error\n");
        return NULL;
    }
    memset(sqlite_context, 0, sizeof(sqlite_context_t));
    sqlite_context->sqlite_cb = sqlite_cb;

    if (tab_name != NULL)
        strncpy(sqlite_context->tab_name, tab_name, strlen(tab_name));

    if(sqlite3_open(p_db_path, &sqlite_context->context) != 0)
    {
        DEBUG_PRINT("Can't open database: %s\n", sqlite3_errmsg(sqlite_context->context));
        if (sqlite_context)
            free(sqlite_context);
        return NULL;
    }
    return sqlite_context;
}

int sqlite_command(void *sqlite_ptr, const char *cmd_str)
{
    if (NULL == sqlite_ptr || NULL == cmd_str)
        return -1;
    
    sqlite_context_t *sqlite_context = (sqlite_context_t*)sqlite_ptr;

    char *p_error = NULL;

    if(sqlite3_exec(sqlite_context->context, cmd_str, NULL, NULL, &p_error) != SQLITE_OK)
    {
        DEBUG_PRINT("sqlite3_exec error: %s\n", p_error);
        if (p_error)
            sqlite3_free(p_error);
        return -1;
    }
    return 0;
}

int get_tab_element_num(void *sqlite_ptr, const char *cmd_str)
{
    if (NULL == sqlite_ptr || NULL == cmd_str)
        return -1;
    
    sqlite_context_t *sqlite_context = (sqlite_context_t*)sqlite_ptr;

    char **db_result = NULL;
    int elements_count = 0;

    if (sqlite3_get_table(sqlite_context->context, 
                          cmd_str, &db_result, &elements_count, NULL, NULL) != SQLITE_OK)
        return -1;

    if(db_result != NULL)
        sqlite3_free_table(db_result);

    return elements_count;
}

static int get_top_index_cb(void *cb_param, int arg_count, char **p_pargs, char **p_propertys)
{
    if (cb_param)
    {
        int *p_id = (int*)cb_param;

        int i = 0;
        for (i = 0; i < arg_count; i++)
        {
            if(!strcmp("id", p_propertys[i]) && (NULL != p_pargs[i]))
            {
                *p_id =  atoi(p_pargs[i]); 
                break;
            }
        }
    }
    return 0;
}

int get_top_element_id(void *sqlite_ptr)
{
    if (NULL == sqlite_ptr)
        return -1;
    
    sqlite_context_t *sqlite_context = (sqlite_context_t*)sqlite_ptr;

    snprintf(sqlite_context->cmd_buf, MAX_CMD_LEN, 
             "SELECT * FROM '%s' LIMIT 1;", sqlite_context->tab_name);

    int id = 0;
    char * p_error = NULL;
    if(sqlite3_exec(sqlite_context->context, sqlite_context->cmd_buf, 
                   get_top_index_cb, &id, &p_error) != SQLITE_OK)
    {
        DEBUG_PRINT("sqlite3_exec error: %s\n", p_error);
        if (p_error)
            sqlite3_free(p_error);
        return -1;
    }
    return id;
}

int get_element_by_id(void *sqlite_ptr, void *p_argument, int id)
{
    if (NULL == sqlite_ptr)
        return -1;
    
    sqlite_context_t *sqlite_context = (sqlite_context_t*)sqlite_ptr;

    snprintf(sqlite_context->cmd_buf, MAX_CMD_LEN, 
             "SELECT * FROM '%s' WHERE id = %d;", sqlite_context->tab_name, id);

    char * p_error = NULL;
    if(sqlite3_exec(sqlite_context->context, sqlite_context->cmd_buf, 
                    sqlite_context->sqlite_cb, p_argument, &p_error) != SQLITE_OK)
    {
        DEBUG_PRINT("sqlite3_exec error: %s\n", p_error);
        if (p_error)
            sqlite3_free(p_error);
        return -1;
    }
    return 0;
}

int get_element_by_str(void *sqlite_ptr, void *p_argument, const char *member, const char *value_str)
{
    if (NULL == sqlite_ptr)
        return -1;
    
    sqlite_context_t *sqlite_context = (sqlite_context_t*)sqlite_ptr;

    snprintf(sqlite_context->cmd_buf, MAX_CMD_LEN, 
             "SELECT * FROM '%s' WHERE %s ='%s';", sqlite_context->tab_name, member, value_str);

    char * p_error = NULL;
    if(sqlite3_exec(sqlite_context->context, sqlite_context->cmd_buf, 
                    sqlite_context->sqlite_cb, p_argument, &p_error) != SQLITE_OK)
    {
        DEBUG_PRINT("sqlite3_exec error: %s\n", p_error);
        if (p_error)
            sqlite3_free(p_error);
        return -1;
    }
    return 0;
}

int delete_element_by_id(void *sqlite_ptr, int id)
{
    if (NULL == sqlite_ptr)
    {
        DEBUG_PRINT("parameter error\n");
        return -1;
    }
    sqlite_context_t *sqlite_context = (sqlite_context_t*)sqlite_ptr;

    snprintf(sqlite_context->cmd_buf, MAX_CMD_LEN,
             "DELETE FROM '%s' WHERE id = %d;", sqlite_context->tab_name, id);

    char * p_error = NULL;
    if( sqlite3_exec(sqlite_context->context, sqlite_context->cmd_buf, NULL, NULL, &p_error) != SQLITE_OK )
    {
        DEBUG_PRINT("sqlite3_exec error: %s\n", p_error);
        if (p_error)
            sqlite3_free(p_error);
        return -1;
    }
    return 0;
}

int delete_element_by_str(void *sqlite_ptr, const char *member, const char *value_str)
{
    if (NULL == sqlite_ptr)
    {
        DEBUG_PRINT("parameter error\n");
        return -1;
    }
    sqlite_context_t *sqlite_context = (sqlite_context_t*)sqlite_ptr;

    snprintf(sqlite_context->cmd_buf, MAX_CMD_LEN,
             "DELETE FROM '%s' WHERE %s = '%s';", sqlite_context->tab_name, member, value_str);

    char * p_error = NULL;
    if( sqlite3_exec(sqlite_context->context, sqlite_context->cmd_buf, NULL, NULL, &p_error) != SQLITE_OK )
    {
        DEBUG_PRINT("sqlite3_exec error: %s\n", p_error);
        if (p_error)
            sqlite3_free(p_error);
        return -1;
    }
    return 0;
}

int inster_text_to_tab(void *sqlite_ptr, const char *member, const char *text_value)
{
    if (NULL == sqlite_ptr)
    {
        DEBUG_PRINT("parameter error\n");
        return -1;
    }
    sqlite_context_t *sqlite_context = (sqlite_context_t*)sqlite_ptr;

    char *sql_handle = sqlite3_mprintf("INSERT INTO '%s' (%Q) VALUES(%Q)", sqlite_context->tab_name, member, text_value);
    char *p_error = NULL;
    
    if(sqlite3_exec(sqlite_context->context, sql_handle, NULL, NULL, &p_error) != SQLITE_OK)  
    {                                                                                   
        DEBUG_PRINT("sqlite3_exec error: %s\n", p_error);                               
        if (p_error)                                                                    
            sqlite3_free(p_error);                                                      
        return -1;
    }
    if (NULL != sql_handle)
        sqlite3_free(sql_handle);
    
    return 0;
}

void destroy_sqlite_connect(void *sqlite_ptr)
{
    if (sqlite_ptr)
    {
        sqlite_context_t *sqlite_context = (sqlite_context_t*)sqlite_ptr;

        if (sqlite_context->context)
            sqlite3_close(sqlite_context->context);

        sqlite_context->context = NULL;

        free(sqlite_context);
        sqlite_context = NULL;
    }
}

