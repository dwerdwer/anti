#ifndef CAPTURE_NETWORK_PACKET_HHH
#define CAPTURE_NETWORK_PACKET_HHH

#include <stdint.h>
#include <ctime>
#include <stdlib.h>
#include <string.h>

#include "packet_info.h"

extern "C" {

typedef void (*http_notifier_t)(http_info_t *p_info, void *p_params);
typedef void (*dns_notifier_t)(dns_info_t *p_info, void *p_params);

typedef struct notifier_info
{
    http_notifier_t notify_http_activity;
    void *p_http_params;
    dns_notifier_t notify_dns_activity;
    void *p_dns_params;
}notifier_info_t;

// Return type is 'void*'. This is compatible to run as an isolated thread.
void *capture_packet_on_nics(notifier_info_t *p_notifiers);
void capture_packet_stop(void *p_handle);

}

#endif

