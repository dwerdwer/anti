#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <pthread.h>
#include <inttypes.h>
#include <map>
#include <string>
#include <sstream>

#include <unistd.h>
#include <arpa/inet.h>

#include "debug_print.h"
#include "net_monitor.h"
#include "stap_monitor.h"
#include "util-spinlock.h"
#include "capture_network_packet.h"

static const char *stap_default_exe = "SYSTEMTAP_STAPIO=./stapio ./staprun -R";
static const char *stap_default_stp = "../lib/edr_stap_net_monitor.ko";

#ifndef LIB_PUBLIC
#define LIB_PUBLIC __attribute__ ((visibility ("default")))
#endif

#define DEFAULT_TIME_OUT            30
#define DEFAULT_ITEM_ARRAY_LEN      256

struct net_monitor
{
    // callbacks
    http_monitor_cb_t http_cb;
    dns_monitor_cb_t dns_cb;
    tcp_monitor_cb_t tcp_cb;
    udp_monitor_cb_t udp_cb;
    icmp_monitor_cb_t icmp_cb;

    // parameters
    void *http_params;
    void *dns_params;
    void *tcp_params;
    void *udp_params;
    void *icmp_params;

    // libpcap
    notifier_info_t notifier_info;
    void *p_libpcap_handle;
    // libpcap cache
    std::multimap<uint16_t, http_info_t *> http_cache;
    time_t http_cache_refresh_prev;
    spinlock_t http_cache_lock;

    std::multimap<uint16_t, dns_info_t *> dns_cache;
    time_t dns_cache_refresh_prev;
    spinlock_t dns_cache_lock;

    // stap
    stap_monitor_t *p_stap_monitor;
    // stap cache
    std::map<uint16_t, net_action_t *> stap_cache;
    time_t stap_cache_refresh_prev;
    spinlock_t stap_cache_lock;

    bool running;
    time_t refresh_delta;

    std::string *item_array;
};

static net_action_t *net_action_deep_copy(const net_action_t *p_src_info)
{
    net_action_t *p_info = (net_action_t *)calloc(1, sizeof(net_action_t));
    if(p_info) {
        memcpy(p_info, p_src_info, sizeof(net_action_t));
    }
    return p_info;
}

static void net_action_free(net_action_t *p_info)
{
    if(p_info) {
        free(p_info);
    }
}

static int net_action_insert(net_monitor_t *p_monitor, const net_action_t *p_net_action)
{
    int ret = -1;
    p_monitor->stap_cache_lock.lock();
    std::map<uint16_t, net_action_t *>::iterator itr = p_monitor->stap_cache.find(p_net_action->local_port);
    if(itr == p_monitor->stap_cache.end()) {
        p_monitor->stap_cache.insert(std::pair<uint16_t, net_action_t *>(p_net_action->local_port, net_action_deep_copy(p_net_action)));
        ret = 0;
    }
    else if(itr->second == NULL) {
        p_monitor->stap_cache.erase(itr);
        p_monitor->stap_cache.insert(std::pair<uint16_t, net_action_t *>(p_net_action->local_port, net_action_deep_copy(p_net_action)));
        ret = 0;
    }
    else {
        net_action_t *p_dest_tuple = itr->second;
        if((p_dest_tuple->foreign_port != p_net_action->foreign_port)
            || (p_dest_tuple->foreign_ip != p_net_action->foreign_ip))
        {
            net_action_free(p_dest_tuple);
            p_monitor->stap_cache.erase(itr);
            p_monitor->stap_cache.insert(std::pair<uint16_t, net_action_t *>(p_net_action->local_port, net_action_deep_copy(p_net_action)));
            ret = 0;
        }
        else {
            p_dest_tuple->time = p_net_action->time;
            ret = 1;
       }
    }
    p_monitor->stap_cache_lock.unlock();

    if(ret == 0) {
        debug_print("%s lport:%5d -> fport:%5d pid:%10d\n",
                __func__, p_net_action->local_port,
                p_net_action->foreign_port, p_net_action->pid);
    }

    return ret;
}

