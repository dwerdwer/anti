#include <stdio.h>
#include <inttypes.h>
#include <signal.h>

#include <unistd.h>     // sleep
#include <arpa/inet.h>  // inet_ntop, ntohs

#include "net_monitor.h"


static void http_callback(http_info_t *p_info, void *cb_param)
{
    printf("%s\n", "HTTP");
    printf("\tpid: %d\n", p_info->pid);
    printf("\turl:[%s]\n", p_info->p_url);

    // tuple
    ip_hdr_t *p_ip = &p_info->ip_hdr;
    char src_ip_str[16], dst_ip_str[16];
    inet_ntop(AF_INET, &p_ip->source_address, src_ip_str, sizeof(src_ip_str));
    inet_ntop(AF_INET, &p_ip->destination_address, dst_ip_str, sizeof(dst_ip_str));
    printf("\ttuple [%s]:[%d] --> [%s][%d]\n",
           src_ip_str, ntohs(p_info->tcp_hdr.source_port),
           dst_ip_str, ntohs(p_info->tcp_hdr.destination_port));
    http_info_free(p_info);

    putchar(10);
}

static void icmp_callback(icmp_info_t *p_info, void *cb_param)
{
    printf("%s\n", "ICMP");
    printf("\tpid: %d\n", p_info->pid);

    // tuple
    ip_hdr_t *p_ip = &p_info->ip_hdr;
    char src_ip_str[16], dst_ip_str[16];
    inet_ntop(AF_INET, &p_ip->source_address, src_ip_str, sizeof(src_ip_str));
    inet_ntop(AF_INET, &p_ip->destination_address, dst_ip_str, sizeof(dst_ip_str));

    printf("\ttuple [%s]--> [%s]\n", src_ip_str, dst_ip_str);

    icmp_info_free(p_info);

    putchar(10);
}

static void tcp_callback(tcp_info_t *p_info, void *cb_param)
{
    printf("%s\n", "TCP");
    printf("\tpid: %d\n", p_info->pid);

    // tuple
    ip_hdr_t *p_ip = &p_info->ip_hdr;
    char src_ip_str[16], dst_ip_str[16];
    inet_ntop(AF_INET, &p_ip->source_address, src_ip_str, sizeof(src_ip_str));
    inet_ntop(AF_INET, &p_ip->destination_address, dst_ip_str, sizeof(dst_ip_str));
    printf("\ttuple [%s]:[%d] --> [%s][%d]",
           src_ip_str, ntohs(p_info->tcp_hdr.source_port),
           dst_ip_str, ntohs(p_info->tcp_hdr.destination_port));

    tcp_info_free(p_info);

    putchar(10);
}

static void dns_callback(dns_info_t *p_info, void *cb_param)
{
    printf("%s\n", "DNS");
    printf("\tpid: %d\n", p_info->pid);

    // tuple
    ip_hdr_t *p_ip = &p_info->ip_hdr;
    char src_ip_str[16], dst_ip_str[16];
    inet_ntop(AF_INET, &p_ip->source_address, src_ip_str, sizeof(src_ip_str));
    inet_ntop(AF_INET, &p_ip->destination_address, dst_ip_str, sizeof(dst_ip_str));
    printf("\ttuple [%s]:[%d] --> [%s][%d]",
           src_ip_str, ntohs(p_info->udp_hdr.source_port),
           dst_ip_str, ntohs(p_info->udp_hdr.destination_port));

    dns_info_free(p_info);

    putchar(10);
}


static net_monitor_t *handle = NULL;

static void on_signal(int sig)
{
    printf("\nexit with sig %d\n", sig);
    stop_net_monitor(handle);
}

int32_t main(int32_t argc,char *args[])
{
#if defined(__cplusplus) && __cplusplus > 199711L
    printf("g++ ver: %lu > 199711L\n", __cplusplus);
#endif
    signal(SIGINT, on_signal);

    handle = init_net_monitor();

    sleep(5);
    net_monitor_set_http(handle, http_callback, NULL);

    net_monitor_set_icmp(handle, icmp_callback, NULL);

    net_monitor_set_tcp(handle, tcp_callback, NULL);

    run_net_monitor(handle);

    stop_net_monitor(handle);
    fin_net_monitor(handle);
    return 0;
}
