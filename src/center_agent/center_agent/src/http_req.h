#ifndef _HTTP_REQ_H__
#define _HTTP_REQ_H__
#include <curl/curl.h>

int do_request(const char* host, const char* p_data, const uint32_t data_size, char** p_retdata, uint32_t* ret_size);

void release_buf(char* p_buf);
#endif
