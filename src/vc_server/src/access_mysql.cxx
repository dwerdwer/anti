
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>

#include "debug_print.h"
#include "access_mysql.h"

#define MYSQL_NAME_LEN          64      // mysql tab name length limit is 64 bit
#define MYSQL_DEFAULT_CMD_LEN   2048    // default command buffer size, more than 1024

struct mysql_connect
{
    MYSQL *p_mysql;
    char *cmd_buf;
    uint32_t cmd_buf_size;
};

struct query_resource
{
    MYSQL_RES *p_mysql_result;
    multi_result_t *multi_result_array; // multi query only
};

mysql_connect_t *create_mysql_connect(const char *host, 
            const char *user, const char *passwd, const char *db_name, uint16_t port)
{
    if(NULL == host || NULL == user || NULL == db_name)
        return NULL;

    mysql_connect_t *p_mysql_con = (mysql_connect_t*)calloc(1, sizeof(mysql_connect_t));
    if(NULL == p_mysql_con)
        return NULL;

    p_mysql_con->cmd_buf = (char*)calloc(MYSQL_DEFAULT_CMD_LEN, sizeof(char));
    p_mysql_con->cmd_buf_size = MYSQL_DEFAULT_CMD_LEN;

    if(NULL == p_mysql_con->cmd_buf)
        goto CreateEnd;

    p_mysql_con->p_mysql = mysql_init(NULL);

    if (NULL == p_mysql_con->p_mysql)
    {
        debug_print("%s: mysql_init error\n", __func__);
        goto CreateEnd;
    }
    if (NULL == mysql_real_connect(p_mysql_con->p_mysql, 
                                   host, user, passwd, db_name, port, NULL, CLIENT_MULTI_STATEMENTS))
    {
        debug_print("%s: mysql_real_connect error: %s\n", __func__, mysql_error(p_mysql_con->p_mysql));
        goto CreateEnd;
    }
    return p_mysql_con;

CreateEnd:
    if(NULL != p_mysql_con)
    {
        if(NULL != p_mysql_con->p_mysql)
            mysql_close(p_mysql_con->p_mysql);

        if(NULL != p_mysql_con->cmd_buf)
            free(p_mysql_con->cmd_buf);

        free(p_mysql_con);
        p_mysql_con = NULL;
    }

    return p_mysql_con;
}

int batch_insert_to_mysql(mysql_connect_t *p_mysql_con, 
            const char *tab_name, const char **pp_member, uint32_t member_count)
{
    int result = -1;

    if(NULL == p_mysql_con || NULL == p_mysql_con->p_mysql 
       || strlen(tab_name) > MYSQL_NAME_LEN || NULL == pp_member)
        return result;
    
    int row_count = 0;  
    MYSQL_RES *mysql_result = NULL;
    // judge if tab doesn't exist
    mysql_result = mysql_list_tables(p_mysql_con->p_mysql, tab_name);  

    while(mysql_fetch_row(mysql_result) != NULL)  
        row_count++;  

    if(row_count <= 0)
    {
        debug_print("%s: Tab %s doesn't exist\n", __func__, tab_name);
        if(mysql_result) 
            mysql_free_result(mysql_result);

        return result;
    }
    if(mysql_result) 
        mysql_free_result(mysql_result);

    uint32_t current_len = (uint32_t)snprintf(p_mysql_con->cmd_buf, 
                                    MYSQL_DEFAULT_CMD_LEN, "INSERT INTO %s VALUES", tab_name);
    uint32_t i = 0;
    for (i = 0; i < member_count; i++)
    {   
        current_len += (uint32_t)strlen(pp_member[i]); 

        // dynamic grown up
        if(current_len >= p_mysql_con->cmd_buf_size)
        {
            uint32_t before_size = p_mysql_con->cmd_buf_size;

            while(current_len >= p_mysql_con->cmd_buf_size)
                p_mysql_con->cmd_buf_size += p_mysql_con->cmd_buf_size;

            char *new_buf = (char*)calloc(p_mysql_con->cmd_buf_size, sizeof(char));
            if(NULL == new_buf) return result;

            memcpy(new_buf, p_mysql_con->cmd_buf, before_size * sizeof(char));

            free(p_mysql_con->cmd_buf);
            p_mysql_con->cmd_buf = new_buf;
        }
        strcat(p_mysql_con->cmd_buf, pp_member[i]);

        p_mysql_con->cmd_buf[current_len] = ',';
        current_len++;
    }
    p_mysql_con->cmd_buf[--current_len] = ';'; // replace ','

    if(0 != mysql_query(p_mysql_con->p_mysql, p_mysql_con->cmd_buf))
    {
        debug_print("%s: mysql_query error: %s\n", __func__, mysql_error(p_mysql_con->p_mysql));
        return result;
    }
    result = 0;
    return result;
}

