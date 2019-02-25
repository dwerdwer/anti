#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "netstat.h"
#include "util-system.h"
#include "packet_info.h"

#define PATH_PROC_NET_TCP   "/proc/net/tcp"
#define PATH_PROC_NET_UDP   "/proc/net/udp"
#define PATH_PROC_NET_TCP6  "/proc/net/tcp6"
#define PATH_PROC_NET_UDP6  "/proc/net/udp6"

const char *proto2str(int type)
{
    switch (type)
    {
        case PROTO_TCP:     return "PROTO_TCP";
        case PROTO_TCP6:    return "PROTO_TCP6";
        case PROTO_UDP:     return "PROTO_UDP";
        case PROTO_UDP6:    return "PROTO_UDP6";
    }
    return "unknown";
}

const char *state2str(int state)
{
    switch (state)
    {
        case TCP_ESTABLISHED:   return "ESTABLISHED";
        case TCP_SYN_SENT:      return "SYN_SENT";
        case TCP_SYN_RECV:      return "SYN_RECV";
        case TCP_FIN_WAIT1:     return "FIN_WAIT1";
        case TCP_FIN_WAIT2:     return "FIN_WAIT2";
        case TCP_TIME_WAIT:     return "TIME_WAIT";
        case TCP_CLOSE:         return "CLOSE";
        case TCP_CLOSE_WAIT:    return "CLOSE_WAIT";
        case TCP_LAST_ACK:      return "LAST_ACK";
        case TCP_LISTEN:        return "LISTEN";
        case TCP_CLOSING:       return "CLOSING";
    }
    return "UNKNOWN";
}

const char *addr2str(int af, const void *addr, uint16_t port, char *buff)
{
    if (inet_ntop(af, addr, buff, MAX_ADDR_STR_SIZE) == NULL)
    {
        *buff = '\0';
        return NULL;
    }
    size_t buffLen = strlen(buff);
    if (port)
    {
        snprintf(buff+buffLen, MAX_ADDR_STR_SIZE-buffLen, ":%d", port);
    }
    else
    {
        strncat(buff+buffLen, ":*", MAX_ADDR_STR_SIZE-buffLen-1);
    }
    return buff;
}

Connect *conn_simple_alloc()
{
    Connect *conn = (Connect *)calloc(1, sizeof(Connect));
    return conn;
}

void ConnectFree(Connect *conn)
{
    free(conn);
}

static void ConnectListClear(ConnectListHead *head)
{
    if(!head->conns)
        return;
    while(head->used > 0)
    {
        ConnectFree(head->conns[head->used-1]);
        head->used--;
    }
    free(head->conns);
    head->conns = NULL;
    head->used = head->capacity = 0;
}

static int ConnectListInsert(ConnectListHead *head, Connect *conn)
{
    if(head->used == head->capacity)
    {
        size_t new_capacity = head->capacity*5/4 + 10;
        Connect **ptr = (Connect **)realloc(head->conns, new_capacity*sizeof(Connect *));
        if((!ptr))
        {
            fprintf(stderr, "%s: realloc failed new capacity:%lu, capacity:%lu used:%lu\n", 
                            __func__, new_capacity, head->capacity, head->used);
            return -1;
        }
        memset(ptr+head->capacity, 0, (new_capacity-head->capacity)*sizeof(Connect *));
        // printf("[%10d], [%p] - [%p]\n", new_capacity*sizeof(Connect *), ptr, ptr+new_capacity);
        head->conns = ptr;
        head->capacity = new_capacity;
    }
    // printf("caps:[%d], used:[%d] using:[%p]\n", head->capacity, head->used, &head->conns[head->used]);
    head->conns[head->used++] = (Connect *)conn;

    return 0;
}

static int RecordIPv4(const char *filename, ConnectListHead *head, const int type)
{
    time_t now = time(NULL);
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) return -1;

    char buf[MAX_LINE_SIZE];
    fgets(buf, MAX_LINE_SIZE, fp);
    while (fgets(buf, MAX_LINE_SIZE, fp))
    {
        Connect *conn = conn_simple_alloc();
        if(!conn)   break;
        conn->time = now;
        conn->af = AF_INET;
        conn->type = type;
        int n = sscanf(buf, " %d: %x:%x %x:%x %x %x:%x"
                            " %x:%x %x %d %d %u",
                       &conn->num, &conn->local.addr_data32[0], (unsigned int*)&conn->lport, &conn->foreign.addr_data32[0], 
                       (unsigned int*)&conn->fport, &conn->state, &conn->txq, &conn->rxq,
                       &conn->tr, &conn->when, &conn->retrnsmt, &conn->uid, &conn->timeout, &conn->ino);
        if (n == 14)
        {
            // get_serv_name_by_port(conn->lport, NULL, conn->name, sizeof(conn->name));
            ConnectListInsert(head, conn);
        }
    }
    fclose(fp);
    return 0;
}

