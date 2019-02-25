#include "dns.h"
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include <stdbool.h>

#define SHOW_FAILED_DNS_NAME

enum
{
    MAX_LABEL_LENGTH = 0x3F,
    MIN_POINTER_VALUE = 0xC0,
    MAX_LABELS_LENGTH = 0xFF,
};

typedef bool (*access_label_length_octet_t)(uint8_t *p_label_length_octet, void *p_params);

static bool parse_dns_labels(uint8_t *p_packet, uint16_t packet_length, 
        uint16_t start_offset_in_packet, 
        access_label_length_octet_t access, void *p_access_params)
{
    bool result = false;

    uint16_t offset = start_offset_in_packet;
    uint16_t total_labels_length = 0;
    uint16_t current_label_length = 0;
    bool is_continue = false;
    bool is_labels_error = false;

    // 'tll' is short for "total labels length"
    uint16_t expected_tll = 0;
    while(true)
    {
        current_label_length = p_packet[offset];

        is_continue = access(p_packet + offset, p_access_params);

        expected_tll = total_labels_length + 1 + current_label_length;

        if (/*(0 == current_label_length && start_offset_in_packet == offset) ||*/ 
                (MAX_LABEL_LENGTH < current_label_length && MIN_POINTER_VALUE > current_label_length) )
        {
            // There is error in label length octet
            is_labels_error = true;
            break;
        }
        else if ((MAX_LABEL_LENGTH >= current_label_length) && 
                (MAX_LABELS_LENGTH < expected_tll || packet_length < expected_tll) )
        {
            // The length of labels exceed the limitation of labels or the length of packet.
            is_labels_error = true;
            break;
        }
        else if (false == is_continue)
        {
            // User request stop
            result = true;
            break;
        }
        else if (0 == current_label_length)
        {
            // Label ends
            ++offset;   // Not necessary
            result = true;
            break;
        }
        else if (MIN_POINTER_VALUE <= current_label_length)
        {
            // Label uses pointer
            // In network byte-order, higher bytes in lower address.
            // The following code can only work in little-endian machine.
            // "offset = ntohs(*((uint16_t*)(p_packet + offset)) & 0xFF3F);"
            // use the following instead:
            offset = ((*(p_packet + offset) & 0x3F) << 8) + *(p_packet + offset + 1);
            //offset = ntohs(*((uint16_t*)(p_packet + offset))) & 0x3FFF;
        }
        else
        {
            ++offset;
            offset += current_label_length;

            total_labels_length = expected_tll;
            continue;
        }
    };
    
#ifdef SHOW_FAILED_DNS_NAME
    if (false == result)
    {
        char buffer[MAX_LABELS_LENGTH + 1] = {0};
        strncpy(buffer, (char*)p_packet + start_offset_in_packet, offset - start_offset_in_packet);
        printf("Labels error : %d, "
                "labels start with (may trancated) : %s, offset : %u, current label length: %u\n", 
                is_labels_error, buffer, offset, current_label_length);
    }
#endif

    return result;
}

static bool accumulate_label_length(uint8_t *p_label_length_octet, void *p_params)
{
    bool result = false;

    uint8_t *p_current_name_length = (uint8_t*)p_params;
    uint8_t octet_value = *p_label_length_octet;

    if (MIN_POINTER_VALUE <= octet_value)
    {
        *p_current_name_length += 2;
    }
    else if (MAX_LABEL_LENGTH >= octet_value && 
            MAX_LABELS_LENGTH >= (*p_current_name_length + 1 + octet_value))
    {
        *p_current_name_length += 1 + octet_value;
        result = true;
    }
    else
    {
        // Do nothing.
        // Error occurs if reach here.
        // Error should be handled by 'parse_dns_labels'.
    }

    return result;
}

