
#include "utils_network.h"

#include <stddef.h>

#include <arpa/inet.h>

//////////////////////////////////////////////////////////////
// Private Implementations
//////////////////////////////////////////////////////////////
static int32_t fill_inet4_addr(peer_addr_t *p_peer_addr, socket_address_t *p_socket_address)
{
    int32_t result = -1;
    struct sockaddr_in *p_sin4 = &(p_socket_address->sin4);
    if (NULL != inet_ntop(AF_INET, &(p_sin4->sin_addr), p_peer_addr->address, INET_ADDRSTRLEN))
    {
        p_peer_addr->port = p_sin4->sin_port;
        result = 0;
    }
    return result;
}

static int32_t fill_inet6_addr(peer_addr_t *p_peer_addr, socket_address_t *p_socket_address)
{
    int32_t result = -1;
    struct sockaddr_in6 *p_sin6 = &(p_socket_address->sin6);
    if (NULL != inet_ntop(AF_INET6, &(p_sin6->sin6_addr), p_peer_addr->address, INET6_ADDRSTRLEN))
    {
        p_peer_addr->port = p_sin6->sin6_port;
        result = 0;
    }
    return result;
}


//////////////////////////////////////////////////////////////
// Public Interfaces
//////////////////////////////////////////////////////////////

int32_t fill_peer_addr(peer_addr_t *p_peer_addr, socket_address_t *p_socket_address)
{
    int32_t result = -1;
    switch(p_socket_address->general.sa_family)
    {
        case AF_INET:
            result = fill_inet4_addr(p_peer_addr, p_socket_address);
            break;
        case AF_INET6:
            result = fill_inet6_addr(p_peer_addr, p_socket_address);
            break;
        default:
            break;
    }
    return result;
}



