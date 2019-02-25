
#ifndef CACHE_TO_SQLITE_HHH
#define CACHE_TO_SQLITE_HHH

#include <stdint.h>
#include <stdio.h> // size_t...

#ifdef __cplusplus
extern "C" {
#endif

typedef struct json_cache json_cache_t;

/* return: succ p_cache    fail NULL */
json_cache_t *create_jsonstr_cache(const char *p_db_path);

/* return: succ 0   fail -1 */
int set_jsonstr_to_cache(json_cache_t *p_cache, const char *json_str);

/* return: succ top id ( 0 express not find)   fail -1 */
int64_t get_jsonstr_cache_top_id(json_cache_t *p_cache);
/* 
 * note: 需外部释放
 * return: succ json_str    fail NULL */
char *get_jsonstr_by_id(json_cache_t *p_cache, int64_t id, size_t *json_len);

/* return: succ 0   fail -1 */
int delete_jsonstr_by_id(json_cache_t *p_cache, int64_t id);

/* return: succ current element sum  fail -1 */
int get_cache_ele_sum(json_cache_t *p_cache);

void destroy_jsonstr_cache(json_cache_t *p_cache);

#ifdef __cplusplus
}
#endif


#endif
