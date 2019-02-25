
#include "make_log.h"
#include "debug_print.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>
#include <pthread.h>

typedef struct log_file
{
	const char *p_log_path; // 路径
	FILE *p_file;           // 文件指针
	char **log_path_array;  // 路径数组
	size_t log_max_size;    // log 限定大小
	uint16_t log_file_num;       // log 限定数量
	pthread_spinlock_t spinlock;
}log_file_t;

void *create_log_file(const char *p_log_file_path, size_t max_log_size, uint16_t log_file_num)
{   
	if (NULL == p_log_file_path || log_file_num < 1)
	{
		DEBUG_PRINT("parameter invalid\n");
		return NULL;
	}
	log_file_t *p_result = (log_file_t*)malloc(sizeof(log_file_t));

	if (NULL == p_result)
	{   
		//perror("malloc error");
		return NULL;
	}
	memset(p_result, 0, sizeof(log_file_t));

	p_result->p_log_path = p_log_file_path;

	p_result->p_file = fopen(p_log_file_path, "a+");

	if (NULL == p_result->p_file)
	{   
		DEBUG_PRINT("p_result->p_file fopen error\n");
		return NULL;
	}
	log_file_num--; // 不包括 current log

	//---------------------------------------------------------------
	// 预申请 path 数组空间
	p_result->log_path_array = (char**)malloc(log_file_num * sizeof(char*));

	if (NULL == p_result->log_path_array)
	{   
		DEBUG_PRINT("malloc error\n");
		return NULL;
	}
	size_t path_len = strlen(p_log_file_path);
	path_len += 5;
   
    uint16_t i = 0;
	for (i = 0; i < log_file_num; i++)
	{
		p_result->log_path_array[i] = (char*)malloc(path_len * sizeof(char));

		if (NULL == p_result->log_path_array[i])
		{   
    		DEBUG_PRINT("malloc error\n");
			return NULL;
		}
		memset (p_result->log_path_array[i], 0, path_len * sizeof(char));

		snprintf(p_result->log_path_array[i], path_len * sizeof(char), "%s_%d", p_log_file_path, i + 1);
	}
	p_result->log_max_size = max_log_size;

	p_result->log_file_num = log_file_num; // 最小为 0

	pthread_spin_init(&(p_result->spinlock), 0);

	return (void*)p_result;
}


/* 日志轮转 */
static int loop_rotate(log_file_t *p_log)
{
	pthread_spin_lock(&p_log->spinlock);

	// delete 最老 log
	remove(p_log->log_path_array[p_log->log_file_num - 1]);

	// ""后移" 现存 log
	uint16_t i = 1;
    for (i = 1; i < p_log->log_file_num; i++)
	{
		rename(p_log->log_path_array[p_log->log_file_num - i - 1], 
				p_log->log_path_array[p_log->log_file_num - i]);
	}
	fclose(p_log->p_file);

	p_log->p_file = NULL;

	// replace log file
	rename(p_log->p_log_path, p_log->log_path_array[0]);

	p_log->p_file = fopen(p_log->p_log_path, "a+");

	if (NULL == p_log->p_file)
	{
		DEBUG_PRINT("fopen error\n");
		return -1;
	}
	pthread_spin_unlock(&p_log->spinlock);

	return 0;
}

/* 写入内容 */
int log_to_file(void *p_log_ret, char *fmt, ...)
{
	if (NULL == p_log_ret)
		return -1;
    log_file_t *p_log =(log_file_t*)p_log_ret;

	char mesg[MAX_LOG_LEN] = {0};
	char buf[MAX_LOG_LEN] = {0};

	struct timeval now;
	char time_buf[64];
	va_list ap;
	va_start(ap, fmt);
	vsprintf(mesg, fmt, ap);
	va_end(ap);

	gettimeofday(&now, NULL);

	strftime(time_buf, 64, "%Y-%m-%d %H:%M:%S",localtime(&now.tv_sec));
	char cut[12] = { 0 };
	snprintf(cut, sizeof(cut), "%lu", now.tv_usec);

	snprintf(buf, MAX_LOG_LEN, "[%s.%.*s] %s\n", time_buf, 3, cut, mesg);

	size_t buf_len = strlen(buf);

	pthread_spin_lock(&p_log->spinlock);
	//---------------------------------------------------------------
	// 判断 current size 是否满足轮换
	fseek(p_log->p_file, 0, SEEK_END);

	size_t log_current_size = (size_t)ftell(p_log->p_file);

	pthread_spin_unlock(&p_log->spinlock);

	log_current_size += buf_len;

	if (buf_len >= p_log->log_max_size)
	{
		DEBUG_PRINT("buf_len >= log_max_size\n");
		return -1;
	}

	if (log_current_size >= p_log->log_max_size)
	{
		// log_file_num 可为 0 此时只有 current log
		if (0 == p_log->log_file_num) 
		{
			fclose(p_log->p_file);

			p_log->p_file = NULL;

			remove(p_log->p_log_path);
			
			p_log->p_file = fopen(p_log->p_log_path, "a+");

			if (NULL == p_log->p_file)
			{
				DEBUG_PRINT("fopen error\n");
				return -1;
			}
		}
		else if (loop_rotate(p_log) != 0)
		{
			DEBUG_PRINT("loop_rotate error\n");
			return -1;
		}
		log_current_size = 0;
	}
	//---------------------------------------------------------------
	// 写入 log 文件
	pthread_spin_lock(&p_log->spinlock);

	fwrite(buf, buf_len, 1, p_log->p_file);
	fflush(p_log->p_file);

	pthread_spin_unlock(&p_log->spinlock);

	return 0;
}

void destroy_log_file(void *p_log_ret)
{
	if (p_log_ret != NULL)
	{
        log_file_t *p_log =(log_file_t*)p_log_ret;
		
        if (p_log->p_file != NULL)
			fclose(p_log->p_file);
	
		if (p_log->log_path_array != NULL)
		{
            uint16_t i = 0; 
			for (i = 0; i <  p_log->log_file_num; i++)
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

