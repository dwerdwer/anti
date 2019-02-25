
#include "debug_print.h"
#include "capture_network_packet.h"

#include "pcap.h"
#include "dns.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <string>
#include <list>
#include <map>

#if __cplusplus >= 201103L
#include <map>
#include <unordered_map>
#include <unordered_set>
#else /* __cplusplus >= 201103L */
#include <map>
#include <set>
#endif /* __cplusplus >= 201103L */

#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h> // struct icmphdr
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>

#include <pthread.h>

#define DEFAULT_RUELS   "tcp or port 53"

//#define SHOW_IP
//#define SHOW_UDP_PORT
//#define SHOW_TCP_PORT
//#define SHOW_DNS
//#define SHOW_URL_RESPONSE

#define URL_MAX_LEN        2048
#define MAX_HOST_LEN       1024
#define MAX_GET_LEN        2048

enum
{
    CLEAN_UP_TIMEOUT = 30,
    REQUEST_TIMEOUT = 15,
};

#define get_u_int8_t(X,O)  (*(uint8_t *)(((uint8_t *)X) + O))
#define get_u_int16_t(X,O)  (*(uint16_t *)(((uint8_t *)X) + O))
#define get_u_int32_t(X,O)  (*(uint32_t *)(((uint8_t *)X) + O))
#define get_u_int64_t(X,O)  (*(uint64_t *)(((uint8_t *)X) + O))
/* typedef struct netinfo { */
/*     /1* uint32_t category *1/ */
/*     /1* notify_scheduler_t notifier; *1/ */
/*     /1* void *p_params; *1/ */
/*     uint8_t p_type;//区分http或dns请求包或响应包， http_request:0;http_response:1;dns_resquest:2,dns_response:3 */
/*     uint16_t s_port;//源端口 */
/*     uint16_t d_port;//目的端口 */
/*     in_addr s_ip;//源ip */
/*     in_addr d_ip;//目的ip */
/*     uint8_t protocol;//协议类型，dns分两种（tcp,udp）,或者是http */
/*     char * url;//dns包存域名，http包存url */
/*     char * domain; */
/*     uint8_t result;//操作结果，0成功，1失败 */
/*     time_t op_time;//操作时间 */
/* } netinfo; */

//typedef struct netinfo {
//    /* uint32_t category */
//    /* notify_scheduler_t notifier; */
//    /* void *p_params; */
//    uint8_t p_type;//区分http或dns请求包或响应包， http_request:0;http_response:1;dns_resquest:2,dns_response:3
//    uint16_t s_port;//源端口
//    uint16_t d_port;//目的端口
//    in_addr s_ip;//源ip
//    in_addr d_ip;//目的ip
//    in_addr dns_ip[16];
//    uint8_t protocol;//协议类型，dns分两种（tcp,udp）,或者是http
//    char * url;//dns包存域名，http包存url
//    char * domain;
//    uint8_t result;//操作结果，0成功，1失败
//    time_t op_time;//操作时间
//} netinfo;



//=================================

void show_ethhdr(struct ethhdr *eth)
{
    printf("----------------eth---------------------\n");
    printf("destination eth addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
        eth->h_dest[0], eth->h_dest[1],
        eth->h_dest[2], eth->h_dest[3],
        eth->h_dest[4], eth->h_dest[5]);
    printf("source eth addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
        eth->h_source[0], eth->h_source[1],
        eth->h_source[2], eth->h_source[3],
        eth->h_source[4], eth->h_source[5]);
    printf("protocol is: %04x\n", ntohs(eth->h_proto));
}

/*Display IP Header*/
void show_iphdr(struct iphdr *ip)
{
    struct in_addr addr;

    printf("----------------ip----------------------\n");
    printf("version: %d\n", ip->version);
    printf("head len: %d\n", ip->ihl * 4);
    printf("total len: %d\n", ntohs(ip->tot_len));
    printf("ttl: %d\n", ip->ttl);
    printf("protocol: %d\n", ip->protocol);
    printf("check: %x\n", ip->check);
    addr.s_addr = ip->saddr;
    printf("saddr: %s\n", inet_ntoa(addr));
    addr.s_addr = ip->daddr;
    printf("daddr: %s\n", inet_ntoa(addr));
}

