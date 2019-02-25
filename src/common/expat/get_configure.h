#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RET_BUF_LEN 256 // 返回值最大内存大小

#ifdef __cplusplus
extern "C" {
#endif

/*
   初始化读配置文件
read_path:	配置文件路径

return:
succ  p_con 指针
fail  NULL
*/
void* create_configure_paser(const char *read_path);

/*
   获取配置信息
ret_buf:    结果缓存 小于 RET_BUF_LEN
p_con:      create_configure_paser 返回指针
key:        传入要查询的 key 值

return:
succ   0
fail  -1
*/
int get_configure_info(void *p_con, char *ret_buf, const char *key);

/*
   释放资源
ret_pointer:   create_configure_paser 返回指针
*/
void destroy_configure_paser(void *p_con);

#ifdef __cplusplus
}
#endif