__attribute__ ((unused)) static int RecordIPv6(const char *filename, ConnectListHead *head, const int type)
{
    time_t now = time(NULL);
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) return -1;

    char buf[MAX_LINE_SIZE];
    fgets(buf, MAX_LINE_SIZE, fp);
    while (fgets(buf, MAX_LINE_SIZE, fp))
    {
        Connect *conn = conn_simple_alloc();
        if(!conn)   break;
        conn->time = now;
        conn->af = AF_INET6;
        conn->type = type;
        int n = sscanf(buf, " %d: %8x%8x%8x%8x:%x %8x%8x%8x%8x:%x %x %x:%x"
                            " %x:%x %x %d %d %u",
                            &conn->num, &conn->local.addr_data32[0], &conn->local.addr_data32[1], &conn->local.addr_data32[2], &conn->local.addr_data32[3], (unsigned int*)&conn->lport,
                                &conn->foreign.addr_data32[0], &conn->foreign.addr_data32[1], &conn->foreign.addr_data32[2], &conn->foreign.addr_data32[3], (unsigned int*)&conn->fport,
                                &conn->state, &conn->txq, &conn->rxq,
                            &conn->tr, &conn->when, &conn->retrnsmt, &conn->uid, &conn->timeout, &conn->ino);
        if (n == 20)
        {
            // get_serv_name_by_port(conn->lport, NULL, conn->name, sizeof(conn->name));
            ConnectListInsert(head, conn);
        }
    }
    fclose(fp);
    return 0;
}

static int ConnectListInit(ConnectListHead *head)
{
    head->used = head->capacity = 0;
    head->conns = NULL;
    return 0;
}

RecordCtx *RecordCtxCreate()
{
    RecordCtx *ctx = (RecordCtx *)calloc(1, sizeof(RecordCtx));
    if(ctx)
    {
        ConnectListInit(&ctx->conns_head);
        return ctx;
    }
    return NULL;
}

void RecordCtxDestroy(RecordCtx *ctx)
{
    if(ctx)
    {
        ConnectListClear(&ctx->conns_head);
        free(ctx);
    }
}

Connect *SearchConnByQuintuple(RecordCtx *ctx, Address *addr1, Port port1, Address *addr2, Port port2, int proto, int af)
{
    Connect **conns = ctx->conns_head.conns;
    Connect *conn;
    int addr_len = (af==AF_INET) ? 4 : ((af==AF_INET6) ? 16 : 0);
    if(!addr_len)   return NULL;
    for(size_t i=0, total=ctx->conns_head.used; i<total; i++)
    {
        conn = conns[i];
        if(!conn)
            continue;
        if(conn->type != proto)
            continue;
        if(port1!=conn->lport && port1!=conn->fport)
            continue;
        if(port2!=conn->lport && port2!=conn->fport)
            continue;
        if(memcmp(addr1, &conn->local, addr_len) && memcmp(addr1, &conn->foreign, addr_len))
            continue;
        if(memcmp(addr2, &conn->foreign, addr_len) && memcmp(addr2, &conn->foreign, addr_len))
            continue;
        return conn;
    }
    return NULL;
}

Connect *SearchConnByPort(RecordCtx *ctx, uint16_t port)
{
    Connect **conns = ctx->conns_head.conns;
    Connect *conn;
    for(size_t i=0, total=ctx->conns_head.used; i<total; i++)
    {
        conn = conns[i];
        if(!conn)
        {
            continue;
        }
        if(port == conn->fport)
        {
            return conn;
        }
    }
    return NULL;
}

Connect *SearchConnByDest(RecordCtx *ctx, uint32_t dst_addr, uint16_t dst_port)
{
    Connect **conns = ctx->conns_head.conns;
    Connect *conn;
    for(size_t i=0, total=ctx->conns_head.used; i<total; i++)
    {
        conn = conns[i];
        if(!conn)
        {
            continue;
        }
        if(dst_port == conn->fport && dst_addr == conn->foreign.addr_data32[0])
        {
            return conn;
        }
    }
    return NULL;
}

static int ConnectCompareByLocalPort(const void *a, const void *b)
{
    Connect *first = *(Connect **)a;
    Connect *second= *(Connect **)b;

    return first->lport - second->lport;
}