/*Display TCP Header*/
void show_tcphdr(struct tcphdr *tcp)
{
    printf("----------------tcp---------------------\n");
    /* printf("tcp len: %d\n", sizeof(struct tcphdr)); */
    printf("tcp->doff: %d\n", tcp->doff * 4);
    printf("source port: %d\n", ntohs(tcp->source));
    printf("dest port: %d\n", ntohs(tcp->dest));
    printf("sequence number: %d\n", ntohs(tcp->seq));
    printf("ack sequence: %d\n", ntohs(tcp->ack_seq));
}

int parse_http_head(const u_char *payload, int payload_len, char *url)
{
    int line_len, offset;
    /* int ustrlen; */
    int hstrlen; //"host: "
    int hostlen;
    int getlen;
    char host[MAX_HOST_LEN];
    char get[MAX_GET_LEN];
    int a, b;

    /*filter get packet*/
    if(memcmp(payload, "GET ", 4)) {
        return 0;
    }

    for(a = 0, b = 0; a < payload_len - 1; a++) {
        if (get_u_int16_t(payload, a) == ntohs(0x0d0a)) {
            line_len = (u_int16_t)(((unsigned long) &payload[a]) - ((unsigned long)&payload[b]));

            if (line_len >= (9 + 4)
                && memcmp(&payload[line_len - 9], " HTTP/1.", 8) == 0) {
                memcpy(get, payload + 4, line_len - 13); //"GET  HTTP/1.x" 13bit
                getlen = line_len - 13;
            }
            /*get url host of pcaket*/
            if (line_len > 6
                && memcmp(&payload[b], "Host:", 5) == 0) {
                if(*(payload + b + 5) == ' ') {
                    hstrlen = b + 6;
                } else {
                    hstrlen = b + 5;
                }
                hostlen = a - hstrlen;
                memcpy(host, payload + hstrlen, (a - hstrlen));
            }
            b = a + 2;
        }
    }
    offset =  7;
    memcpy(url, "http://", offset);
    memcpy(url + offset, host, hostlen);
    offset += hostlen;
    memcpy(url + offset, get, getlen);

    return strlen(url);
}

void packet_http_handle(const u_char *tcp_payload, int payload_len)
{
    int url_len;
    char url[URL_MAX_LEN];

    url_len = parse_http_head(tcp_payload, payload_len, url);
    if (url_len <= 7) {
        return;
    }
    printf("----------------HTTP---------------------\n");
    printf("url_len: %d\n", url_len);
    printf("url: %s\n", url);
}

int prase_packet(const u_char *buf,  int caplen)
{
    uint16_t e_type;
    uint32_t offset;
    int payload_len;
    const u_char *tcp_payload;

    /* ether header */
    struct ethhdr *eth = NULL;
    eth = (struct ethhdr *)buf;
    e_type = ntohs(eth->h_proto);
    offset = sizeof(struct ethhdr);
    show_ethhdr(eth);

    /*vlan 802.1q*/
    while(e_type == ETH_P_8021Q) {
        e_type = (buf[offset+2] << 8) + buf[offset+3];
        offset += 4;
    }
    if (e_type != ETH_P_IP) {
        return -1;
    }

    /* ip header */
    struct iphdr *ip = (struct iphdr *)(buf + offset);
    e_type = ntohs(ip->protocol);
    offset += sizeof(struct iphdr);
    show_iphdr(ip);

    if(ip->protocol != IPPROTO_TCP) {
        return -1;
    }

    /*tcp header*/
    struct tcphdr *tcp = (struct tcphdr *)(buf + offset);
    offset += (tcp->doff << 2);
    payload_len = caplen - offset;
    tcp_payload = (buf + offset);
    show_tcphdr(tcp);

    /*prase http header*/
    packet_http_handle(tcp_payload, payload_len);

    return 0;
}

