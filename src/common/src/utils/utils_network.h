#ifndef UTILS_NETWORK_HHH
#define UTILS_NETWORK_HHH

#include <netinet/in.h>

typedef union socket_address
{
    struct sockaddr_storage storage;
    struct sockaddr general;
    struct sockaddr_in6 sin6;
    struct sockaddr_in sin4;
}socket_address_t;

typedef struct peer_addr
{
    char address[INET6_ADDRSTRLEN];
    in_port_t port;
    //socket_address_t socket_address;
}peer_addr_t;

int32_t fill_peer_addr(peer_addr_t *p_peer_addr, socket_address_t *p_socket_address);

#endif

