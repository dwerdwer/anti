#ifndef __NETSTAT_H__
#define __NETSTAT_H__
#include <time.h>
#include <stdint.h>
#include <pthread.h>
#include <vector>
#include <string>
#include "defs.h"

#ifdef __cplusplus
extern "C" {
#endif

// this enum is already declare in <netinet/tcp.h>
#ifndef TCP_ESTABLISHED
enum {
    TCP_UNKNOWN = 0,
    TCP_ESTABLISHED,
    TCP_SYN_SENT,
    TCP_SYN_RECV,
    TCP_FIN_WAIT1,
    TCP_FIN_WAIT2,
    TCP_TIME_WAIT,
    TCP_CLOSE,
    TCP_CLOSE_WAIT,
    TCP_LAST_ACK,
    TCP_LISTEN,
    TCP_CLOSING,
};
#endif


enum {
    PROTO_TCP = 0,
    PROTO_UDP,
    PROTO_TCP6,
    PROTO_UDP6,
};

/* Address */
typedef union Address_ {
    uint32_t  addr_data32[4];
    uint16_t  addr_data16[8];
    uint8_t   addr_data8[16];
} Address;

/* Port is just a uint16_t */
typedef uint16_t Port;

/* store one connect info */
typedef struct Connect_ {
    int
        af,         // AF_INET, AF_INET6
        type,       // enum proto PROTO_*
        state;      // enum state TCP_*

    int compare_state; // for old compare with new

    uint32_t
        txq,
        rxq,
        num,
        tr,
        when,
        retrnsmt,
        uid,
        timeout,
        ino;

    Address 
        local,
        foreign;

    Port 
        lport,
        fport;

    time_t time;

    char name[MAX_LEN_ABSOLUTE_PATH_X_FILE_NAME+1];
    char edr_pid[64];
    char edr_ppid[64];
} Connect;

typedef struct ConnectListHead_{
    size_t    capacity;   // capacity of conns
    size_t    used;       // num of conns, and capacity will increment 1/4 automically when used==capacity
    Connect **conns;
} ConnectListHead;

typedef struct RecordCtx_ {
    ConnectListHead conns_head;
} RecordCtx;

RecordCtx *RecordCtxCreate();
void RecordCtxDestroy(RecordCtx *ctx);

int ConnRecordLoad(RecordCtx *ctx);

Connect *SearchConnByQuintuple(RecordCtx *ctx, Address *addr1, Port port1, Address *addr2, Port port2, int proto, int af);
Connect *SearchConnByDest(RecordCtx *ctx, uint32_t dst_addr, uint16_t dst_port);
Connect *SearchConnByPort(RecordCtx *ctx, uint16_t port);

const char *proto2str(int type);
const char *state2str(int state);
const char *addr2str(int af, const void *addr, uint16_t port, char *buff);

#ifndef STATE_EMPTY
#define STATE_EMPTY     0
#define STATE_NEW       1
#define STATE_DESTROY   2
#define STATE_KEEP      3
#endif

Connect *conn_simple_alloc();
Connect *conn_deep_copy(Connect *that);
void ConnectFree(Connect *conn);

void net_compare(ConnectListHead *prev_list, ConnectListHead *curr_list);
void conn_action_reset(ConnectListHead *conn_list);
void conn_debug(RecordCtx *ctx);

#ifdef __cplusplus
}
#endif

#endif // __NETSTAT_H__
