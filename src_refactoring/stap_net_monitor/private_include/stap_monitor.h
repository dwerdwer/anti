#ifndef __STAP_MONITOR_H__
#define __STAP_MONITOR_H__

#include <inttypes.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct {
    int         pid;
    int         ppid;
    uint32_t    local_ip;
    uint16_t    local_port;
    uint32_t    foreign_ip;
    uint16_t    foreign_port;
    time_t      time;
} net_action_t;

enum tokes_network_index {
    INDEX_NETWORK_TIMESTAMP        = 0,
    INDEX_NETWORK_PARENT_PID       = 1,
    INDEX_NETWORK_PID              = 2,
    INDEX_NETWORK_LOCAL_HOST       = 3,
    INDEX_NETWORK_LOCAL_PORT       = 4,
    INDEX_NETWORK_FOREIGN_HOST     = 5,
    INDEX_NETWORK_FOREIGN_PORT     = 6,
    INDEX_NETWORK_MAX
};

enum toks_icmp_index
{
    INDEX_ICMP_TIMESTAMP        = 0,
    INDEX_ICMP_PARENT_PID       = 1,
    INDEX_ICMP_PID              = 2,
    INDEX_ICMP_LOCAL_HOST       = 3,
    INDEX_ICMP_FOREIGN_HOST     = 4,
    INDEX_ICMP_MAX
};


typedef void (*stap_action_cb)(const net_action_t *p_action, void *p_user_data);

typedef void (*stap_row_cb)(const char *row_data, uint32_t row_len, void *p_user_data);

typedef struct stap_monitor stap_monitor_t;

stap_monitor_t *init_stap_monitor(const char *exe, const char *stp, stap_action_cb p_callback, void *p_user_data);

void set_stap_row_cb(stap_monitor_t *p_stap_monitor, stap_row_cb cb, void *row_params);
void off_stap_row_cb(stap_monitor_t *p_stap_monitor);

void wait_stap_monitor(stap_monitor_t *p_stap_monitor, struct timeval *timeout);
void fin_stap_monitor(stap_monitor_t *p_stap_monitor);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __STAP_MONITOR_H__ */
