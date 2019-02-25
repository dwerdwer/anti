
#include "capture_network_packet.h"

#include <stdint.h>


void notify_network_activity(network_info_t *p_info, void *p_params)
{

}

void notify_dns_activity(dns_info_t *p_info, void *p_params)
{

}

int32_t main(void)
{
    notifier_info_t notifiers;
    notifiers.notify_network_activity = notify_network_activity;
    notifiers.p_network_params = NULL;
    notifiers.notify_dns_activity = notify_dns_activity;
    notifiers.p_dns_params = NULL;

    capture_packet_on_nics(&notifiers);

    return 0;
}

