
#include "capture_network_packet.h"

#include "pcap.h"
#include "dns.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <list>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>

#include <pthread.h>

//#define SHOW_IP
//#define SHOW_UDP_PORT
//#define SHOW_TCP_PORT
#define SHOW_DNS
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
void get_domain_and_url( const u_char *data,
                         int* offset,
                         const int restlength,
                        const char * key){
    for(;*offset<restlength;(*offset)++){
        if(!memcmp(data+*offset,key,strlen(key))){
            break;

        } 
    }
}

//void my_callback(u_char *userless, const struct pcap_pkthdr *pkthdr, 
//                    const u_char *packet)
//{
//   
//    /* map<const char*,netinfo*> *urlmap = new map<const char *,netinfo*>; */
//    netinfo *info = (netinfo *)malloc(sizeof(netinfo));
//    memset(info,0,sizeof(netinfo));
//    char keybuf[128] = {0};
//    info->p_type = -1;
//    info->result = 0;
//    /* n1->protocol = 6; */    
//    /* netinfo n2; */
//    /* n2.protocol = 2; */
//    /* netinfo n3; */
//    /* n3.protocol = 17; */
//    
//    /* urlmap["one"] = n1; */
//    /* urlmap["two"] = n2; */
//    /* urlmap["three"] = n3; */
//    /* char * one = "one"; */
//    /* char * two = "two"; */
//    /* char * three = "three"; */
//    /* urlmap->insert(make_pair(one,n1)); */
//    /* urlmap->insert(make_pair(two,&n2)); */
//    /* urlmap->insert(make_pair(three,&n3)); */
//    /* typedef map<const char *,netinfo *>::iterator IT; */
//    /* for(IT it = urlmap->begin();it != urlmap->end();it++){ */
//    /*     /1* cout << "keyword " << it->first << "=======" << it->second->protocol <<endl; *1/ */
//    /*     printf("key:%s,value:%d\n",it->first,it->second->protocol); */
//
//    /* } */
//    /* urlmap["zs"] = "zhangsan"; */
//    /* urlmap["ls"] = "lisi"; */
//    /* urlmap["ww"] = "wangwu"; */
//    struct in_addr addr;
//    struct iphdr *ipptr;
//    struct tcphdr *tcpptr;//太次片，，ip，tcp数据结构
//    const u_char *data;
//        
//    /* pcap_t *descr = (pcap_t*)userless;//捕获网络数据包的数据包捕获描述字 */
//    //const u_char *packet;
//    struct pcap_pkthdr hdr = *pkthdr;//(libpcap 自定义数据包头部)，
//    struct ether_header *eptr;//以太网字头
//    u_char *ptr;
//    int i;
// 
//    if (packet == NULL)//packet里面有内容，可以证明上面的猜想，
//    {
//        printf ("Didn't grab packet!\n");
//        exit (1);
//    }
//    printf ("\n$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
//    printf ("Grabbed packet of length %d\n", hdr.len);
//    printf ("Received at : %s\n", ctime((const time_t*)&hdr.ts.tv_sec));
//    info->op_time = (time_t)hdr.ts.tv_sec;
//    char the_time[32] = {0};
//    char op_time[32] = {0};
//    int res = strftime(the_time,sizeof(the_time),
//                       "%Y%m%d%H%M%S00000",
//                       localtime((const time_t*)&hdr.ts.tv_sec));
//    res = strftime(op_time,sizeof(the_time),
//                       "%Y-%m-%d %H:%M:%S",
//                       localtime((const time_t*)&hdr.ts.tv_sec));
//    printf("op_time:%s\n",op_time);
//    printf ("formatReceived at : %s\n", the_time);
//    printf ("Ethernet address length is %d\n", ETHER_HDR_LEN);
//    
//    eptr = (struct ether_header*)packet;//得到以太网字头
//    
//    if (ntohs(eptr->ether_type) == ETHERTYPE_IP)
//    {
//        printf ("Ethernet type hex:%x dec:%d is an IP packet\n",
//                    ntohs(eptr->ether_type), ntohs(eptr->ether_type));
//    }
//    else 
//    {
//        if (ntohs(eptr->ether_type) == ETHERTYPE_ARP)
//        {
//            printf ("Ethernet type hex:%x dec:%d is an ARP packet\n",
//                        ntohs(eptr->ether_type), ntohs(eptr->ether_type));
//        }
//        else
//        {
//            printf ("Ethernet type %x not IP\n", ntohs(eptr->ether_type));
//            exit (1);
//        }
//    }
//        
//    ptr = eptr->ether_dhost;
//    i = ETHER_ADDR_LEN;
//    printf ("i=%d\n", i);
//    printf ("Destination Address: ");
//    do
//    {
//        printf ("%s%x", (i == ETHER_ADDR_LEN)?"":":", *ptr++);
//    }while(--i>0);
//    printf ("\n");
//    //printf ("%x\n",ptr);
//    
//    ptr = eptr->ether_shost;
//    i = ETHER_ADDR_LEN;
//    printf ("Source Address: ");
//    do
//    {
//        printf ("%s%x", (i == ETHER_ADDR_LEN)?"":":", *ptr++);
//    }while(--i>0);
//    printf ("\n");
//    printf ("Now decoding the IP packet.\n");
//    ipptr = (struct iphdr*)    (packet+sizeof(struct ether_header));//得到ip包头
//    
//    printf ("the IP packets total_length is :%d\n", ipptr->tot_len);
//    printf ("the IP protocol is %d\n", ipptr->protocol);
//
//    /* char *protocol_name = NULL; */
//    /* if(ipptr->protocol == 6){ */
//    /*     protocol_name ="TCP"; */
//    /* }else if(ipptr->protocol == 17){ */
//    /*     protocol_name = "UDP"; */
//    /* } */
//    /* printf("protocolname:%s\n",protocol_name); */
//    addr.s_addr = ipptr->daddr;
//    printf ("Destination IP: %s\n", inet_ntoa(addr));    
//    info->d_ip = addr;
//    strcat(keybuf,inet_ntoa(addr));
//    strcat(keybuf,"|");
//    addr.s_addr = ipptr->saddr;
//    printf ("Source IP: %s\n", inet_ntoa(addr));
//    info->s_ip = addr;
//    strcat(keybuf,inet_ntoa(addr));
//    strcat(keybuf,"|");
//    
//    printf ("Now decoding the TCP packet.\n");
//    tcpptr = (struct tcphdr*)(packet+sizeof(struct ether_header)
//                                    +sizeof(struct iphdr));//得到tcp包头
//    /* unsigned short dst_port = tcpptr->dest; */
//    printf ("Destination port : %hu\n", ntohs(tcpptr->dest));
//    /* printf ("dst_port : %u\n", dst_port); */
//    info->d_port = ntohs(tcpptr->dest);
//    char dport_str[16] = {0};
//    sprintf(dport_str,"%hu",ntohs(tcpptr->dest));
//    strcat(keybuf,dport_str);
//    strcat(keybuf,"|");
//    printf ("Source port : %hu\n", ntohs(tcpptr->source));
//    info->s_port = ntohs(tcpptr->source);
//    char sport_str[16] = {0};
//    sprintf(sport_str,"%hu",ntohs(tcpptr->source));
//    strcat(keybuf,sport_str);
//    strcat(keybuf,"|");
//    info->protocol = ipptr->protocol;
//    char p_str[16] = {0};
//    sprintf(p_str,"%hu",(ipptr->protocol));
//    printf("strstr3333:%s\n",p_str);
//    strcat(keybuf,p_str);
//    printf("strstr:%s\n",keybuf);
//    printf ("the seq of packet is %u\n", ntohl(tcpptr->seq));
//    printf ("the ack of packet is %u\n", tcpptr->ack);
//    printf ("the doff of packet is %u\n", tcpptr->doff);
////以上关于ip、tcp的结构信息请查询/usr/include/linux/ip.h | tcp.h
//    
//    data = (packet+sizeof(struct ether_header)+sizeof(struct iphdr)
//                                    +sizeof(struct tcphdr));//得到数据包里内容，不过一般为乱码。
//
//    
//
//
//
//    if(ntohs(tcpptr->source)==53){
//        info->p_type = 2;
//        printf("request-----------------------------\n");
//        DNSContext *ctx1 = DNSContextAlloc();
//        ProIPParse((uint8_t *)packet+14, ctx1);
//        debug_dns_ctx(ctx1);
//        DNSContextFree(ctx1);
//        return;
//
//    }
//    if(ntohs(tcpptr->dest)==53){
//        info->p_type = 3;
//        DNSContext *ctx2 = DNSContextAlloc();
//        ProIPParse((uint8_t *)packet+14, ctx2);
//        debug_dns_ctx(ctx2);
//        DNSContextFree(ctx2);
//        return;
//
//    }
//
//    int get_or_post = 0;
//    /* printf("%s",data); */
//    if(!memcmp(data,"GET /",5)){
//        printf("GET\n");
//        /* data += 4; */
//        get_or_post = 1;
//    }else if(!memcmp(data,"POST /",6)){
//        printf("POST\n");
//        /* data += 5; */
//    }else{
//        printf("not get and post");
//        return;
//    }
//    int offset_init = 0;
//    int* offset = &offset_init;
//    /* int restlength = pkthdr->len -sizeof( struct ether_header ) */
//    /*                              -sizeof(struct iphdr) */   
//    /*                              -sizeof(struct tcphdr) */
//    /*                              -5; */
//    int restlength = pkthdr->len - 5;
//    char the_url[4096] = {0};
//    char is_http = 0;
//    /* int get_or_post_offset = 0; */
//    /* int host_offset = 0; */
//    int start_offset;
//    int end_offset;
//    for(;*offset< restlength;(*offset)++){
//        if(!memcmp(data+*offset,"\r\n\r\n",4)){
//            printf("包头结尾 offset:%d\n",*offset);
//            *offset = 0;
//            is_http = 1;
//            break;
//        }
//    }
//    if(!is_http){
//        return ;
//    }
//    info->p_type = 0;
//    /* protocol_name = "HTTP"; */
//    /* printf("protocolname:%s\n",protocol_name); */
//    get_domain_and_url(data, offset,restlength,"Host: ");
//    start_offset = *offset;
//    /* printf("start_offset:%d\n",start_offset); */
//    get_domain_and_url(data, offset,restlength,"\r\n");
//    end_offset = *offset;
//    int length = end_offset-start_offset-strlen("Host: ");
//    char *url = (char *)malloc(length+1);
//    memset(url,0,length+1);
//    memcpy(url,data+start_offset+strlen("Host: "),length);
//    memcpy(the_url,data+start_offset,end_offset-start_offset);
//    /* memcpy(the_url,data,pkthdr->len); */
//    printf("zmhdomain:%s\n",url);
//    printf("zmhdomurl:%s\n",the_url);
//
//
//
//
//
//    *offset = 0;
//    if(get_or_post){
//        get_domain_and_url(data, offset,restlength,"GET ");
//    }else{
//        get_domain_and_url(data, offset,restlength,"POST ");
//    }
//    start_offset = *offset;
//    printf("start_offset:%d\n",start_offset);
//    get_domain_and_url(data, offset,restlength,"\r\n");
//    end_offset = *offset;
//    printf("end_offset:%d\n",end_offset);
//    int the_url_length = end_offset - start_offset - strlen("GET ") - strlen(" HTTP/1.1");
//    url = (char *)realloc(url,length+the_url_length+sizeof(char));
//    /* domain = NULL; */
//    memset(url+length,0,the_url_length+1);
//    if(get_or_post){
//        memcpy(url+length,data+start_offset+strlen("GET "),the_url_length);
//    }else{
//        memcpy(url+length,data+start_offset+strlen("POST "),the_url_length);
//    }
//    /* memcpy(the_url,data,pkthdr->len); */
//    printf("zmh_url:%s\n",url);
//    info->url = url;
//    if(url!=NULL){
//        free(url);
//        url = NULL;
//
//    }
//}