static void refresh_stap_cache(net_monitor_t *p_monitor, time_t now)
{
    if(p_monitor->stap_cache_refresh_prev + p_monitor->refresh_delta > now) {
        return;
    }
    p_monitor->stap_cache_refresh_prev = now;

    int warn=0, tmout=0;
    p_monitor->stap_cache_lock.lock();
    for(std::map<uint16_t, net_action_t *>::iterator itr = p_monitor->stap_cache.begin();
            itr != p_monitor->stap_cache.end(); ) {

        net_action_t *p_net_action = itr->second;
        if(p_net_action == NULL) {
            p_monitor->stap_cache.erase(itr++);
            ++warn;
        }
        else if(now >= p_net_action->time + DEFAULT_TIME_OUT) {
            net_action_free(p_net_action);
            p_monitor->stap_cache.erase(itr++);
            ++tmout;
        }
        else {
            ++itr;
        }
    }
    debug_print("%10s warn:%d, tmout:%d, left:%d\n", __func__, warn, tmout, (int)p_monitor->http_cache.size());
    p_monitor->stap_cache_lock.unlock();
}

static void on_stap_net_action(const net_action_t *p_net_action, void *p_user_data)
{
    net_monitor_t *p_monitor = (net_monitor_t *)p_user_data;
    if(!p_monitor->running)
        return;

    refresh_stap_cache(p_monitor, p_net_action->time);

    if(net_action_insert(p_monitor, p_net_action)) {
        return;
    }

    do {
        p_monitor->http_cache_lock.lock();
        std::multimap<uint16_t, http_info_t *>::iterator http_itr = p_monitor->http_cache.find(p_net_action->local_port);
        if(http_itr == p_monitor->http_cache.end()) {
            p_monitor->http_cache_lock.unlock();
            break;
        }
        http_info_t *p_info = http_itr->second;
        if(p_info == NULL || p_info->time + DEFAULT_TIME_OUT < p_net_action->time) {
            http_info_free(p_info);
            p_monitor->http_cache.erase(http_itr);
            p_monitor->http_cache_lock.unlock();
            continue;
        }
        p_info->pid = p_net_action->pid;
        // don't free p_info, just erase iterator, free will invoked by p_user_data
        p_monitor->http_cache.erase(http_itr);
        p_monitor->http_cache_lock.unlock();

        if(p_monitor->http_cb) {
            p_monitor->http_cb(p_info, p_monitor->http_params);
        }
        debug_print("%s lport:%d -> fport:%d pid:%d",
                    __func__,
                    ntohs(p_info->tcp_hdr.destination_port),
                    ntohs(p_info->tcp_hdr.source_port), p_info->pid);
        if (p_info->p_url) {
            debug_print(" [%s]\n", p_info->p_url);
        }
        else {
            debug_print("\n");
        }
    } while(true);

    do {
        p_monitor->dns_cache_lock.lock();
        std::multimap<uint16_t, dns_info_t *>::iterator dns_itr = p_monitor->dns_cache.find(p_net_action->local_port);
        if(dns_itr == p_monitor->dns_cache.end()) {
            p_monitor->dns_cache_lock.unlock();
            break;
        }
        dns_info_t *p_info = dns_itr->second;
        if(p_info == NULL || p_info->time + DEFAULT_TIME_OUT < p_net_action->time) {
            dns_info_free(p_info);
            p_monitor->dns_cache.erase(dns_itr);
            p_monitor->dns_cache_lock.unlock();
            continue;
        }
        p_info->pid = p_net_action->pid;
        // don't free p_info, just erase iterator, free will invoked by p_user_data
        p_monitor->dns_cache.erase(dns_itr);
        p_monitor->dns_cache_lock.unlock();

        if(p_monitor->dns_cb) {
            p_monitor->dns_cb(p_info, p_monitor->dns_params);
        }
        debug_print("%s lport:%d -> fport:%d pid:%d [%s]\n",
                __func__,
                ntohs(p_info->tcp_hdr.destination_port),
                ntohs(p_info->tcp_hdr.source_port),
                p_info->pid, p_info->p_dns_query);
    } while(true);
}