static uint8_t calculate_name_length(uint8_t offset_in_packet, 
        const uint8_t *p_packet, uint16_t packet_length)
{
    uint8_t name_length = 0;
    parse_dns_labels((uint8_t*)p_packet, packet_length, offset_in_packet, 
            accumulate_label_length, &name_length);
    return name_length;
}
// [lxw] original code
//static uint8_t calculate_name_length(uint8_t offset, const uint8_t *payload, uint8_t payloadLen)
//{
//    if(payload[offset] == 0x00)
//        return(1);
//    else if(payload[offset] == 0xC0)
//        return(2);
//    else {
//        uint8_t len = payload[offset];
//        uint8_t off = len + 1;
//
//        if(off == 0) /* Bad packet */
//            return(0);
//        else
//            return(off + calculate_name_length(offset+off, payload, payloadLen));
//    }
//}


#if 0
static uint8_t getName(uint8_t offset, const uint8_t *payload, uint8_t payloadLen, char *name)
{
    uint8_t len = getNameLength(offset, payload, payloadLen);
    if(len > 2)
        memcpy(name, payload, len);
    return len;
}
#endif

#if 0
static uint8_t *getN(uint16_t *i, const uint8_t *payload, uint16_t len)
{
    uint8_t *ptr = payload + (*i);

    (*i) += len;

    return ptr;
}
#endif

static uint16_t get16(uint16_t *i, const uint8_t *payload) 
{
    uint16_t v = *(uint16_t*)&payload[*i];

    (*i) += sizeof(uint16_t);

    return(ntohs(v));
}

static uint32_t get32(uint16_t *i, const uint8_t *payload) 
{
    uint32_t v = *(uint32_t *)&payload[*i];

    (*i) += sizeof(uint32_t);

    return (ntohl(v));
}

static bool replace_label_length_octet(uint8_t *p_label_length_octet, void *p_params)
{
    char *p_replace_octet = (char*)p_params;
    if (MIN_POINTER_VALUE <= *p_label_length_octet)
    {
        *p_label_length_octet = 0;
    }
    else if (MAX_LABEL_LENGTH >= *p_label_length_octet && 0 < *p_label_length_octet)
    {
        *p_label_length_octet = *p_replace_octet;
    }
    else if (0 == *p_label_length_octet)
    {
        // Leave it be.
    }
    else
    {
        // Do nothing.
        // Error occurs if reach here.
        // Error should be handled by 'parse_dns_labels'.
    }
    return true;
}

static char *nameTransFormat(char *name, uint16_t nameLen)
{
    char replace_octet = '.';
    //char *p_ori = (char*)malloc(nameLen);
    //memcpy(p_ori, name, nameLen);
    parse_dns_labels((uint8_t*)name, nameLen, 0, 
            replace_label_length_octet, &replace_octet);
    //free(p_ori);
    memmove(name, name + 1, nameLen - 1);
    name[nameLen - 1] = 0;
    return name;

    // [lxw] original code
    //char *tmp;
    //name[nameLen] = 0;
    //for(tmp=name; tmp<name+nameLen; tmp++)
    //    if((*tmp) && !isalnum(*tmp))
    //        *tmp = '.';

    //for(tmp=name; tmp<name+nameLen; tmp++)
    //    *tmp = *(tmp+1);
    //return name;
}

static int DNSContextAddQuery(DNSContext *ctx, 
        const uint8_t *name, const uint16_t nameLen, 
        const uint16_t type, const uint16_t class_alias)
{
    DNSQuery *query = (DNSQuery *)calloc(1, sizeof(DNSQuery));
    if(!query) {
        return -1;
    }
    query->nameLen = (nameLen<MAX_DOMAIN_NAME_LEN ? nameLen : MAX_DOMAIN_NAME_LEN - 1);
    memcpy(query->name, name, query->nameLen);
    query->type = type;
    query->class_alias = class_alias;

    nameTransFormat(query->name, query->nameLen);
    query->nameLen = strlen(query->name);

    TAILQ_INSERT_TAIL(&ctx->querys, query, next);
    return 0;
}