//void * pcap(void *name)
//{
//    printf("pthread:%lu\n",pthread_self());
//    /* printf("getpid:%d:%s\n",getpid(),name); */
//    char errBuf[PCAP_ERRBUF_SIZE]; 
//    /* char * devStr; */
//  
//  /* open a device, wait until a packet arrives */
//  pcap_t * device = pcap_open_live((char *)name, 65535, 0, 0, errBuf);
//  printf("namespace:%s\n",(char *)name);
//  
//  if(!device)
//  {
//    printf("error: pcap_open_live(): %s\n", errBuf);
//    exit(1);
//  }
//  
//  /* construct a filter */
//  struct bpf_program filter;
//  pcap_compile(device, &filter,
//               "tcp or port 53", 1, 0);
//  pcap_setfilter(device, &filter);
//  
//  /* wait loop forever */
//  int id = 0;
//  /* pcap_loop(device, -1, getPacket, (u_char*)&id); */
//  pcap_loop(device, -1, my_callback, (u_char*)&id);
//  /* pcap_loop(device, -1,get_packet, (u_char*)&id); */
//  
//  pcap_close(device);
//
//  return 0;
//}

/* map<const char *,netinfo *> urlmap = new map<const char *,netinfo *>; */
//map<const char* ,netinfo *> *urlmap = new map<const char *,netinfo*>;
//int main (int argc, const char * argv[])
//{
//    struct ifaddrs * ifAddrStruct=NULL;
//    void * tmpAddrPtr=NULL;
//
//    getifaddrs(&ifAddrStruct);
//    char str[512] = {0};
//
//    while (ifAddrStruct!=NULL) {
//        tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
//        char addressBuffer[INET_ADDRSTRLEN];
//        inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
//            /* printf("%s IP Address--> %s\n", ifAddrStruct->ifa_name, addressBuffer); */ 
//        /* if(!(strstr(str,ifAddrStruct->ifa_name))){ */
//        /*     strcat(str,ifAddrStruct->ifa_name); */
//        /*     strcat(str,"|"); */
//        /*     /1* printf("%s IP Address--> %s\n", ifAddrStruct->ifa_name, addressBuffer); *1/ */ 
//        /*     pid_t pid; */
//        /*     //创建子进程 */
//        /*     pid=fork(); */
//        /*     if (pid==-1){//创建子进程失败 */
//        /*         perror("fork"); */
//        /*         return 1; */
//        /*     } */
//        /*     if(pid==0){//子进程的代码 */
//        /*         /1* printf("this is chlid....%d,%d\n",getpid(),getppid()); *1/ */
//        /*         pcap(ifAddrStruct->ifa_name); */
//        /*         ifAddrStruct = NULL; */
//        /*     }else{//父进程的代码 */
//        /*         /1* printf("this is parent...%d\n",getpid()); *1/ */
//        /*         ifAddrStruct=ifAddrStruct->ifa_next; */
//        /*     } */
//        /* }else{ */
//        /*     ifAddrStruct=ifAddrStruct->ifa_next; */
//
//        /* } */
//
//        if(!(strstr(str,ifAddrStruct->ifa_name))){
//            strcat(str,ifAddrStruct->ifa_name);
//            strcat(str,"|");
//            pthread_t tid;
//            pthread_create(&tid,
//                           NULL,
//                           pcap,
//                           (void *)(ifAddrStruct->ifa_name));
//            pthread_detach(tid);
//        }
//        ifAddrStruct=ifAddrStruct->ifa_next;
//
//    }
//    sleep(10000);
//    return 0;
//}

