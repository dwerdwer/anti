#ifndef __DNS_PARSER_H__
#define __DNS_PARSER_H__

#include <stdint.h>
#include "util-queue.h"

#define MAX_DNS_REQUESTS  16
#define MAX_DNS_RESPONSES 64
#define MAX_DOMAIN_NAME_LEN 256

/* #pragma pack(1) */
typedef struct DNSPacketHeader_{
    uint16_t tr_id;
    uint16_t flags;
    uint16_t num_queries;
    uint16_t num_answers;
    uint16_t authority_rrs;
    uint16_t additional_rrs;
} DNSPacketHeader;
/* #pragma push() */

typedef struct DNSQuery_{
    char name[MAX_DOMAIN_NAME_LEN];
    uint16_t nameLen;
    uint16_t type;
    uint16_t class_alias;
    TAILQ_ENTRY(DNSQuery_) next;
} DNSQuery;

#define DNS_CLASS_IN 1
#define HOST_ADDRESS 1
#define C_NAME 5

typedef struct DNSResponse_{
    DNSQuery *queryer;
    uint16_t type;    // HOST_ADDRESS, C_NAME
    uint16_t class_alias;
    uint32_t ttl;
    uint16_t dataLen;
    union {                // decided by type
        uint32_t addr;
        char name[MAX_DOMAIN_NAME_LEN];
    };
    TAILQ_ENTRY(DNSResponse_) next;
} DNSResponse;

typedef struct DNSContext_{
    DNSPacketHeader head;

    TAILQ_HEAD(, DNSQuery_) querys;
    uint16_t query_num;

    TAILQ_HEAD(, DNSResponse_) responses;
    uint16_t response_addr_num;
    uint16_t response_domain_num;
} DNSContext;

DNSContext *DNSContextAlloc();
void DNSContextClear(DNSContext *ctx);
void DNSContextFree(DNSContext *ctx);

void debug_dns_ctx(DNSContext *ctx);
DNSQuery *DNSContextFindQuery(const DNSContext *ctx, const uint32_t ip_addr);



// ret     alloc error: -1
//        packet format parse error: 1
//        ok: 0
int ProDNSParse(uint8_t *dns_packet,uint16_t len, DNSContext *ctx);
int ProIPParse(uint8_t *ip_packet, DNSContext *ctx);



#endif //__DNS_PARSER_H__