void get_packet(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packet)
{
    static int count = 0;
    printf("\n----------------------------------------\n");
    printf("\t\tpacket %d\n", count);
    printf("----------------------------------------\n");
    printf("Packet id: %d\n", count);
    printf("Packet length: %d\n", pkthdr->len);
    printf("Number of bytes: %d\n", pkthdr->caplen);
      printf("Recieved time: %s\n", ctime((const time_t *)&pkthdr->ts.tv_sec));

    prase_packet(packet, pkthdr->len);
    count++;
}
//=================================
void getPacket(u_char * arg, const struct pcap_pkthdr * pkthdr, const u_char * packet)
{
  int * id = (int *)arg;

  printf("id: %d\n", ++(*id));
  printf("Packet length: %d\n", pkthdr->len);
  printf("Number of bytes: %d\n", pkthdr->caplen);
  printf("Recieved time: %s", ctime((const time_t *)&pkthdr->ts.tv_sec));

  uint32_t i;
  for(i=0; i<pkthdr->len; ++i)
  {
    printf(" %02x", packet[i]);
    if( (i + 1) % 16 == 0 )
    {
      printf("\n");
    }
  }

  printf("\n\n");
}
void get_domain_and_url( const u_char *data, int* offset,
                         const int restlength, const char * key)
{
    for(;*offset<restlength;(*offset)++){
        if(!memcmp(data+*offset,key,strlen(key))){
            break;
        }
    }
}

typedef struct priv_http
{
    const char *p_url;
    time_t time;
}priv_http_t;

#if __cplusplus >= 201103L
typedef std::unordered_map<std::string, std::list<priv_http_t> > string_http_list_pair;
#else /* __cplusplus >= 201103L */
typedef std::map<std::string, std::list<priv_http_t> > string_http_list_pair;
#endif /* __cplusplus >= 201103L */

//typedef std::unordered_map<std::string, time_t> string_time_pair;
typedef std::map<std::string, time_t> string_time_pair;

struct packet_params_t {
    notifier_info_t notifiers;
    string_http_list_pair conn_priv_http_pairs;
    string_time_pair dn_time_pairs;
    time_t last_clean_up_time;
};

struct nic_capture_params_t {
    std::string p_iface_name;
    std::string filter_rules;
    packet_params_t packet_params;
    pthread_t thread_id;
};

static void clean_up_stored_request(packet_params_t *p_packet_params, time_t curr_time)
{
    notifier_info_t *p_notifier_info = &(p_packet_params->notifiers);

    if (CLEAN_UP_TIMEOUT <= (curr_time - p_packet_params->last_clean_up_time))
    {
        // Note: traverse 'conn_priv_http_pairs' and 'dn_time_pairs', notify or remove item
        // which is waiting at least 'REQUEST_TIMEOUT' seconds.
        string_time_pair::iterator dn_time_iter = p_packet_params->dn_time_pairs.begin();
        for (; dn_time_iter != p_packet_params->dn_time_pairs.end(); )
        {
            if(REQUEST_TIMEOUT <= (curr_time - dn_time_iter->second))
            {
                p_packet_params->dn_time_pairs.erase(dn_time_iter++);
            }
            else
            {
                ++dn_time_iter;
            }
        }

        string_http_list_pair::iterator conn_http_iter = p_packet_params->conn_priv_http_pairs.begin();
        for (; conn_http_iter != p_packet_params->conn_priv_http_pairs.end(); )
        {
            std::list<priv_http_t> &url_list = conn_http_iter->second;
            std::list<priv_http_t>::iterator url_iter = url_list.begin();
            while(url_list.end() != url_iter)
            {
                priv_http_t priv_http = *url_iter;

                if(REQUEST_TIMEOUT <= (curr_time - priv_http.time))
                {
                    http_info_t http_info;

                    http_info.ip_hdr = *(ip_hdr_t*)(conn_http_iter->first.data());
                    http_info.p_url = priv_http.p_url;
                    http_info.time = priv_http.time;

                    p_notifier_info->notify_http_activity(&http_info,
                            p_notifier_info->p_http_params);
#ifdef SHOW_URL_RESPONSE
                    printf("Responsed url is %s\n", priv_http.p_url);
#endif
                    free((void*)priv_http.p_url);

                    url_iter = url_list.erase(url_iter);
                }
                else
                {
                    ++url_iter;
                }
            }

            if (true == url_list.empty())
            {
                p_packet_params->conn_priv_http_pairs.erase(conn_http_iter++);
            }
            else
            {
                ++conn_http_iter;
            }
        }

        p_packet_params->last_clean_up_time = curr_time;
    }
}

