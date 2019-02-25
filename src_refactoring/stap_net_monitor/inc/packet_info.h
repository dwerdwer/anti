#ifndef __PACKET_INFO_H__
#define __PACKET_INFO_H__

#include <inttypes.h>
#include <time.h>

/* Network byte order */
typedef struct
{
    uint32_t source_address;
    uint32_t destination_address;
} ip_hdr_t;

/* Network byte order */
typedef struct
{
    uint16_t source_port;
    uint16_t destination_port;
} tcp_hdr_t, udp_hdr_t;

typedef struct http_info
{
    ip_hdr_t ip_hdr;
    tcp_hdr_t tcp_hdr;
    const char *p_url;
    time_t time;
    int pid;
} http_info_t;

typedef struct dns_info
{
    ip_hdr_t ip_hdr;
    udp_hdr_t udp_hdr;
    const char *p_dns_query;
    uint32_t response_address;
    time_t time;
    int pid;
} dns_info_t;

typedef struct icmp_info
{
    ip_hdr_t ip_hdr;
    int pid;
    time_t time;
} icmp_info_t;

typedef struct
{
    ip_hdr_t ip_hdr;
    tcp_hdr_t tcp_hdr;
    int pid;
    time_t time;
} tcp_info_t;

typedef struct
{
    ip_hdr_t ip_hdr;
    udp_hdr_t udp_hdr;
    int pid;
    time_t time;
} udp_info_t;

__attribute__((visibility("default")))
http_info_t *http_info_deep_copy(const http_info_t *p_src_info);

__attribute__((visibility("default")))
void http_info_free(http_info_t *p_info);

__attribute__((visibility("default")))
dns_info_t *dns_info_deep_copy(const dns_info_t *p_src_info);

__attribute__((visibility("default")))
void dns_info_free(dns_info_t *p_info);

__attribute__((visibility("default")))
icmp_info_t *icmp_info_deep_copy(const icmp_info_t *p_src_info);

__attribute__((visibility("default")))
void icmp_info_free(icmp_info_t *p_info);

__attribute__((visibility("default")))
tcp_info_t *tcp_info_deep_copy(const tcp_info_t *p_src_info);

__attribute__((visibility("default")))
void tcp_info_free(tcp_info_t *p_info);

__attribute__((visibility("default")))
udp_info_t *udp_info_deep_copy(const udp_info_t *p_src_info);

__attribute__((visibility("default")))
void udp_info_free(udp_info_t *p_info);

#endif /* __PACKET_INFO_H__ */