typedef struct http_info
{
    const char *p_url;
    time_t time;
}http_info_t;

typedef std::unordered_map<std::string, std::list<http_info_t> > string_http_list_pair;
//typedef std::unordered_map<std::string, time_t> string_time_pair;
typedef std::map<std::string, time_t> string_time_pair;

typedef struct packet_params
{
    notifier_info_t notifiers;    
    string_http_list_pair conn_http_info_pairs;
    string_time_pair dn_time_pairs;
    time_t last_clean_up_time;
}packet_params_t;

typedef struct nic_capture_params
{
    const char *p_iface_name;
    const void *p_filter_params;    
    packet_params_t packet_params;
    pthread_t thread_id;
}nic_capture_params_t;

static void clean_up_stored_request(packet_params_t *p_packet_params, time_t curr_time)
{
    notifier_info_t *p_notifier_info = &(p_packet_params->notifiers);

    if (CLEAN_UP_TIMEOUT <= (curr_time - p_packet_params->last_clean_up_time))
    {
        // Note: traverse 'conn_http_info_pairs' and 'dn_time_pairs', notify or remove item
        // which is waiting at least 'REQUEST_TIMEOUT' seconds.
        string_time_pair::iterator dn_time_iter = p_packet_params->dn_time_pairs.begin();
        for (; dn_time_iter != p_packet_params->dn_time_pairs.end(); )
        {
            if(REQUEST_TIMEOUT <= (curr_time - dn_time_iter->second))
            {
                dn_time_iter = p_packet_params->dn_time_pairs.erase(dn_time_iter);
            }
            else
            {
                ++dn_time_iter;
            }
        }

        string_http_list_pair::iterator conn_http_iter = p_packet_params->conn_http_info_pairs.begin();
        for (; conn_http_iter != p_packet_params->conn_http_info_pairs.end(); )
        {
            std::list<http_info_t> &url_list = conn_http_iter->second;
            std::list<http_info_t>::iterator url_iter = url_list.begin();
            while(url_list.end() != url_iter)
            {
                http_info_t http_info = *url_iter;

                if(REQUEST_TIMEOUT <= (curr_time - http_info.time))
                {
                    network_info_t network_info;

                    network_info.connection = *(conn_info_t*)(conn_http_iter->first.data());
                    network_info.protocol = NP_HTTP;
                    network_info.p_url = http_info.p_url;
                    network_info.time = http_info.time;
                    network_info.result = 1;

                    p_notifier_info->notify_network_activity(&network_info, 
                            p_notifier_info->p_network_params);
#ifdef SHOW_URL_RESPONSE
                    printf("Responsed url is %s\n", http_info.p_url);
#endif
                    
                    free((void*)http_info.p_url);

                    url_iter = url_list.erase(url_iter);
                }
                else
                {
                    ++url_iter;
                }
            }

            if (true == url_list.empty())
            {
                conn_http_iter = p_packet_params->conn_http_info_pairs.erase(conn_http_iter);
            }
            else
            {
                ++conn_http_iter;
            }
        }

        p_packet_params->last_clean_up_time = curr_time;
    }
}