__attribute__ ((unused)) static Connect *ConnectSearchByLocalPort(ConnectListHead *head, Connect *conn)
{
    Connect **conn_addr = (Connect **)bsearch(&conn, head->conns, head->used, sizeof(head->conns[0]), ConnectCompareByLocalPort);
    if(conn_addr)
    {
        return *conn_addr;
    }
    return NULL;
}

static void ConnectListSort(ConnectListHead *head)
{
    if(head && head->conns && head->used)
    {
        qsort(head->conns, head->used, sizeof(head->conns[0]), ConnectCompareByLocalPort);
    }
}

int ConnRecordLoad(RecordCtx *ctx)
{
    if(RecordIPv4(PATH_PROC_NET_TCP,  &ctx->conns_head, PROTO_TCP) < 0)
    {
        printf("%s: RecordIPv4 failed\n", __func__);
        return -1;
    }
    if(RecordIPv4(PATH_PROC_NET_UDP,  &ctx->conns_head, PROTO_UDP) < 0)
    {
        printf("%s: RecordIPv4 failed\n", __func__);
        return -1;
    }

    return 0;
}

Connect *conn_deep_copy(Connect *that)
{
    Connect *thiz = (Connect *)calloc(1, sizeof(Connect));
    if(thiz)
    {
        memcpy(thiz, that, sizeof(Connect));
    }

    return thiz;
}

void net_compare(ConnectListHead *prev_list, ConnectListHead *curr_list)
{
    if(!prev_list || !curr_list)
        return ;
    if(!prev_list->conns || !curr_list->conns)
        return ;

    size_t pused = prev_list->used;
    size_t cused = curr_list->used;

    Connect *conn;
    int do_sort = 0;
    for(size_t p=0, c=0; p<prev_list->used || c<curr_list->used; )
    {
        if(p==pused && c!=cused)
        {
            do_sort = 1;
            curr_list->conns[c]->compare_state = STATE_NEW;
            if(NULL != (conn=conn_deep_copy(curr_list->conns[c])))
            {
                do_sort = 1;
                ConnectListInsert(prev_list, conn);
            }
            c++;
            continue;
        }
        else if(p!=pused && c==cused)
        {
            prev_list->conns[p]->compare_state = STATE_DESTROY;
            p++;
            continue;
        }
        else if(p==pused && c==cused)
        {
            break;
        }

        if(prev_list->conns[p]->lport > curr_list->conns[c]->lport)
        {
            do_sort = 1;
            curr_list->conns[c]->compare_state = STATE_NEW;
            if(NULL != (conn=conn_deep_copy(curr_list->conns[c])))
            {
                do_sort = 1;
                ConnectListInsert(prev_list, conn);
            }
            c++;
        }
        else if(prev_list->conns[p]->lport < curr_list->conns[c]->lport)
        {
            prev_list->conns[p]->compare_state = STATE_DESTROY;
            p++;
        }
        else
        {
            if(prev_list->conns[p]->fport != curr_list->conns[c]->fport){
                prev_list->conns[p]->compare_state = STATE_DESTROY;
                curr_list->conns[c]->compare_state = STATE_NEW;
                if(NULL != (conn=conn_deep_copy(curr_list->conns[c])))
                {
                    do_sort = 1;
                    ConnectListInsert(prev_list, conn);
                }
                p++; c++;
            }
            else
            {
                prev_list->conns[p]->compare_state = STATE_KEEP;
                curr_list->conns[c]->compare_state = STATE_KEEP;
                p++; c++;
            }
        }
    }
    if(do_sort)
    {
        ConnectListSort(prev_list);
    }
    return ;
}

void conn_action_reset(ConnectListHead *conn_list)
{
    if(!conn_list || !conn_list->conns) return;

    for(size_t i=0; i<conn_list->used; i++)
        conn_list->conns[i]->compare_state = STATE_EMPTY;
}

void conn_debug(RecordCtx *ctx)
{
    char ip_src[32], ip_dst[32];
    for(size_t i=ctx->conns_head.used-1; i>=0; i--)
    {
        ip_src[0] = 0; ip_dst[0] = 0;
        Connect *conn = ctx->conns_head.conns[i];
        inet_ntop(AF_INET, &conn->local.addr_data32[0], ip_src, sizeof(ip_src));
        inet_ntop(AF_INET, &conn->foreign.addr_data32[0], ip_dst, sizeof(ip_dst));
        printf("[%20s]%s:%d --ino:%d--> %s:%d\n", 
            conn->edr_pid, ip_src, conn->lport, conn->ino, ip_dst, conn->fport);
    }
}