static void refresh_http_cache(net_monitor_t *p_monitor, time_t now)
{
    if(p_monitor->http_cache_refresh_prev + p_monitor->refresh_delta > now) {
        return;
    }
    p_monitor->http_cache_refresh_prev = now;

    int warn = 0, tmout = 0;
    p_monitor->http_cache_lock.lock();
    for(std::multimap<uint16_t, http_info_t *>::iterator itr = p_monitor->http_cache.begin();
            itr != p_monitor->http_cache.end(); ) {

        http_info_t *p_info = itr->second;
        if(p_info == NULL) {
            p_monitor->http_cache.erase(itr++);
            ++warn;
        }
        else if(now >= p_info->time + DEFAULT_TIME_OUT) {
            http_info_free(p_info);
            p_monitor->http_cache.erase(itr++);
            ++tmout;
        }
        else {
            ++itr;
        }
    }
    debug_print("%10s warn:%d, tmout:%d, left:%d\n", __func__, warn, tmout, (int)p_monitor->http_cache.size());
    p_monitor->http_cache_lock.unlock();
}

static void on_pcap_http_action(http_info_t *p_data, void *p_params)
{
    if (NULL == p_data || NULL == p_params)
        return;

    http_info_t *p_info = http_info_deep_copy(p_data);
    if (p_info == NULL)
        return;

    net_monitor_t *p_monitor = (net_monitor_t*)p_params;
    if(!p_monitor->running)
        return;

    refresh_http_cache(p_monitor, p_info->time);

    uint16_t local_port = ntohs(p_info->tcp_hdr.destination_port);
    uint16_t foreign_port = ntohs(p_info->tcp_hdr.source_port);

    bool matched = false;
    do {
        p_monitor->stap_cache_lock.lock();
        std::map<uint16_t, net_action_t *>::iterator itr = p_monitor->stap_cache.find(local_port);
        if(itr == p_monitor->stap_cache.end()) {
            p_monitor->stap_cache_lock.unlock();
            break;
        }

        net_action_t *p_net_action = itr->second;

        if(p_net_action == NULL) {
            p_monitor->stap_cache.erase(itr);
            p_monitor->stap_cache_lock.unlock();
            break;
        }
        if(p_net_action->foreign_port != foreign_port) {
            net_action_free(p_net_action);
            p_monitor->stap_cache.erase(itr);
            p_monitor->stap_cache_lock.unlock();
            break;
        }
        p_info->pid = p_net_action->pid;
        p_monitor->stap_cache_lock.unlock();

        debug_print("%s lport:%5d -> fport:%5d pid:%10d",
                    __func__, local_port, foreign_port, p_info->pid);
        if (p_info->p_url) {
            debug_print(" [%s]\n", p_info->p_url);
        }
        else {
            debug_print("\n");
        }
        matched = true;
        // don't free p_info, free will invoked by p_user_data
        if(p_monitor->http_cb) {
            p_monitor->http_cb(p_info, p_monitor->http_params);
        }
    } while(0);

    if (!matched) {
        p_monitor->http_cache_lock.lock();
        p_monitor->http_cache.insert(std::pair<uint16_t, http_info_t *>(local_port, p_info));
        p_monitor->http_cache_lock.unlock();
    }
}

static void refresh_dns_cache(net_monitor_t *p_monitor, time_t now)
{
    if(p_monitor->dns_cache_refresh_prev + p_monitor->refresh_delta > now) {
        return;
    }
    p_monitor->dns_cache_refresh_prev = now;

    int warn = 0, tmout = 0;
    p_monitor->dns_cache_lock.lock();
    for(std::multimap<uint16_t, dns_info_t *>::iterator itr = p_monitor->dns_cache.begin();
            itr != p_monitor->dns_cache.end(); ) {

        dns_info_t *p_info = itr->second;
        if(p_info == NULL) {
            p_monitor->dns_cache.erase(itr++);
            ++warn;
        }
        else if(now >= p_info->time + DEFAULT_TIME_OUT) {
            dns_info_free(p_info);
            p_monitor->dns_cache.erase(itr++);
            ++tmout;
        }
        else {
            ++itr;
        }
    }
    debug_print("%10s warn:%d, tmout:%d, left:%d\n", __func__, warn, tmout, (int)p_monitor->dns_cache.size());
    p_monitor->dns_cache_lock.unlock();
}

