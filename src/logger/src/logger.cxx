
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <sys/time.h>
#include <pthread.h>

#include "logger_interfaces.h"
#include "utils/utils_library.h"
#include "module_data.h"

#define MAX_LOG_LEN 4096
#define TIME_FORMAT_LEN 24

typedef struct
{
    uint32_t category;
    notify_scheduler_t notifier;
    void *p_params;
	
	FILE *wfp;
    char log_path[512];
	char **log_path_array;
    char log_buf[MAX_LOG_LEN];
    size_t max_size;
	uint16_t log_count;

	pthread_spinlock_t spinlock;

}log_file_t;


LIB_PUBLIC module_t *create(uint32_t category, notify_scheduler_t notifier, 
                     void *p_params, uint32_t arg_count, const char **p_args)
{
    if (arg_count < 3 || NULL == p_params)
		return NULL;
    
    const char *log_path    = p_args[0];
	size_t max_log_size     = (size_t)atol(p_args[1]);
    uint16_t log_count      = (uint16_t)atoi(p_args[2]);

    if (NULL == log_path || log_count < 1)
		return NULL;
    /* 预留长度 */
	int path_len = strlen(log_path) + 8;
	
	log_file_t *p_result = (log_file_t*)calloc(1, sizeof(log_file_t));

	if (NULL == p_result)
		return NULL;

	strncpy(p_result->log_path, log_path, sizeof(p_result->log_path));

    p_result->category = category;
    p_result->notifier = notifier;
    p_result->p_params = p_params;

	p_result->wfp = fopen(log_path, "a+");

    if (NULL == p_result->wfp) {
#ifdef _DEBUG
        debug_print("%s: %s\t", __func__, log_path);
        perror("fopen error");
#endif
        free(p_result);
		return NULL;
    }
	log_count--; // 不包括 current log

	p_result->log_path_array = (char**)calloc(log_count, sizeof(char*));

    if (NULL == p_result->log_path_array) {
#ifdef _DEBUG
        perror("create_log_handle: calloc error");
#endif
        fclose(p_result->wfp);
        free(p_result);
		return NULL;
    }
	for (uint16_t i = 0; i < log_count; i++)
	{
		p_result->log_path_array[i] = (char*)calloc(path_len, sizeof(char));

		snprintf(p_result->log_path_array[i], path_len, "%s_%d", log_path, i + 1);
	}
	p_result->max_size = max_log_size;

	p_result->log_count = log_count; // 最小为 0

	pthread_spin_init(&(p_result->spinlock), 0);

	return (module_t*)p_result;
}

LIB_PUBLIC void get_inputted_message_type(module_t *p_module, 
                const char *** const ppp_inputted_message_types, uint32_t *p_message_type_count)
{
    *ppp_inputted_message_types = NULL;
    *p_message_type_count = 0;
}

/* 日志轮转 */
static int loop_rotate(log_file_t *p_log)
{
    int result = 0;
    pthread_spin_lock(&p_log->spinlock);
    
    /* delete the oldest */
    remove(p_log->log_path_array[p_log->log_count - 1]);

    /* rename current log */
    for (uint16_t i = 1; i < p_log->log_count; i++)
    {
        rename(p_log->log_path_array[p_log->log_count - i - 1], 
               p_log->log_path_array[p_log->log_count - i]);
    }
    fclose(p_log->wfp);

    p_log->wfp = NULL;

    /* replace log file */
    rename(p_log->log_path, p_log->log_path_array[0]);

    p_log->wfp = fopen(p_log->log_path, "a+");

    if (NULL == p_log->wfp)
        result = -1;

    pthread_spin_unlock(&p_log->spinlock);

    return result;
}

LIB_PUBLIC module_state_t run(module_t *p_module)
{
    if (NULL == p_module)
        return MODULE_ERROR;

    return MODULE_OK;	
}

LIB_PUBLIC module_state_t stop(module_t *p_module)
{
    if (NULL == p_module)
        return MODULE_ERROR;

    log_file_t *p_log = (log_file_t*)p_module;

    pthread_spin_lock(&p_log->spinlock);

    fflush(p_log->wfp);

    pthread_spin_unlock(&p_log->spinlock);

    return MODULE_OK;
}

LIB_PUBLIC void destroy(module_t *p_module)
{
    if (p_module != NULL)
    {
        log_file_t *p_log = (log_file_t*)p_module;
        
        if (p_log->wfp != NULL)
            fclose(p_log->wfp);

        if (p_log->log_path_array != NULL)
        {
            for (uint16_t i = 0; i <  p_log->log_count; i++)
            {
                if (p_log->log_path_array[i] != NULL)
                    free (p_log->log_path_array[i]);

                p_log->log_path_array[i] = NULL;
            }
            free(p_log->log_path_array);
            p_log->log_path_array = NULL;
        }
        pthread_spin_destroy(&(p_log->spinlock));

        free(p_log);
        p_log = NULL;
    }
}

LIB_PUBLIC module_data_t* assign(module_t *p_module, const module_data_t *p_data, bool is_sync)
{
    if (NULL == p_module || NULL == p_data) 
        return NULL;

    log_file_t *p_log = (log_file_t*)p_module;

    const char *log_msg = NULL;
    uint32_t log_msg_len = 0;

    if (0 !=  get_module_data_property(p_data, "LOG_MESSAGE", &log_msg, &log_msg_len))
        return NULL;

    struct timeval now;
	char time_buf[TIME_FORMAT_LEN] = { 0 };
    
    gettimeofday(&now, NULL);

	strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&now.tv_sec));

    snprintf(time_buf + strlen(time_buf), 
             TIME_FORMAT_LEN - strlen(time_buf), ".%lu", now.tv_usec);

    pthread_spin_lock(&p_log->spinlock);
    
	snprintf(p_log->log_buf, MAX_LOG_LEN, "[%s] %s\n", time_buf, log_msg);

    size_t buf_len = strlen(p_log->log_buf);

    /* 判断 current size 是否满足轮换 */
    fseek(p_log->wfp, 0, SEEK_END);

    size_t log_current_size = (size_t)ftell(p_log->wfp);

    pthread_spin_unlock(&p_log->spinlock);

    log_current_size += buf_len;

    if (buf_len >= p_log->max_size)
        return NULL;

    if (log_current_size >= p_log->max_size && 0 == p_log->log_count)
    {
        /* log_count 可为 0 此时只有一个log文件 */
        pthread_spin_lock(&p_log->spinlock);
        fclose(p_log->wfp);
        p_log->wfp = NULL;

        remove(p_log->log_path);

        p_log->wfp = fopen(p_log->log_path, "a+");

        pthread_spin_unlock(&p_log->spinlock);

        if (NULL == p_log->wfp) {
#ifdef _DEBUG
            perror("log to file: fopen error");
#endif
            return NULL;
        }
    }
    else if (log_current_size >= p_log->max_size && loop_rotate(p_log) != 0) {
#ifdef _DEBUG
        perror("loop_rotate error");
#endif
        return NULL;
    }
    pthread_spin_lock(&p_log->spinlock);

    fwrite(p_log->log_buf, buf_len, 1, p_log->wfp);
    fflush(p_log->wfp);

    pthread_spin_unlock(&p_log->spinlock);

    return NULL;
}