static network_protocol_t convert_protocol_num(uint8_t protocol)
{
    network_protocol_t result = NP_UNKNOWN;
    switch(protocol)
    {
        case 6:
            result = NP_TCP;
            break;
        case 17:
            result = NP_UDP;
            break;
        default:
            break;
    }
    return result;
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
                // inet_ntop(AF_INET, &source_address, dns_info.connection.source_address, 
                //         sizeof(dns_info.connection.source_address));
                // inet_ntop(AF_INET, &destination_address, dns_info.connection.destination_address, 
                //         sizeof(dns_info.connection.destination_address));
                memcpy(dns_info.connection.destination_address, &(destination_address), sizeof(destination_address));
                memcpy(dns_info.connection.source_address, &(source_address), sizeof(source_address));
                dns_info.connection.destination_port = destination_port;
                dns_info.connection.source_port = source_port;

                dns_info.p_dns_query = query.c_str();
                inet_ntop(AF_INET, &(p_response->addr), dns_info.response_address, 
                        sizeof(dns_info.response_address));
                dns_info.time = citer->second;

                notifier_info_t *p_notifier_info = &(p_params->notifiers);
                p_notifier_info->notify_dns_activity(&dns_info, p_notifier_info->p_dns_params);
#ifdef SHOW_DNS
                char ip_src[32], ip_dst[32];
                inet_ntop(AF_INET, &source_address, ip_src, 
                        sizeof(ip_src));
                inet_ntop(AF_INET, &destination_address, ip_dst, 
                        sizeof(ip_dst));
                printf("DNS [%s:%d] -> [%s:%d] query: %s, response: %s\n", 
                        ip_src, htons(dns_info.connection.source_port),
                        ip_dst, htons(dns_info.connection.destination_port),
                        dns_info.p_dns_query, dns_info.response_address);
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
    network_info_t network_info;
    memset(&network_info, 0, sizeof(network_info));

    clean_up_stored_request(p_packet_params, pkthdr->ts.tv_sec);

    struct in_addr addr;
    struct iphdr *ipptr;
    struct tcphdr *tcpptr;//太次片，，ip，tcp数据结构
    struct udphdr *udpptr;
    const u_char *data;
        
    /* pcap_t *descr = (pcap_t*)userless;//捕获网络数据包的数据包捕获描述字 */
    //const u_char *packet;

    struct pcap_pkthdr hdr = *pkthdr;//(libpcap 自定义数据包头部)，

    struct ether_header *eptr;//以太网字头
    //u_char *ptr;
    //int i;
    //if (packet == NULL)//packet里面有内容，可以证明上面的猜想，
    //{
    //    printf ("Didn't grab packet!\n");
    //    exit (1);
    //}
    //printf ("\n$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
    //printf ("Grabbed packet of length %d\n", hdr.len);
    //printf ("Received at : %s\n", ctime((const time_t*)&hdr.ts.tv_sec));
    //info->op_time = (time_t)hdr.ts.tv_sec;
    //char the_time[32] = {0};
    //char op_time[32] = {0};
    //int res = strftime(the_time,sizeof(the_time),
    //                   "%Y%m%d%H%M%S00000",
    //                   localtime((const time_t*)&hdr.ts.tv_sec));
    //res = strftime(op_time,sizeof(the_time),
    //                   "%Y-%m-%d %H:%M:%S",
    //                   localtime((const time_t*)&hdr.ts.tv_sec));
    //printf("op_time:%s\n",op_time);
    //printf ("formatReceived at : %s\n", the_time);
    //printf ("Ethernet address length is %d\n", ETHER_HDR_LEN);
    
    eptr = (struct ether_header*)packet;//得到以太网字头
    
    if (ntohs(eptr->ether_type) == ETHERTYPE_IP)
    {
        //printf ("Ethernet type hex:%x dec:%d is an IP packet\n",
        //            ntohs(eptr->ether_type), ntohs(eptr->ether_type));
    }
    else 
    {
        if (ntohs(eptr->ether_type) == ETHERTYPE_ARP)
        {
            //printf ("Ethernet type hex:%x dec:%d is an ARP packet\n",
            //            ntohs(eptr->ether_type), ntohs(eptr->ether_type));
            return;
        }
        else
        {
            //printf ("Ethernet type %x not IP\n", ntohs(eptr->ether_type));
            //exit (1);
            return;
        }
    }

    network_info.time = hdr.ts.tv_sec;

    //ptr = eptr->ether_dhost;
    //i = ETHER_ADDR_LEN;
    //printf ("i=%d\n", i);
    //printf ("Destination Address: ");
    //do
    //{
    //    printf ("%s%x", (i == ETHER_ADDR_LEN)?"":":", *ptr++);
    //}while(--i>0);
    //printf ("\n");
    ////printf ("%x\n",ptr);
    //
    //ptr = eptr->ether_shost;
    //i = ETHER_ADDR_LEN;
    //printf ("Source Address: ");
    //do
    //{
    //    printf ("%s%x", (i == ETHER_ADDR_LEN)?"":":", *ptr++);
    //}while(--i>0);
    //printf ("\n");

    //printf ("Now decoding the IP packet.\n");
    ipptr = (struct iphdr*)(packet + sizeof(struct ether_header));//得到ip包头
    //printf ("the IP packets total_length is :%d\n", ipptr->tot_len);
    network_info.protocol = convert_protocol_num(ipptr->protocol);
    //printf ("Protocol is %d\n", ipptr->protocol);

    addr.s_addr = ipptr->daddr;
    memcpy(network_info.connection.destination_address, &(addr.s_addr), sizeof(addr.s_addr));
#ifdef SHOW_IP
    printf ("Destination IP: %s\n", inet_ntoa(addr));    
#endif
    addr.s_addr = ipptr->saddr;
    memcpy(network_info.connection.source_address, &(addr.s_addr), sizeof(addr.s_addr));
#ifdef SHOW_IP
    printf ("Source IP: %s\n", inet_ntoa(addr));
#endif

    uint16_t ip_header_len = ipptr->ihl * sizeof(uint32_t);
    uint16_t ip_total_len = ntohs(ipptr->tot_len);

    if (17 == ipptr->protocol)
    {
        //printf ("Now decoding the UDP packet.\n");
        udpptr = (struct udphdr *)((uint8_t*)ipptr + ip_header_len);

        network_info.connection.destination_port = udpptr->dest;
#ifdef SHOW_UDP_PORT
        printf ("UDP destination port : %hu\n", ntohs(udpptr->dest));
#endif
        network_info.connection.source_port = udpptr->source;
#ifdef SHOW_UDP_PORT
        printf ("UDP source port : %hu\n", ntohs(udpptr->source));
#endif

//        if(53 != ntohs(udpptr->source) && 80 != ntohs(udpptr->dest))
//            return;
//        printf("%s 17 come in\n", __func__);

        uint16_t udp_header_len = sizeof(struct udphdr);
        uint16_t udp_total_len = udpptr->len;
        
        data = (uint8_t*)udpptr + udp_header_len;
        uint16_t data_len = udp_total_len - udp_header_len;

        // For UDP, 'result' is always 0.
        network_info.result = 0;

        p_notifier_info->notify_network_activity(&network_info, 
                p_notifier_info->p_network_params);

        check_dns(p_packet_params, data, data_len, (uint32_t)ipptr->daddr, udpptr->dest, (uint32_t)ipptr->saddr, udpptr->source, hdr.ts.tv_sec);
    }
    else if (6 == ipptr->protocol)
    {
        // TCP
        //printf ("Now decoding the TCP packet.\n");

        tcpptr = (struct tcphdr *)((uint8_t*)ipptr + ip_header_len);

        network_info.connection.destination_port = tcpptr->dest;
#ifdef SHOW_TCP_PORT
        printf ("TCP destination port : %hu\n", ntohs(tcpptr->dest));
#endif
        network_info.connection.source_port = tcpptr->source;
#ifdef SHOW_TCP_PORT
        printf ("TCP source port : %hu\n", ntohs(tcpptr->source));
#endif

//        if(53 != ntohs(tcpptr->source) && 80 != ntohs(tcpptr->dest))
//            return;
//        printf("%s  6 come in\n", __func__);

        //以上关于ip、tcp的结构信息请查询/usr/include/linux/ip.h | tcp.h
        /* For compatibility */
        uint16_t tcp_header_len = tcpptr->doff * sizeof(uint32_t);
        uint16_t tcp_total_len = ip_total_len - ip_header_len;
        
        data = (uint8_t*)tcpptr + tcp_header_len;
        uint16_t data_len = tcp_total_len - tcp_header_len;

        bool is_dns = check_dns(p_packet_params, data, data_len, 
                (uint32_t)ipptr->daddr, tcpptr->dest, (uint32_t)ipptr->saddr, tcpptr->source, hdr.ts.tv_sec);

        if (true == is_dns)
        {
            // For TCP, 'result' is always 0.
            network_info.result = 0;

            p_notifier_info->notify_network_activity(&network_info, 
                    p_notifier_info->p_network_params);
        }
        else
        {
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
                conn_info_t temp = network_info.connection;
                memcpy(temp.source_address, network_info.connection.destination_address, 16);
                memcpy(temp.destination_address, network_info.connection.source_address, 16);
                temp.source_port = network_info.connection.destination_port;
                temp.destination_port = network_info.connection.source_port;

                std::string conn_string((const char*)(&temp), 
                        sizeof(temp));

                string_http_list_pair::iterator iter = 
                    p_packet_params->conn_http_info_pairs.find(conn_string);
                if (p_packet_params->conn_http_info_pairs.end() != iter)
                {
                    std::list<http_info_t> &list = iter->second;
                    while(false ==  list.empty())
                    {
                        http_info_t http_info = list.front();

                        network_info.protocol = NP_HTTP;
                        network_info.p_url = http_info.p_url;
                        network_info.time = http_info.time;
                        network_info.result = 0;

                        p_notifier_info->notify_network_activity(&network_info, 
                                p_notifier_info->p_network_params);
#ifdef SHOW_URL_RESPONSE
                        printf("Responsed url is %s\n", http_info.p_url);
#endif
                        
                        free((void*)http_info.p_url);

                        list.pop_front();
                    }

                    // remove 'iter'
                    p_packet_params->conn_http_info_pairs.erase(iter);
                }
                return;
            }
            else{
                //printf("not get and post\n");
                network_info.result = 0;

                p_notifier_info->notify_network_activity(&network_info, 
                        p_notifier_info->p_network_params);
                return;
            }
            int offset_init = 0;
            int* offset = &offset_init;
            /* int restlength = pkthdr->len -sizeof( struct ether_header ) */
            /*                              -sizeof(struct iphdr) */   
            /*                              -sizeof(struct tcphdr) */
            /*                              -5; */

            int restlength = data_len;

            //char the_url[4096] = {0};
            char is_http = 0;
            /* int get_or_post_offset = 0; */
            /* int host_offset = 0; */
            int start_offset;
            int end_offset;
            for(;*offset< restlength;(*offset)++){
                if(!memcmp(data+*offset,"\r\n\r\n",4)){
                    //printf("message body length:%d\n", data_len - *offset - 4);
                    *offset = 0;
                    is_http = 1;
                    //network_info.protocol = NP_HTTP;
                    break;
                }
            }
            if(!is_http){
                return ;
            }
            /* protocol_name = "HTTP"; */
            /* printf("protocolname:%s\n",protocol_name); */
            get_domain_and_url(data, offset,restlength,"Host: ");
            start_offset = *offset;
            /* printf("start_offset:%d\n",start_offset); */
            get_domain_and_url(data, offset,restlength,"\r\n");
            end_offset = *offset;
            int length = end_offset-start_offset-strlen("Host: ");
            char *url = (char *)malloc(length+1);
            memset(url,0,length+1);
            memcpy(url,data+start_offset+strlen("Host: "),length);
            //memcpy(the_url,data+start_offset,end_offset-start_offset);
            /* memcpy(the_url,data,pkthdr->len); */
            //printf("domain:%s\n",url);
            //printf("domurl:%s\n",the_url);

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
            /* memcpy(the_url,data,pkthdr->len); */
            //printf("url:%s\n",url);

            http_info_t http_info;
            http_info.p_url = url;
            http_info.time = network_info.time;

            std::string conn_string((const char*)(&network_info.connection), 
                    sizeof(network_info.connection));
            p_packet_params->conn_http_info_pairs[conn_string].push_back(http_info);
        }
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
    pcap_t * device = pcap_open_live((char *)p_nic_params->p_iface_name, 65535, 0, 0, errBuf);

    if(!device)
    {
        printf("error: pcap_open_live(): %s\n", errBuf);
        return NULL;
    }

    /* construct a filter */
    struct bpf_program filter;
    pcap_compile(device, &filter, "tcp or port 53", 1, 0);
    pcap_setfilter(device, &filter);

#ifdef PCAP_DEBUG
    printf("pcap_debug %s pcap_loop\n", __func__);
#endif

    /* wait loop forever */
    pcap_loop(device, -1, handle_packet, (u_char*)(&(p_nic_params->packet_params)));

    pcap_close(device);

    return NULL;
}

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
            p_nic_params_array[count].p_filter_params = "tcp or port 53";

            interfaces.erase(iter);
        }

        p_if = p_if->ifa_next;
    }
}