static int DNSContextAddResponse(DNSContext *ctx, 
        const uint8_t *name, const uint16_t nameLen, 
        const uint16_t type, const uint16_t class_alias, const uint32_t ttl, 
        const uint8_t *data, const uint16_t dataLen)
{
    DNSResponse *resp = (DNSResponse *)calloc(1, sizeof(DNSResponse));
    if(!resp) {
        return -1;
    }
    if(TAILQ_FIRST(&ctx->querys))
        resp->queryer = TAILQ_FIRST(&ctx->querys);
    resp->type = type;
    resp->class_alias = class_alias;
    resp->ttl = ttl;

    if(type == HOST_ADDRESS) {
        if(dataLen == sizeof(uint32_t)) {
            resp->addr = *((uint32_t*)data);
            resp->dataLen = dataLen;
            ctx->response_addr_num++;
        }
    }else if(type == C_NAME) {
        if(dataLen > 4) {
            resp->dataLen = (dataLen<MAX_DOMAIN_NAME_LEN ? dataLen : MAX_DOMAIN_NAME_LEN - 1);
            memcpy(resp->name, data, resp->dataLen);
            nameTransFormat(resp->name, resp->dataLen);
            resp->dataLen = strlen(resp->name);
            ctx->response_domain_num++;
        }
    }

    TAILQ_INSERT_TAIL(&ctx->responses, resp, next);
    return 0;
}

static int ProDNSQueryParse(const DNSPacketHeader *dns_header, 
        const uint8_t *payload, const uint16_t payloadLen, 
        DNSContext *ctx)
{
    int ret_val = 0;
    if((dns_header->num_queries > 0) 
            && (dns_header->num_queries <= MAX_DNS_REQUESTS)
            // && (((dns_header->flags & 0x2800) == 0x2800 /* Dynamic DNS Update */)
            // || ((dns_header->num_answers == 0) && (dns_header->authority_rrs == 0))
            // )
      ) {

        /* This is a good query */
        int num_queries;
        for(num_queries=0; num_queries < dns_header->num_queries && ret_val == 0; num_queries++) {
            uint16_t offset = 0;

            const uint8_t *name = payload+offset;

            uint16_t nameLen = calculate_name_length(offset + sizeof(DNSPacketHeader), 
                    payload - sizeof(DNSPacketHeader), payloadLen + sizeof(DNSPacketHeader));
            // [lxw] original code
            //uint16_t nameLen = calculate_name_length(offset, payload, payloadLen);

            if(nameLen == 0)  {
                ret_val = 1;
                break;
            }    
            offset += nameLen;

            /* type:2B, class:2B */
            uint16_t type = get16(&offset, payload);
            uint16_t class_alias= get16(&offset, payload);

            DNSContextAddQuery(ctx, name, nameLen, type, class_alias);
            ctx->query_num++;
        }
    } else {
        ret_val = 1;
    }

    return ret_val;
}

