
#ifndef VC_SERVER_FUNCTION_HHH
#define VC_SERVER_FUNCTION_HHH

#include <stdint.h>
#include <stdarg.h>
#include "module_interfaces.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ipc_connect ipc_connect_t;

/* return: succ p_ipc_connect   fail NULL */
ipc_connect_t *create_ipc_connect(uint32_t wt_count, uint32_t port, void *p_cache_data_queue);

void destroy_ipc_connect(ipc_connect_t *p_ipc_connect);

/* return: succ 0   fail -1 */
int send_to_other_modules(module_t *p_module, const char *p_data, 
            uint32_t data_len, const char *p_message_id, const char *p_message_type);

/* return: succ 0   fail -1 */
int send_to_reporter(module_t *p_module, const char *p_data, uint32_t data_len);

void vc_server_log(module_t *p_module, const char* fmt, ...);

#ifdef __cplusplus
}
#endif            

#endif
