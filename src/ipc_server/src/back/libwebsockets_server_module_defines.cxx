
#include "libwebsockets_server_module_defines.h"

#include <cstddef>

void callback_base::notify(void *p_conn_params, server_notify_reason_t reason, 
        peer_addr_t *p_peer_addr, const char *p_info, uint32_t info_len)
{
}

bool callback_base::is_closable(void *p_conn_params)
{
    return false;
}

void *callback_base::connection_create(void)
{
    return NULL;
}

void callback_base::connection_destroy(void *p_conn_params)
{
}

