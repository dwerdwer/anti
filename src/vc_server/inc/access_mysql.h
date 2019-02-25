#ifndef ACCESS_MYSQL_HHH
#define ACCESS_MYSQL_HHH

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*  note:
 *  对于这层接口 数据结构是未知的
 *  insert 的 pp_member 字符串数组 需外部组织 
 * */

typedef struct query_resource query_resource_t;
typedef struct mysql_connect mysql_connect_t;

typedef struct
{
    const char **pp_member;
    uint32_t member_count;

}multi_result_t;

/* return: succ p_mysql_con    fail NULL */
mysql_connect_t *create_mysql_connect(const char *host, 
            const char *user, const char *passwd, const char *db_name, uint16_t port);

/* return: succ 0    fail -1 */
int batch_insert_to_mysql(mysql_connect_t *p_mysql_con, 
            const char *tab_name, const char **pp_member, uint32_t member_count);

/* Call destroy_query_content release
 * return: succ p_query_resource    fail NULL */
query_resource_t *query_single_content(mysql_connect_t *p_mysql_con, 
            const char *p_cmd, const char ***ppp_member, uint32_t *p_member_count);

/* Call destroy_query_content release
 * return: succ p_query_resource    fail NULL */
query_resource_t *query_multi_content(mysql_connect_t *p_mysql_con, 
            const char *p_cmd, multi_result_t **p_multi_result_array, uint64_t *p_result_count);

/* When p_query_resource is destroyed 
 * ppp_member and p_multi_result_array will not exist */
void destroy_query_content(query_resource_t *p_query_resource);

/* return: succ 0    fail -1 */
int delete_content_by_condition(mysql_connect_t *p_mysql_con, const char *target, const char *condition);

void destroy_mysql_connect(mysql_connect_t *p_mysql_con);

#ifdef __cplusplus
}
#endif

#endif