static void checked_dns_query(packet_params_t *p_params, DNSContext *p_dns_info,
                uint32_t destination_address, uint16_t destination_port, uint32_t source_address, uint16_t source_port)
{
    DNSResponse *p_response = NULL;

    TAILQ_FOREACH(p_response, &(p_dns_info->responses), next)
    {
        if (sizeof(uint32_t) == p_response->dataLen)
        {
            // A IPv4 address
            std::string query(p_response->queryer->name, p_response->queryer->nameLen);
            string_time_pair::const_iterator citer = p_params->dn_time_pairs.find(query);

            if (p_params->dn_time_pairs.end() != citer)
            {
                dns_info_t dns_info;

                dns_info.ip_hdr.destination_address = destination_address;
                dns_info.ip_hdr.source_address = source_address;
                dns_info.udp_hdr.destination_port = destination_port;
                dns_info.udp_hdr.source_port = source_port;

                dns_info.p_dns_query = query.c_str();
                dns_info.response_address = p_response->addr;
                dns_info.time = citer->second;

                notifier_info_t *p_notifier_info = &(p_params->notifiers);
                p_notifier_info->notify_dns_activity(&dns_info, p_notifier_info->p_dns_params);
#ifdef SHOW_DNS
                char ip_src[32], ip_dst[32], ip_resp[32];
                inet_ntop(AF_INET, &source_address, ip_src,
                        sizeof(ip_src));
                inet_ntop(AF_INET, &destination_address, ip_dst,
                        sizeof(ip_dst));
                inet_ntop(AF_INET, &p_response->addr, ip_resp,
                        sizeof(ip_resp));
                printf("DNS [%s:%d] -> [%s:%d] query: %s, response: %s\n",
                        ip_src, htons(source_port),
                        ip_dst, htons(destination_port),
                        query.c_str(), ip_resp);
#endif
            }

            break;  // Only use the first IP address.
        }
    }
}

static bool check_source(packet_params_t *p_params, const uint8_t *p_data,
        uint32_t data_len, uint32_t destination_address, uint16_t destination_port, uint32_t source_address, uint16_t source_port)
{
    bool result = false;
    if (53 == ntohs(source_port))
    {
        // DNS response
        DNSContext *ctx1 = DNSContextAlloc();
        ProDNSParse((uint8_t *)p_data, data_len, ctx1);

        checked_dns_query(p_params, ctx1, destination_address, destination_port, source_address, source_port);

        DNSContextFree(ctx1);
        result = true;
    }
    return result;
}

static void store_dns_query(string_time_pair &dn_time_pairs, DNSContext *p_dns_info,
        time_t time)
{
    DNSQuery *p_query = NULL;

    TAILQ_FOREACH(p_query, &(p_dns_info->querys), next)
    {
        std::string query_string(p_query->name, p_query->nameLen);
        dn_time_pairs[query_string] = time;
    }
}

static bool check_destination(packet_params_t *p_params, const uint8_t *p_data,
        uint32_t data_len, uint32_t destination_address, uint16_t destination_port, uint32_t source_address, uint16_t source_port, time_t time)
{
    bool result = false;
    if (53 == ntohs(destination_port))
    {
        // DNS query
        DNSContext *ctx1 = DNSContextAlloc();
        ProDNSParse((uint8_t *)p_data, data_len, ctx1);

        store_dns_query(p_params->dn_time_pairs, ctx1, time);

        DNSContextFree(ctx1);

        result = true;
    }
    return result;
}

static bool check_dns(packet_params_t *p_params, const uint8_t *p_data, uint32_t data_len,
        uint32_t destination_address, uint16_t destination_port, uint32_t source_address, uint16_t source_port, time_t time)
{
    bool is_destination = check_destination(p_params, p_data, data_len, destination_address, destination_port, source_address, source_port, time);
    bool is_source = check_source(p_params, p_data, data_len, destination_address, destination_port, source_address, source_port);
    return (is_destination || is_source) ? true : false;
}

