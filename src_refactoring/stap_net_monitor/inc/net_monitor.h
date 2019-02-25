#ifndef __NET_MONITOR_H__
#define __NET_MONITOR_H__

#include "packet_info.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define PUBLIC_API __attribute__((visibility("default")))

typedef struct net_monitor net_monitor_t;

typedef void (*http_monitor_cb_t)(http_info_t *p_info, void *cb_param);

typedef void (*dns_monitor_cb_t)(dns_info_t *p_info, void *cb_param);

typedef void (*tcp_monitor_cb_t)(tcp_info_t *p_info, void *cb_param);

typedef void (*udp_monitor_cb_t)(udp_info_t *p_info, void *cb_param);

typedef void (*icmp_monitor_cb_t)(icmp_info_t *p_info, void *cb_param);


net_monitor_t *init_net_monitor(void);

int run_net_monitor(net_monitor_t *p_monitor);

void stop_net_monitor(net_monitor_t *p_monitor);

void fin_net_monitor(net_monitor_t *p_monitor);

/* return: succ 0   fail -1 */
int net_monitor_set_http(net_monitor_t *p_monitor, http_monitor_cb_t cb, void *cb_param);

int net_monitor_off_http(net_monitor_t *p_monitor);

int net_monitor_set_dns(net_monitor_t *p_monitor, dns_monitor_cb_t cb, void *cb_param);

int net_monitor_off_dns(net_monitor_t *p_monitor);

int net_monitor_set_tcp(net_monitor_t *p_monitor, tcp_monitor_cb_t cb, void *cb_param);

int net_monitor_off_tcp(net_monitor_t *p_monitor);

int net_monitor_set_udp(net_monitor_t *p_monitor, udp_monitor_cb_t cb, void *cb_param);

int net_monitor_off_udp(net_monitor_t *p_monitor);

int net_monitor_set_icmp(net_monitor_t *p_monitor, icmp_monitor_cb_t cb, void *cb_param);

int net_monitor_off_icmp(net_monitor_t *p_monitor);


#undef PUBLIC_API

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __NET_MONITOR_H__ */