static int ProDNSResponseParse(const DNSPacketHeader *dns_header, 
        const uint8_t *payload, const uint16_t payloadLen, 
        DNSContext *ctx)
{
    int ret_val = 0;
    if((dns_header->num_queries > 0) 
        && (dns_header->num_queries <= MAX_DNS_REQUESTS) /* Don't assume that num_queries must be zero */
        && (((dns_header->num_answers > 0) && (dns_header->num_answers <= MAX_DNS_REQUESTS))
            || ((dns_header->authority_rrs > 0) && (dns_header->authority_rrs <= MAX_DNS_REQUESTS))
            || ((dns_header->additional_rrs > 0) && (dns_header->additional_rrs <= MAX_DNS_REQUESTS))
           )
      ) {
        /* This is a good reply */
        ProDNSQueryParse(dns_header, payload, payloadLen, ctx);

        /* skip query section */
        uint16_t num_queries = dns_header->num_queries;
        uint16_t offset = 0;
        while(0 < num_queries)
        {
            offset++;
            if(payload[offset] != '\0') {
                while((offset < payloadLen) && (payload[offset] != '\0')) {
                    offset++;
                }
                offset++;
            }
            offset += 4;

            --num_queries;
        }

        /* now offset is at the begin of reply */
        if(dns_header->num_answers > 0) {
            uint16_t num_answers;
            for(num_answers = 0; num_answers < dns_header->num_answers; num_answers++) {
                if((offset+16) > payloadLen) {
                    break;
                }

                uint16_t nameLen = calculate_name_length(offset + sizeof(DNSPacketHeader), 
                        payload - sizeof(DNSPacketHeader), payloadLen + sizeof(DNSPacketHeader));
                // [lxw] original code
                //uint16_t nameLen = calculate_name_length(offset, payload, payloadLen);
                if(nameLen == 0) {
                    ret_val = 1;
                    break;
                }
                const uint8_t *name = payload+offset;
                offset += nameLen;

                /* type:2B */
                uint16_t type = get16(&offset, payload);
                if(HOST_ADDRESS != type && C_NAME != type)
                    break;

                /* class:2B */
                uint16_t class_alias = get16(&offset, payload);
                if(DNS_CLASS_IN != class_alias)
                    break;
                /* ttl:4B */
                uint32_t ttl = get32(&offset, payload);

                /* dataLen */
                uint16_t dataLen = get16(&offset, payload);
                if(dataLen < 2 || dataLen > payloadLen-offset)
                    break;
                if(HOST_ADDRESS == type && 4 != dataLen)
                    break;
                if(C_NAME == type && dataLen < 2)
                    break;
                const uint8_t *data = payload+offset;
                offset += dataLen;

                DNSContextAddResponse(ctx, name, nameLen, type, class_alias, ttl, data, dataLen);
            }
        }
    }
    return ret_val;
}

int ProDNSParse(uint8_t *dns_packet,uint16_t len, DNSContext *ctx)
{
    if(len < sizeof(DNSPacketHeader))
        return 1;

    /* header */
    DNSPacketHeader dns_header;
    memcpy(&dns_header, dns_packet, sizeof(DNSPacketHeader));
    dns_header.tr_id = ntohs(dns_header.tr_id);
    dns_header.flags = ntohs(dns_header.flags);
    dns_header.num_queries = ntohs(dns_header.num_queries);
    dns_header.num_answers = ntohs(dns_header.num_answers);
    dns_header.authority_rrs = ntohs(dns_header.authority_rrs);
    dns_header.additional_rrs = ntohs(dns_header.additional_rrs);
    memcpy(&ctx->head, &dns_header, sizeof(DNSPacketHeader));

    int is_query=0, is_invalid=0;

#define FLAGS_MASK 0x8000
    if((dns_header.flags & FLAGS_MASK) == 0x0000)
        is_query = 1;
    else if((dns_header.flags & FLAGS_MASK) == 0x8000)
        is_query = 0;
    else
        is_invalid = 1;

    /* body */
    int ret_val = 0;
    if(!is_invalid) {
        uint8_t *payload = dns_packet + sizeof(DNSPacketHeader);
        uint16_t payloadLen = len - sizeof(DNSPacketHeader);
        if(is_query){
            /* query */
            ret_val = ProDNSQueryParse(&dns_header, payload, payloadLen, ctx);
        }else{
            /* response */
            ret_val = ProDNSResponseParse(&dns_header, payload, payloadLen, ctx);
        }    
    }
    return ret_val;
}

#define UDP_OFFSET      sizeof(struct udphdr)
int ProIPParse(uint8_t *ip_packet, DNSContext *ctx)
{
    struct ip *iph = (struct ip *)ip_packet;

    uint16_t ip_hlen = iph->ip_hl * 4;
    uint16_t ip_tlen = ntohs(iph->ip_len);
    if (ip_tlen < ip_hlen + UDP_OFFSET)
        return 1;
    if(iph->ip_p != IPPROTO_UDP)
        return 1;

    struct udphdr *udph = (struct udphdr *)((uint8_t *)iph + ip_hlen);
    uint16_t udp_tlen = ntohs(udph->len);
    if (ip_tlen != ip_hlen + udp_tlen)
        return 1;

    uint8_t *payload = (uint8_t *)((uint8_t *)iph + ip_hlen + UDP_OFFSET);
    int payloadLen = udp_tlen - UDP_OFFSET;
    if(payloadLen < 1)
        return 1;

    uint16_t sport = ntohs(udph->source);
    uint16_t dport = ntohs(udph->dest);
    if(sport == 53 || dport == 53)
        return ProDNSParse(payload, payloadLen, ctx);

    return 0;
}