static void handle_packet(u_char *p_user,
        const struct pcap_pkthdr *pkthdr, const u_char *packet)
{
    packet_params_t *p_packet_params = (packet_params_t*)p_user;
    notifier_info_t *p_notifier_info = &(p_packet_params->notifiers);
    http_info_t http_info;
    memset(&http_info, 0, sizeof(http_info));

    clean_up_stored_request(p_packet_params, pkthdr->ts.tv_sec);

    http_notifier_t notify_http_cb = p_notifier_info->notify_http_activity;

    struct in_addr addr;
    struct iphdr *ipptr;
    struct tcphdr *tcpptr;
    struct udphdr *udpptr;
    const u_char *data;

    struct pcap_pkthdr hdr = *pkthdr;//(libpcap 自定义数据包头部)，

    struct ether_header *eptr = (struct ether_header*)packet;//以太网字头

    uint16_t ether_type = ntohs(eptr->ether_type);

    if (ETHERTYPE_IP != ether_type) {
        return;
    }
    http_info.time = hdr.ts.tv_sec;

    //printf ("Now decoding the IP packet.\n");
    ipptr = (struct iphdr*)(packet + sizeof(struct ether_header));//得到ip包头
    //printf ("the IP packets total_length is :%d\n", ipptr->tot_len);
    //printf ("Protocol is %d\n", ipptr->protocol);

    addr.s_addr = ipptr->daddr;
    http_info.ip_hdr.destination_address = addr.s_addr;
#ifdef SHOW_IP
    printf ("Destination IP: %s\n", inet_ntoa(addr));
#endif
    addr.s_addr = ipptr->saddr;
    http_info.ip_hdr.source_address = addr.s_addr;
#ifdef SHOW_IP
    printf ("Source IP: %s\n", inet_ntoa(addr));
#endif

    uint16_t ip_header_len = ipptr->ihl * sizeof(uint32_t);
    uint16_t ip_total_len = ntohs(ipptr->tot_len);

    if (17 == ipptr->protocol) // UDP
    {
        udpptr = (struct udphdr *)((uint8_t*)ipptr + ip_header_len);

        uint16_t udp_header_len = sizeof(struct udphdr);
        uint16_t udp_total_len = udpptr->len;

        data = (uint8_t*)udpptr + udp_header_len;
        uint16_t data_len = udp_total_len - udp_header_len;

        check_dns(p_packet_params, data, data_len, (uint32_t)ipptr->daddr,
                udpptr->dest, (uint32_t)ipptr->saddr, udpptr->source, hdr.ts.tv_sec);
    }
    else if (6 == ipptr->protocol) // TCP
    {
        tcpptr = (struct tcphdr *)((uint8_t*)ipptr + ip_header_len);

        http_info.tcp_hdr.destination_port = tcpptr->dest;
#ifdef SHOW_TCP_PORT
        printf ("TCP destination port : %hu\n", ntohs(tcpptr->dest));
#endif
        http_info.tcp_hdr.source_port = tcpptr->source;
#ifdef SHOW_TCP_PORT
        printf ("TCP source port : %hu\n", ntohs(tcpptr->source));
#endif
        /* For compatibility */
        uint16_t tcp_header_len = tcpptr->doff * sizeof(uint32_t);
        uint16_t tcp_total_len = ip_total_len - ip_header_len;

        data = (uint8_t*)tcpptr + tcp_header_len;
        uint16_t data_len = tcp_total_len - tcp_header_len;

        bool is_dns = check_dns(p_packet_params, data, data_len,
                (uint32_t)ipptr->daddr, tcpptr->dest, (uint32_t)ipptr->saddr, tcpptr->source, hdr.ts.tv_sec);

        if (true == is_dns)
        {
            //notify_http_cb(&http_info, p_notifier_info->p_http_params);
            return;
        }
        // Check if HTTP
        int get_or_post = 0;
        /* printf("%s",data); */
        if(!memcmp(data,"GET /",5))
        {
            //printf("GET\n");
            /* data += 4; */
            get_or_post = 1;
        }
        else if(!memcmp(data,"POST /",6))
        {
            //printf("POST\n");
            /* data += 5; */
        }
        else if (0 == memcmp(data, "HTTP/", 5))
        {
            // HTTP response
            // Note: exchange source and destination.
            ip_hdr_t temp;
            temp.source_address = http_info.ip_hdr.destination_address;
            temp.destination_address = http_info.ip_hdr.source_address;

            std::string conn_string((const char*)(&temp), sizeof(temp));

            string_http_list_pair::iterator iter =
                p_packet_params->conn_priv_http_pairs.find(conn_string);
            if (p_packet_params->conn_priv_http_pairs.end() != iter)
            {
                std::list<priv_http_t> &list = iter->second;
                while(false ==  list.empty())
                {
                    priv_http_t priv_http = list.front();

                    http_info.p_url = priv_http.p_url;
                    http_info.time = priv_http.time;

                    notify_http_cb(&http_info, p_notifier_info->p_http_params);
#ifdef SHOW_URL_RESPONSE
                    printf("Responsed url is %s\n", priv_http.p_url);
#endif

                    free((void*)priv_http.p_url);

                    list.pop_front();
                }
                // remove 'iter'
                p_packet_params->conn_priv_http_pairs.erase(iter);
            }
            return;
        }
        else {
#ifdef OLD_WAY // It may need to norified
            //notify_http_cb(&http_info, p_notifier_info->p_http_params);
#endif
            return;
        }
        /* The following code is for handling POST and GET */
        int offset_init = 0;
        int* offset = &offset_init;

        int restlength = data_len;

        char is_http = 0;
        int start_offset;
        int end_offset;
        for(;*offset< restlength;(*offset)++){
            if(!memcmp(data+*offset,"\r\n\r\n",4)){
                //printf("message body length:%d\n", data_len - *offset - 4);
                *offset = 0;
                is_http = 1;
                break;
            }
        }
        if(!is_http){
            return ;
        }
        get_domain_and_url(data, offset,restlength,"Host: ");
        start_offset = *offset;
        get_domain_and_url(data, offset,restlength,"\r\n");
        end_offset = *offset;
        int length = end_offset-start_offset-strlen("Host: ");
        char *url = (char *)malloc(length+1);
        memset(url,0,length+1);
        memcpy(url,data+start_offset+strlen("Host: "),length);

        *offset = 0;
        if(get_or_post){
            get_domain_and_url(data, offset,restlength,"GET ");
        }else{
            get_domain_and_url(data, offset,restlength,"POST ");
        }
        start_offset = *offset;
        //printf("start_offset:%d\n",start_offset);
        get_domain_and_url(data, offset,restlength,"\r\n");
        end_offset = *offset;
        //printf("end_offset:%d\n",end_offset);
        int the_url_length = end_offset - start_offset - strlen("GET ") - strlen(" HTTP/1.1");
        url = (char *)realloc(url,length+the_url_length+sizeof(char));
        /* domain = NULL; */
        memset(url+length,0,the_url_length+1);
        if(get_or_post){
            memcpy(url+length,data+start_offset+strlen("GET "),the_url_length);
        }else{
            memcpy(url+length,data+start_offset+strlen("POST "),the_url_length);
        }

        priv_http_t priv_http;
        priv_http.p_url = url;
        priv_http.time = http_info.time;

        std::string conn_string((const char*)(&http_info.ip_hdr),
                                sizeof(http_info.ip_hdr));
        p_packet_params->conn_priv_http_pairs[conn_string].push_back(priv_http);
    }
    else
    {
        // Do nothing.
    }
}

