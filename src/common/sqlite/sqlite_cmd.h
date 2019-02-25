
#ifndef SQLITE_CMD_HHH
#define SQLITE_CMD_HHH


#define MAX_CMD_LEN 1024 

#ifdef __cplusplus
extern "C" {
#endif


typedef int (*sqlite_cb_t)(void*, int, char**, char**);

/* 
   sqlite_cb: Callback function
   return: succ sqlite_ptr fail NULL
*/
void *create_sqlite_connect(const char *p_db_path, sqlite_cb_t sqlite_cb, const char *tab_name);

/* return: succ 0   fail -1 */
int sqlite_command(void *sqlite_ptr, const char *cmd_str);

/* return: succ top_id  '0' express can't find     fail -1 */
int get_top_element_id(void *sqlite_ptr);

/* 
   p_argument: 1st argument to callback
   return: succ 0   fail -1 
*/
int get_element_by_id(void *sqlite_ptr, void *p_argument, int id);
/* return: succ 0   fail -1 */
int delete_element_by_id(void *sqlite_ptr, int id);

/* 
   p_argument: 1st argument to callback
   return: succ 0   fail -1 
*/
int get_element_by_str(void *sqlite_ptr, void *p_argument, const char *member, const char *value_str);

/* return: succ 0   fail -1 */
int delete_element_by_str(void *sqlite_ptr, const char *member, const char *value_str);

/* return: succ tab_element_num   fail -1 */
int get_tab_element_num(void *sqlite_ptr, const char *cmd_str);

/* return: succ 0   fail -1 */
int inster_text_to_tab(void *sqlite_ptr, const char *member, const char *text_value);

void destroy_sqlite_connect(void *sqlite_ptr);

#ifdef __cplusplus
}
#endif

#endif
