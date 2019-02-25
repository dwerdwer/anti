
#ifndef FILE_DIGEST_SQLITE_HHH
#define FILE_DIGEST_SQLITE_HHH

#include <time.h>
#include <stdint.h>
#include <linux/limits.h>

#ifdef __cplusplus
extern "C" {
#endif

/* input & output */
typedef struct 
{
    char md5[64];
    char file_path[PATH_MAX];
    size_t file_size;
    time_t created_at; /* time of last status change */

}file_digest_t;

/* return: succ p_cache_data    fail NULL */
void *create_file_digest_cache(const char *p_db_path);

/* return: succ 0   fail -1 */
int cache_to_file_digest(void *p_cache_data, const char *p_file_path);

/* return: succ top_id  '0' express can't find     fail -1 */ 
//int get_cache_top_id(void *p_cache_data);

/* 
 * note: 需外部释放
 * return: succ p_file_digest    fail NULL */
file_digest_t *get_file_digest_by_md5(void *p_cache_data, const char *md5_str);
/* return: succ 0   fail -1 */
int delete_file_digest_by_md5(void *p_cache_data, const char *md5_str);

/* 
 * note: 需外部释放
 * return: succ p_file_digest    fail NULL */
file_digest_t *get_file_digest_by_path(void *p_cache_data, const char *p_file_path);
/* return: succ 0   fail -1 */
int delete_file_digest_by_path(void *p_cache_data, const char *p_file_path);

/* return: succ current element sum  fail -1 */
int get_cache_file_digest_sum(void *p_cache_data);

void destroy_file_digest_cache(void *p_cache_data);

#ifdef __cplusplus
}
#endif


#endif