DNSQuery *DNSContextFindQuery(const DNSContext *ctx, const uint32_t ip_addr)
{
    DNSResponse *resp;
    TAILQ_FOREACH(resp, &ctx->responses, next) {
        if(resp->type == HOST_ADDRESS && resp->addr == ip_addr) {
            return resp->queryer;
        }
    }
    return NULL;
}

DNSContext *DNSContextAlloc()
{
    DNSContext *ptr = (DNSContext *)malloc(sizeof(DNSContext));
    if(!ptr) {
        return NULL;
    }
    memset(ptr, 0, sizeof(DNSContext));

    TAILQ_INIT(&ptr->querys);
    TAILQ_INIT(&ptr->responses);

    return ptr;
}

void DNSContextClear(DNSContext *ctx)
{
    DNSQuery *query, *queryTemp;
    TAILQ_FOREACH_SAFE(query, &ctx->querys, next, queryTemp) {
        TAILQ_REMOVE(&ctx->querys, query, next);
        free(query);
    }

    DNSResponse *resp, *respTemp;
    TAILQ_FOREACH_SAFE(resp, &ctx->responses, next, respTemp) {
        TAILQ_REMOVE(&ctx->responses, resp, next);
        free(resp);
    }
}

void DNSContextFree(DNSContext *ctx)
{
    DNSContextClear(ctx);
    free(ctx);
}

void debug_dns_ctx(DNSContext *ctx)
{
    DNSQuery *query;
    TAILQ_FOREACH(query, &ctx->querys, next) {
        printf("[queryresponse] : %s\n", query->name);    
    }

    DNSResponse *resp;
    TAILQ_FOREACH(resp, &ctx->responses, next) {
        if(resp->type == HOST_ADDRESS) {
            printf("[response] addr:%u.%u.%u.%u\n", ((uint8_t *)(&resp->addr))[0], 
                    ((uint8_t *)(&resp->addr))[1], 
                    ((uint8_t *)(&resp->addr))[2], 
                    ((uint8_t *)(&resp->addr))[3]);

        } else {
            printf("[response] domain:%s\n", resp->name);
        }
    }
}