static void *capture_packet_on_one_nic(void *p_params)
{
    nic_capture_params_t *p_nic_params = (nic_capture_params_t*)p_params;
    char errBuf[PCAP_ERRBUF_SIZE];

    /* open a device, wait until a packet arrives */
    pcap_t * device = pcap_open_live(p_nic_params->p_iface_name.c_str(), 65535, 0, 0, errBuf);

    if(!device)
    {
        printf("error: pcap_open_live(): %s\n", errBuf);
        return NULL;
    }

    /* construct a filter */
    struct bpf_program filter;
    if (-1 == pcap_compile(device, &filter, p_nic_params->filter_rules.c_str(), 1, 0))
    {
        debug_print("pcap_compile error: %s\n", pcap_geterr(device));
    }
    else {
        pcap_setfilter(device, &filter);
    }
    debug_print("pcap_debug %s pcap_loop\n", __func__);


    /* wait loop forever */
    pcap_loop(device, -1, handle_packet, (u_char*)(&(p_nic_params->packet_params)));

    pcap_close(device);

    return NULL;
}

#if __cplusplus >= 201103L
static std::unordered_set<std::string> get_interfaces(struct ifaddrs *p_if)
{
    std::unordered_set<std::string> result;
    char host[NI_MAXHOST];
    for(;NULL != p_if; p_if=p_if->ifa_next)
    {
        if (p_if->ifa_addr == NULL)
            continue;
        int family = p_if->ifa_addr->sa_family;
        /* family can be one of[AF_PACKET, AF_INET, AF_INET6] */
        if(AF_INET != family)
            continue;

        /* get ethernet IP string such as '127.0.0.1' */
        int s = getnameinfo(p_if->ifa_addr, (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
        if(0 != s) {
            printf("getnameinfo() failed: %s\n", gai_strerror(s));
            continue;
        }

        // Note: not collect packet from 'lo'
        if (0 == strcmp("127.0.0.1", host)) {
            printf("looper ethernet is ignore\n");
            continue;
        }

        result.insert(std::string(p_if->ifa_name));
        printf("%s get ethernet[%s]:[%s]\n", __func__, p_if->ifa_name, host);
    }
    return result;
}

static void initialize_nic_params(nic_capture_params_t *p_nic_params_array, uint32_t count,
        std::unordered_set<std::string> &interfaces, struct ifaddrs *p_if,
        notifier_info_t *p_notifiers)
{
    const char *rules = DEFAULT_RUELS;

    while (NULL != p_if && 0 < count)
    {
        std::unordered_set<std::string>::iterator iter =
            interfaces.find(std::string(p_if->ifa_name));
        if (interfaces.end() != iter)
        {
            --count;
            p_nic_params_array[count].packet_params.notifiers = *p_notifiers;
            p_nic_params_array[count].packet_params.last_clean_up_time = time(NULL);
            // Note: initialize the data member of packet_params_t here.

            p_nic_params_array[count].p_iface_name = p_if->ifa_name;
            p_nic_params_array[count].filter_rules = rules;

            interfaces.erase(iter);
            debug_print("%d: device:%s\n", count, p_nic_params_array[count].p_iface_name.data());
        }

        p_if = p_if->ifa_next;
    }
}

#else /* __cplusplus >= 201103L */

static std::set<std::string> get_interfaces(struct ifaddrs *p_if)
{
    std::set<std::string> result;
    char host[NI_MAXHOST];
    for(;NULL != p_if; p_if=p_if->ifa_next)
    {
        if (p_if->ifa_addr == NULL)
            continue;
        int family = p_if->ifa_addr->sa_family;
        /* family can be one of[AF_PACKET, AF_INET, AF_INET6] */
        if(AF_INET != family)
            continue;

        /* get ethernet IP string such as '127.0.0.1' */
        int s = getnameinfo(p_if->ifa_addr, (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
        if(0 != s) {
            printf("getnameinfo() failed: %s\n", gai_strerror(s));
            continue;
        }

        // Note: not collect packet from 'lo'
        if (0 == strcmp("127.0.0.1", host)) {
            printf("looper ethernet is ignore\n");
            continue;
        }

        result.insert(std::string(p_if->ifa_name));
        debug_print("%s get ethernet[%s]:[%s]\n", __func__, p_if->ifa_name, host);
    }
    return result;
}

static void initialize_nic_params(nic_capture_params_t *p_nic_params_array, uint32_t count,
        std::set<std::string> &interfaces, struct ifaddrs *p_if,
        notifier_info_t *p_notifiers)
{
    const char *rules = DEFAULT_RUELS;

    while (NULL != p_if && 0 < count)
    {
        std::set<std::string>::iterator iter =
            interfaces.find(std::string(p_if->ifa_name));
        if (interfaces.end() != iter)
        {
            --count;
            p_nic_params_array[count].packet_params.notifiers = *p_notifiers;
            p_nic_params_array[count].packet_params.last_clean_up_time = time(NULL);
            // Note: initialize the data member of packet_params_t here.

            p_nic_params_array[count].p_iface_name = p_if->ifa_name;
            p_nic_params_array[count].filter_rules = rules;

            interfaces.erase(iter);
            debug_print("%d: device:%s\n", count, p_nic_params_array[count].p_iface_name.data());
        }
        p_if = p_if->ifa_next;
    }
}

#endif /* __cplusplus >= 201103L */


static nic_capture_params_t *create_nic_params(struct ifaddrs *p_if,
        notifier_info_t *p_notifiers, uint32_t *p_count)
{
    nic_capture_params_t *p_result = NULL;

#if __cplusplus >= 201103L
    std::unordered_set<std::string> interfaces = get_interfaces(p_if);
#else /* __cplusplus >= 201103L */
    std::set<std::string> interfaces = get_interfaces(p_if);
#endif /* __cplusplus >= 201103L */
    *p_count = interfaces.size();
    nic_capture_params_t *p_temp = new nic_capture_params_t[(*p_count)+1];
    p_temp[(*p_count)].thread_id = 0;

    if (0 < *p_count && NULL != p_temp)
    {
        initialize_nic_params(p_temp, *p_count, interfaces, p_if, p_notifiers);
        p_result = p_temp;
    }
    else
    {
        delete [] p_temp;
    }

    return p_result;
}

void capture_packet_stop(void *p_handle)
{
    nic_capture_params_t *p_nic_params_array = (nic_capture_params_t *)p_handle;
    nic_capture_params_t *iter = p_nic_params_array;

    while(iter->thread_id) {
        pthread_cancel(iter->thread_id);
        pthread_join(iter->thread_id, NULL);

        iter++;
    }

    delete []p_nic_params_array;
}

void *capture_packet_on_nics(notifier_info_t *p_notifiers)
{
    printf("pcap_debug %s come in\n", __func__);

    struct ifaddrs *ifAddrStruct = NULL;

    getifaddrs(&ifAddrStruct);

    uint32_t count = 0;
    nic_capture_params_t *p_nic_params_array =
        create_nic_params(ifAddrStruct, p_notifiers, &count);

    freeifaddrs(ifAddrStruct);

    debug_print("pcap_debug %s ethernet count %d.\n", __func__, count);

    while (count > 0)
    {
        --count;
        pthread_create(&(p_nic_params_array[count].thread_id), NULL,
                capture_packet_on_one_nic, (void *)(p_nic_params_array + count));
        // pthread_detach(p_nic_params_array[count].thread_id);
    }

    // This thread can not be ended prematurely.
    // if (1 == count)
    // {
    //     p_nic_params_array[0].thread_id = pthread_self();
    //     capture_packet_on_one_nic(p_nic_params_array);
    // }


#ifdef PCAP_DEBUG
    printf("pcap_debug %s count %d\n", __func__, count);
#endif

    return p_nic_params_array;
}

http_info_t *http_info_deep_copy(const http_info_t *p_src_info)
{
    http_info_t *p_info = (http_info_t *)calloc(1, sizeof(http_info_t));
    if(p_info) {
        memcpy(p_info, p_src_info, sizeof(http_info_t));
        if (p_src_info->p_url)
            p_info->p_url = strdup(p_src_info->p_url);
    }
    return p_info;
}

void http_info_free(http_info_t *p_info)
{
    if(p_info) {
        if(p_info->p_url)
            free((char *)p_info->p_url);
        free(p_info);
    }
}

dns_info_t *dns_info_deep_copy(const dns_info_t *p_src_info)
{
    dns_info_t *p_info = (dns_info_t *)calloc(1, sizeof(dns_info_t));
    if(p_info) {
        memcpy(p_info, p_src_info, sizeof(dns_info_t));
        p_info->p_dns_query = strdup(p_src_info->p_dns_query);
    }
    return p_info;
}

void dns_info_free(dns_info_t *p_info)
{
    if(p_info) {
        if(p_info->p_dns_query)
            free((char *)p_info->p_dns_query);
        free(p_info);
    }
}

icmp_info_t *icmp_info_deep_copy(const icmp_info_t *p_src_info)
{
    icmp_info_t *p_info = (icmp_info_t *)calloc(1, sizeof(icmp_info_t));
    if(p_info) {
        memcpy(p_info, p_src_info, sizeof(icmp_info_t));
    }
    return p_info;
}

void icmp_info_free(icmp_info_t *p_info)
{
    if(p_info) {
        free(p_info);
    }
}

tcp_info_t *tcp_info_deep_copy(const tcp_info_t *p_src_info)
{
    tcp_info_t *p_info = (tcp_info_t *)calloc(1, sizeof(tcp_info_t));
    if(p_info) {
        memcpy(p_info, p_src_info, sizeof(tcp_info_t));
    }
    return p_info;
}

void tcp_info_free(tcp_info_t *p_info)
{
    if(p_info) {
        free(p_info);
    }
}

udp_info_t *udp_info_deep_copy(const udp_info_t *p_src_info)
{
    udp_info_t *p_info = (udp_info_t *)calloc(1, sizeof(udp_info_t));
    if(p_info) {
        memcpy(p_info, p_src_info, sizeof(udp_info_t));
    }
    return p_info;
}

void udp_info_free(udp_info_t *p_info)
{
    if(p_info) {
        free(p_info);
    }
}
