#ifndef __NET_HISTORY_H__
#define __NET_HISTORY_H__

#include <time.h>
#include "packet_info.h"

struct url_cache {
    url_cache() :
	pid(0), time(0) {}
    url_cache(const char *p_url, int pid, time_t tm) :
        url(p_url), pid(pid), time(tm) {}
    std::string url;
    int pid;
    time_t time;
};

class net_history_impl;

class net_history {
public:
    net_history();
    ~net_history();
    void insert(http_info_t *ninfo);
    void insert(dns_info_t *dinfo);
    void clear_timeout(time_t timeout);
    url_cache search(const char *domain);
private:
    net_history_impl *p_his;
};



#endif /* __NET_HISTORY_H__ */
