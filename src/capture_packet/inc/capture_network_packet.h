#ifndef CAPTURE_NETWORK_PACKET_HHH
#define CAPTURE_NETWORK_PACKET_HHH

#include <stdint.h>
#include <ctime>
#include <stdlib.h>
#include <string.h>

extern "C" {

// 'NP' is short for "network protocol"
typedef enum network_protocol
{
    NP_UNKNOWN = 0,
    NP_TCP,
    NP_UDP,
    NP_HTTP,
    NP_HTTPS,
}network_protocol_t;

typedef struct conn_info
{
    char source_address[16];
    char destination_address[16];
    uint16_t source_port;
    uint16_t destination_port;
}conn_info_t;

typedef struct network_info
{
    conn_info_t connection;
    network_protocol_t protocol;
    const char *p_url;
    time_t time;
    uint8_t result;
}network_info_t;

typedef void (*network_notifier_t)(network_info_t *p_info, void *p_params);

typedef struct dns_info
{
    conn_info_t connection;
    const char *p_dns_query;
    char response_address[46];
    time_t time;
}dns_info_t;

typedef void (*dns_notifier_t)(dns_info_t *p_info, void *p_params);

typedef struct notifier_info
{
    network_notifier_t notify_network_activity;
    void *p_network_params;
    dns_notifier_t notify_dns_activity;
    void *p_dns_params;
}notifier_info_t;

// Return type is 'void*'. This is compatible to run as an isolated thread.
void *capture_packet_on_nics(notifier_info_t *p_notifiers);

static network_info_t *network_info_alloc(uint32_t ip_src, uint16_t port_src, uint32_t ip_dest, uint16_t port_dest, network_protocol_t protocol, const char *p_url, time_t time, uint8_t result)
{
    network_info_t *p_info = (network_info_t *)calloc(1, sizeof(network_info_t));
    if(p_info) {
        memcpy(p_info->connection.source_address, &ip_src, sizeof(ip_src));
        memcpy(p_info->connection.destination_address, &ip_dest, sizeof(ip_dest));
        p_info->connection.source_port = port_src;
        p_info->connection.destination_port = port_dest;

        p_info->p_url = strdup(p_url);
        p_info->protocol = protocol;
        p_info->result = result;
        p_info->time = time;
    }
    return p_info;
}
static network_info_t *network_info_deep_copy(network_info_t *p_src_info)
{
    network_info_t *p_info = (network_info_t *)calloc(1, sizeof(network_info_t));
    if(p_info) {
        memcpy(p_info, p_src_info, sizeof(network_info_t));
        p_info->p_url = strdup(p_src_info->p_url);
    }
    return p_info;
}
static void network_info_free(network_info_t *p_info)
{
    if(p_info) {
        if(p_info->p_url)
            free((char *)p_info->p_url);
        free(p_info);
    }
}

static dns_info_t *dns_info_deep_copy(dns_info_t *p_src_info)
{
    dns_info_t *p_info = (dns_info_t *)calloc(1, sizeof(dns_info_t));
    if(p_info) {
        memcpy(p_info, p_src_info, sizeof(dns_info_t));
        p_info->p_dns_query = strdup(p_src_info->p_dns_query);
    }
    return p_info;
}

static void dns_info_free(dns_info_t *p_info)
{
    if(p_info) {
        if(p_info->p_dns_query)
            free((char *)p_info->p_dns_query);
        free(p_info);
    }
}

}

#endif