static void on_pcap_dns_action(dns_info_t *p_data, void *p_params)
{
    if (NULL == p_params)
        return;
    if (p_data == NULL || p_data->p_dns_query == NULL)
        return;
    dns_info_t *p_info = dns_info_deep_copy(p_data);
    if(p_info == NULL || p_info->p_dns_query == NULL)
        return;

    net_monitor_t *p_monitor = (net_monitor_t*)p_params;
    if (NULL == p_monitor->dns_cb || !p_monitor->running)
        return;

    refresh_dns_cache(p_monitor, p_info->time);

    uint16_t local_port = ntohs(p_info->udp_hdr.destination_port);
    uint16_t foreign_port = ntohs(p_info->udp_hdr.source_port);

    bool matched = false;
    do {
        p_monitor->stap_cache_lock.lock();
        std::map<uint16_t, net_action_t *>::iterator itr = p_monitor->stap_cache.find(local_port);
        if(itr == p_monitor->stap_cache.end()) {
            p_monitor->stap_cache_lock.unlock();
            break;
        }

        net_action_t *p_net_action = itr->second;

        if(p_net_action == NULL) {
            p_monitor->stap_cache.erase(itr);
            p_monitor->stap_cache_lock.unlock();
            break;
        }
        if(p_net_action->foreign_port != foreign_port) {
            // unknown tuple, remove it
            net_action_free(p_net_action);
            p_monitor->stap_cache.erase(itr);
            p_monitor->stap_cache_lock.unlock();
            break;
        }
        p_info->pid = p_net_action->pid;
        p_monitor->stap_cache_lock.unlock();

        p_monitor->dns_cb(p_info, p_monitor->dns_params);
        matched = true;
        debug_print("%s lport:%5d -> fport:%5d pid:%10d [%s]\n",
            __func__, local_port, foreign_port,
            p_info->pid, p_info->p_dns_query);
        // don't free p_info, free will invoked by p_user_data
    } while(0);

    if (!matched) {
        p_monitor->dns_cache_lock.lock();
        p_monitor->dns_cache.insert(std::pair<uint16_t, dns_info_t *>(local_port, p_info));
        p_monitor->dns_cache_lock.unlock();
    }
}

static int assignment_item_array(std::string *item_array, const char *line, const char *tag)
{
    int result = -1;

    std::stringstream line_ss;

    line_ss.str(line + strlen(tag) + 1);

    std::string item;

    for (int i = 0; std::getline(line_ss, item, ' '); )
    {
        if (item.empty())
            continue;

        item_array[i] = item;
        i++;
        result = 0;
    }
    return result;
}

