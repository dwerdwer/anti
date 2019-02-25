#pragma once
#include <stdio.h>
#include <stdint.h>

#define MAX_LOG_LEN 1024

/* 
return:
succ   void* p_log_ret
fail   NULL
 */
void *create_log_file(const char *p_log_file_path, size_t max_log_size, uint16_t log_file_num);

/*
return:
succ   0
fail  -1
 */
int log_to_file(void *p_log_ret, char *fmt, ...);


void destroy_log_file(void *p_log_ret);


