
#include "libwebsockets_server_module_defines.h"

#include <cstddef>

void callback_base::notify(void *p_rpc_conn, server_notify_reason_t reason,
        peer_addr_t *p_peer_addr, const char *p_info, uint32_t info_len)
{
}

bool callback_base::is_closable(void *p_rpc_conn)
{
    return false;
}
connect_t callback_base::connection_create(IN const peer_addr_t *p_peer_addr, IN void* p_token)
{
    return NULL;
}

void callback_base::connection_destroy(void *p_rpc_conn)
{
}