static void catch_stap_row(const char *row_data, uint32_t row_len, void *p_user_data)
{
    net_monitor_t *p_monitor = (net_monitor_t*)p_user_data;
    std::string *item_array = p_monitor->item_array;

    if(!p_monitor->running)
    {
        return;
    }

    if (p_monitor->icmp_cb && strstr(row_data, "ICMP"))
    {
         if (0 != assignment_item_array(item_array, row_data, "ICMP"))
             return;

        icmp_info_t icmp_info;
        memset(&icmp_info, 0, sizeof(icmp_info_t));

        icmp_info.ip_hdr.source_address =
            (uint32_t)inet_addr(item_array[INDEX_ICMP_LOCAL_HOST].c_str());

        icmp_info.ip_hdr.destination_address =
            (uint32_t)inet_addr(item_array[INDEX_ICMP_FOREIGN_HOST].c_str());

        icmp_info.pid = atoi(item_array[INDEX_ICMP_PID].c_str());

        icmp_info.time = atol(item_array[INDEX_ICMP_TIMESTAMP].c_str());

        p_monitor->icmp_cb(icmp_info_deep_copy(&icmp_info), p_monitor->icmp_params);
    }
    if (p_monitor->tcp_cb && strstr(row_data, "TCP"))
    {
        if (0 != assignment_item_array(item_array, row_data, "TCP"))
            return;

        tcp_info_t tcp_info;
        memset(&tcp_info, 0, sizeof(tcp_info_t));

        tcp_info.ip_hdr.source_address =
            (uint32_t)inet_addr(item_array[INDEX_NETWORK_LOCAL_HOST].c_str());

        tcp_info.tcp_hdr.source_port =
            htons((uint16_t)atoi(item_array[INDEX_NETWORK_LOCAL_PORT].c_str()));

        tcp_info.ip_hdr.destination_address =
            (uint32_t)inet_addr(item_array[INDEX_NETWORK_FOREIGN_HOST].c_str());

        tcp_info.tcp_hdr.destination_port =
            htons((uint16_t)atoi(item_array[INDEX_NETWORK_FOREIGN_PORT].c_str()));

        tcp_info.pid = atoi(item_array[INDEX_NETWORK_PID].c_str());

        tcp_info.time = atol(item_array[INDEX_NETWORK_TIMESTAMP].c_str());

        p_monitor->tcp_cb(tcp_info_deep_copy(&tcp_info), p_monitor->tcp_params);
    }
    if (p_monitor->udp_cb && strstr(row_data, "UDP"))
    {
        if (0 != assignment_item_array(item_array, row_data, "UDP"))
            return;

        udp_info_t udp_info;
        memset(&udp_info, 0, sizeof(udp_info_t));

        udp_info.ip_hdr.source_address =
            (uint32_t)inet_addr(item_array[INDEX_NETWORK_LOCAL_HOST].c_str());

        udp_info.udp_hdr.source_port =
            htons((uint16_t)atoi(item_array[INDEX_NETWORK_LOCAL_PORT].c_str()));

        udp_info.ip_hdr.destination_address =
            (uint32_t)inet_addr(item_array[INDEX_NETWORK_FOREIGN_HOST].c_str());

        udp_info.udp_hdr.destination_port =
            htons((uint16_t)atoi(item_array[INDEX_NETWORK_FOREIGN_PORT].c_str()));

        udp_info.pid = atoi(item_array[INDEX_NETWORK_PID].c_str());

        udp_info.time = atol(item_array[INDEX_NETWORK_TIMESTAMP].c_str());

        p_monitor->udp_cb(udp_info_deep_copy(&udp_info), p_monitor->udp_params);
    }
}

LIB_PUBLIC net_monitor_t *init_net_monitor(void)
{
    net_monitor_t *p_monitor = new net_monitor_t;
    assert(p_monitor);

    p_monitor->http_cb          = NULL;
    p_monitor->dns_cb           = NULL;
    p_monitor->tcp_cb           = NULL;
    p_monitor->udp_cb           = NULL;
    p_monitor->icmp_cb          = NULL;

    p_monitor->http_params      = NULL;
    p_monitor->dns_params       = NULL;
    p_monitor->tcp_params       = NULL;
    p_monitor->udp_params       = NULL;
    p_monitor->icmp_params      = NULL;

    p_monitor->running              = false;
    p_monitor->http_cache_refresh_prev  = 0;
    p_monitor->stap_cache_refresh_prev  = 0;
    p_monitor->dns_cache_refresh_prev   = 0;
    p_monitor->refresh_delta            = DEFAULT_TIME_OUT;

    p_monitor->item_array               = new std::string[DEFAULT_ITEM_ARRAY_LEN];

    // register pcap callback
    memset(&p_monitor->notifier_info, 0, sizeof(notifier_info_t));

    p_monitor->notifier_info.notify_http_activity = on_pcap_http_action;
    p_monitor->notifier_info.p_http_params = (void*)p_monitor;

    p_monitor->notifier_info.notify_dns_activity = on_pcap_dns_action;
    p_monitor->notifier_info.p_dns_params = (void*)p_monitor;

    p_monitor->p_libpcap_handle = capture_packet_on_nics(&p_monitor->notifier_info);
    assert(p_monitor->p_libpcap_handle);

    p_monitor->p_stap_monitor = init_stap_monitor(stap_default_exe, stap_default_stp, on_stap_net_action, p_monitor);
    assert(p_monitor->p_stap_monitor);

    set_stap_row_cb(p_monitor->p_stap_monitor, catch_stap_row, (void*)p_monitor);

    return p_monitor;
}

