#include <map>
#include <string>
#include <time.h>

#include "packet_info.h"
#include "net_history.h"

class net_history_impl {
public:
    void insert(const char *p_url, int pid, time_t tm);
    void clear_timeout(time_t timeout);
    url_cache search(const char *domain);
private:
    std::map<std::string, url_cache> nets;
};

void net_history_impl::insert(const char *p_url, int pid, time_t tm)
{
    nets[p_url] = url_cache(p_url, pid, tm);
}

void net_history_impl::clear_timeout(time_t timeout)
{
    time_t time_s = time(NULL);
    for (std::map<std::string, url_cache>::iterator itr = nets.begin(); itr != nets.end();)
    {
        if(time_s > itr->second.time + timeout)
        {
            nets.erase(itr++);
        }
        else
        {
            itr++;
        }
    }
}

url_cache net_history_impl::search(const char *domain)
{
    for (std::map<std::string, url_cache>::iterator itr = nets.begin(); itr != nets.end(); itr++)
    {
        if(std::string::npos != itr->first.find(domain))
        {
            // printf("net_history_impl::search [%s] -> pid:%d, url:%s\n", 
		    // domain, itr->second.pid, itr->second.url.data());
            return itr->second;
        }
    }
    return url_cache();
}

void net_history::insert(http_info_t *ninfo)
{
    return p_his->insert(ninfo->p_url, ninfo->pid, ninfo->time);
}

void net_history::insert(dns_info_t *dinfo)
{
    return p_his->insert(dinfo->p_dns_query, dinfo->pid, dinfo->time);
}

void net_history::clear_timeout(time_t timeout)
{
    return p_his->clear_timeout(timeout);
}

url_cache net_history::search(const char *domain)
{
    return p_his->search(domain);
}

net_history::net_history() :
    p_his(new net_history_impl)
{
}

net_history::~net_history()
{
    delete p_his;
}