static int query_mysql_by_cmd(mysql_connect_t *p_mysql_con, const char *p_cmd)
{
    int result = -1;

    if(NULL == p_mysql_con || NULL == p_mysql_con->p_mysql || NULL == p_cmd)
        return result;
 
    if(0 != mysql_query(p_mysql_con->p_mysql, p_cmd))
    {
        debug_print("%s: mysql_query error: %s\n", __func__, mysql_error(p_mysql_con->p_mysql));
        return result;
    }   
    result = 0;
    return result;
}

query_resource_t *query_single_content(mysql_connect_t *p_mysql_con, 
            const char *p_cmd, const char ***ppp_member, uint32_t *p_member_count)
{
    if(0 != query_mysql_by_cmd(p_mysql_con, p_cmd))
        return NULL;

    MYSQL_RES *mysql_result = mysql_store_result(p_mysql_con->p_mysql);
    if(NULL == mysql_result)
    {
        debug_print("%s: mysql_store_result error: %s\n", __func__, mysql_error(p_mysql_con->p_mysql));
        return NULL;
    } 
    uint32_t field_count = mysql_field_count(p_mysql_con->p_mysql);
    
    MYSQL_ROW mysql_row = mysql_fetch_row(mysql_result);

    if(NULL == mysql_row) {
        *p_member_count = 0;
        return NULL;
    }
    *p_member_count = field_count;
    *ppp_member = (const char**)mysql_row;

    query_resource_t *p_query = (query_resource_t*)calloc(1, sizeof(query_resource_t));

    if(p_query) {
        p_query->p_mysql_result = mysql_result;
    }
    return p_query;
}

query_resource_t *query_multi_content(mysql_connect_t *p_mysql_con, 
            const char *p_cmd, multi_result_t **p_multi_result_array, uint64_t *p_result_count)
{
    if(NULL == p_result_count || 0 != query_mysql_by_cmd(p_mysql_con, p_cmd))
        return NULL;

    MYSQL_RES *mysql_result = mysql_store_result(p_mysql_con->p_mysql);
    
    if(NULL == mysql_result) {
        debug_print("%s: mysql_store_result error: %s\n", __func__, mysql_error(p_mysql_con->p_mysql));
        return NULL;
    } 
    *p_result_count = (uint64_t)mysql_num_rows(mysql_result);
    
    multi_result_t *multi_result_array = (multi_result_t*)calloc(*p_result_count, sizeof(multi_result_t));

    if(NULL == multi_result_array) {
        mysql_free_result(mysql_result);
        return NULL;
    }
    uint32_t field_count = mysql_field_count(p_mysql_con->p_mysql);
    
    MYSQL_ROW mysql_row = NULL;
    
    uint32_t i = 0;
    for(; i < *p_result_count && (mysql_row = mysql_fetch_row(mysql_result)); i++) 
    {
        multi_result_array[i].pp_member = (const char**)mysql_row;
        multi_result_array[i].member_count = field_count;
    }
    query_resource_t *p_query = (query_resource_t*)calloc(1, sizeof(query_resource_t));

    if(NULL == p_query) {
        mysql_free_result(mysql_result);
        free(multi_result_array);
        *p_result_count = 0;
    }
    else {
        p_query->p_mysql_result = mysql_result;
        p_query->multi_result_array = multi_result_array;
        
        *p_multi_result_array = multi_result_array;
    }
    return p_query;
}

void destroy_query_content(query_resource_t *p_query_resource)
{
    if(p_query_resource) 
    {
        if(p_query_resource->p_mysql_result) 
        {
            mysql_free_result(p_query_resource->p_mysql_result);
            p_query_resource->p_mysql_result = NULL;
        }
        if(p_query_resource->multi_result_array) 
        {
            free(p_query_resource->multi_result_array);
            p_query_resource->multi_result_array = NULL;
        }
        free(p_query_resource);
        p_query_resource = NULL;
    }
}

int delete_content_by_condition(mysql_connect_t *p_mysql_con, const char *target, const char *condition)
{
    int result = -1;

    if(NULL == target || NULL == condition)
        return result;

    snprintf(p_mysql_con->cmd_buf, MYSQL_DEFAULT_CMD_LEN, "DELETE FROM %s WHERE %s;", target, condition);

    result = query_mysql_by_cmd(p_mysql_con, p_mysql_con->cmd_buf);

    return result;
}

void destroy_mysql_connect(mysql_connect_t *p_mysql_con)
{
    if(p_mysql_con)
    {
        if(NULL != p_mysql_con->p_mysql)
            mysql_close(p_mysql_con->p_mysql);

        if(NULL != p_mysql_con->cmd_buf)
            free(p_mysql_con->cmd_buf);

        free(p_mysql_con);
        p_mysql_con = NULL;
    }
}