LIB_PUBLIC int run_net_monitor(net_monitor_t *p_monitor)
{
    if (NULL == p_monitor)
        return -1;
    p_monitor->running = true;

    while(p_monitor->running) {
        wait_stap_monitor(p_monitor->p_stap_monitor, NULL);
    }

    return 0;
}

LIB_PUBLIC void stop_net_monitor(net_monitor_t *p_monitor)
{
    if (NULL == p_monitor)
        return;
    p_monitor->running = false;
}

LIB_PUBLIC void fin_net_monitor(net_monitor_t *p_monitor)
{
    if (NULL == p_monitor)
        return;
    capture_packet_stop(p_monitor->p_libpcap_handle);
    fin_stap_monitor(p_monitor->p_stap_monitor);

    if (p_monitor->item_array)
        delete [] p_monitor->item_array;

    delete p_monitor;
}

LIB_PUBLIC int net_monitor_set_http(net_monitor_t *p_monitor, http_monitor_cb_t cb, void *cb_param)
{
    int result = -1;
    if (NULL == p_monitor && NULL == cb)
        return result;

    p_monitor->http_cb      = cb;
    p_monitor->http_params  = cb_param;

    result = 0;
    return result;
}

LIB_PUBLIC int net_monitor_off_http(net_monitor_t *p_monitor)
{
    int result = -1;
    if (NULL == p_monitor)
        return result;

    p_monitor->http_cb      = NULL;
    p_monitor->http_params  = NULL;

    result = 0;
    return result;
}

LIB_PUBLIC int net_monitor_set_dns(net_monitor_t *p_monitor, dns_monitor_cb_t cb, void *cb_param)
{
    int result = -1;
    if (NULL == p_monitor && NULL == cb)
        return result;

    p_monitor->dns_cb       = cb;
    p_monitor->dns_params   = cb_param;

    result = 0;
    return result;
}

LIB_PUBLIC int net_monitor_off_dns(net_monitor_t *p_monitor)
{
    int result = -1;
    if (NULL == p_monitor)
        return result;

    p_monitor->dns_cb      = NULL;
    p_monitor->dns_params  = NULL;

    result = 0;
    return result;
}

LIB_PUBLIC int net_monitor_set_tcp(net_monitor_t *p_monitor, tcp_monitor_cb_t cb, void *cb_param)
{
    int result = -1;
    if (NULL == p_monitor && NULL == cb)
        return result;

    p_monitor->tcp_cb       = cb;
    p_monitor->tcp_params   = cb_param;

    result = 0;
    return result;
}

LIB_PUBLIC int net_monitor_off_tcp(net_monitor_t *p_monitor)
{
    int result = -1;
    if (NULL == p_monitor)
        return result;

    p_monitor->tcp_cb      = NULL;
    p_monitor->tcp_params  = NULL;

    result = 0;
    return result;
}

LIB_PUBLIC int net_monitor_set_udp(net_monitor_t *p_monitor, udp_monitor_cb_t cb, void *cb_param)
{
    int result = -1;
    if (NULL == p_monitor && NULL == cb)
        return result;

    p_monitor->udp_cb       = cb;
    p_monitor->udp_params   = cb_param;

    result = 0;
    return result;
}

LIB_PUBLIC int net_monitor_off_udp(net_monitor_t *p_monitor)
{
    int result = -1;
    if (NULL == p_monitor)
        return result;

    p_monitor->udp_cb       = NULL;
    p_monitor->udp_params   = NULL;

    result = 0;
    return result;
}

LIB_PUBLIC int net_monitor_set_icmp(net_monitor_t *p_monitor, icmp_monitor_cb_t cb, void *cb_param)
{
    int result = -1;
    if (NULL == p_monitor && NULL == cb)
        return result;

    p_monitor->icmp_cb      = cb;
    p_monitor->icmp_params  = cb_param;

    result = 0;
    return result;
}

LIB_PUBLIC int net_monitor_off_icmp(net_monitor_t *p_monitor)
{
    int result = -1;
    if (NULL == p_monitor)
        return result;

    p_monitor->icmp_cb      = NULL;
    p_monitor->icmp_params  = NULL;

    result = 0;
    return result;
}