static nic_capture_params_t *create_nic_params(struct ifaddrs *p_if, 
        notifier_info_t *p_notifiers, uint32_t *p_count)
{
    nic_capture_params_t *p_result = NULL;

    std::unordered_set<std::string> interfaces = get_interfaces(p_if);
    *p_count = interfaces.size();
    nic_capture_params_t *p_temp = new nic_capture_params_t[*p_count];

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

void *capture_packet_on_nics(notifier_info_t *p_notifiers)
{
    printf("pcap_debug %s come in\n", __func__);

    struct ifaddrs *ifAddrStruct = NULL;

    getifaddrs(&ifAddrStruct);

    uint32_t count = 0;
    nic_capture_params_t *p_nic_params_array = 
        create_nic_params(ifAddrStruct, p_notifiers, &count);

    // for compatible
//    if (1 < count) count = 1;
    
    printf("pcap_debug %s ethernet count %d.\n", __func__, count);

    while (1 < count)
    {
        --count;
        pthread_create(&(p_nic_params_array[count].thread_id), NULL, 
                capture_packet_on_one_nic, (void *)(p_nic_params_array + count));
        pthread_detach(p_nic_params_array[count].thread_id);
    }

    // This thread can not be ended prematurely.
    if (1 == count)
    {
        p_nic_params_array[0].thread_id = pthread_self();
        capture_packet_on_one_nic(p_nic_params_array);
    }

    delete [] p_nic_params_array;

#ifdef PCAP_DEBUG
    printf("pcap_debug %s count %d\n", __func__, count);
#endif

    return NULL;
}