/* int main(int argc, char const *argv[]) */
/* { */
/*     uint8_t packet1[] = {0x24, 0x9e, 0xab, 0xa4, 0x60, 0x6b, 0x8c, 0xec, 0x4b, 0x40, 0x34, 0x01, 0x08, 0x00, 0x45, 0x00, 0x00, 0x3b, 0x16, 0x40, 0x00, 0x00, 0x80, 0x11, 0x00, 0x00, 0xc0, 0xa8, 0x0a, 0x68, 0xc0, 0xa8, 0x00, 0xc8, 0xd8, 0x00, 0x00, 0x35, 0x00, 0x27, 0x8c, 0xb9, 0x9a, 0x2c, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x77, 0x77, 0x77, 0x05, 0x69, 0x66, 0x65, 0x6e, 0x67, 0x03, 0x63, 0x6f, 0x6d, 0x00, 0x00, 0x01, 0x00, 0x01}; */
/*     uint8_t packet2[] = {0x8c, 0xec, 0x4b, 0x40, 0x34, 0x01, 0x24, 0x9e, 0xab, 0xa4, 0x60, 0x6b, 0x08, 0x00, 0x45, 0x00, 0x01, 0x4a, 0x1b, 0xb7, 0x40, 0x00, 0x7f, 0x11, 0x52, 0x6b, 0xc0, 0xa8, 0x00, 0xc8, 0xc0, 0xa8, 0x0a, 0x68, 0x00, 0x35, 0xd8, 0x00, 0x01, 0x36, 0x5b, 0xa5, 0x9a, 0x2c, 0x81, 0x80, 0x00, 0x01, 0x00, 0x04, 0x00, 0x05, 0x00, 0x02, 0x03, 0x77, 0x77, 0x77, 0x05, 0x69, 0x66, 0x65, 0x6e, 0x67, 0x03, 0x63, 0x6f, 0x6d, 0x00, 0x00, 0x01, 0x00, 0x01, 0xc0, 0x0c, 0x00, 0x05, 0x00, 0x01, 0x00, 0x00, 0x03, 0xe4, 0x00, 0x19, 0x03, 0x77, 0x77, 0x77, 0x05, 0x69, 0x66, 0x65, 0x6e, 0x67, 0x03, 0x63, 0x6f, 0x6d, 0x08, 0x69, 0x66, 0x65, 0x6e, 0x67, 0x63, 0x64, 0x6e, 0xc0, 0x16, 0xc0, 0x2b, 0x00, 0x05, 0x00, 0x01, 0x00, 0x00, 0x02, 0x58, 0x00, 0x16, 0x03, 0x77, 0x77, 0x77, 0x05, 0x69, 0x66, 0x65, 0x6e, 0x67, 0x03, 0x63, 0x6f, 0x6d, 0x05, 0x6c, 0x78, 0x64, 0x6e, 0x73, 0xc0, 0x16, 0xc0, 0x50, 0x00, 0x05, 0x00, 0x01, 0x00, 0x00, 0x02, 0x58, 0x00, 0x16, 0x01, 0x63, 0x09, 0x78, 0x64, 0x77, 0x73, 0x63, 0x61, 0x63, 0x68, 0x65, 0x07, 0x6f, 0x75, 0x72, 0x67, 0x6c, 0x62, 0x30, 0xc0, 0x16, 0xc0, 0x72, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x02, 0x58, 0x00, 0x04, 0x7c, 0xca, 0xa6, 0x39, 0xc0, 0x7e, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x64, 0x84, 0x00, 0x12, 0x04, 0x64, 0x6e, 0x73, 0x35, 0x07, 0x6f, 0x75, 0x72, 0x67, 0x6c, 0x62, 0x30, 0x03, 0x6f, 0x72, 0x67, 0x00, 0xc0, 0x7e, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x64, 0x84, 0x00, 0x07, 0x04, 0x64, 0x6e, 0x73, 0x33, 0xc0, 0xa9, 0xc0, 0x7e, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x64, 0x84, 0x00, 0x13, 0x04, 0x64, 0x6e, 0x73, 0x32, 0x07, 0x6f, 0x75, 0x72, 0x67, 0x6c, 0x62, 0x30, 0x04, 0x69, 0x6e, 0x66, 0x6f, 0x00, 0xc0, 0x7e, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x64, 0x84, 0x00, 0x07, 0x04, 0x64, 0x6e, 0x73, 0x34, 0xc0, 0xda, 0xc0, 0x7e, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x64, 0x84, 0x00, 0x07, 0x04, 0x64, 0x6e, 0x73, 0x31, 0xc0, 0xa9, 0xc0, 0xd5, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x10, 0xc5, 0x00, 0x04, 0xdd, 0xca, 0xcc, 0xe8, 0xc0, 0xc2, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x10, 0xc5, 0x00, 0x04, 0x65, 0xf6, 0xb6, 0x2f}; */

/*     DNSContext *ctx1 = DNSContextAlloc(); */
/*     ProIPParse(packet1+14, ctx1); */
/*     debug_dns_ctx(ctx1); */
/*     DNSContextFree(ctx1); */

/*     printf("response-----------------------------\n"); */

/*     DNSContext *ctx2 = DNSContextAlloc(); */
/*     ProIPParse(packet2+14, ctx2); */
/*     debug_dns_ctx(ctx2); */
/*     DNSContextFree(ctx2); */

/*     return 0; */
/* } */
