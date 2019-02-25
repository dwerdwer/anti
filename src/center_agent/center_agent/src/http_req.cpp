#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "http_req.h"

struct MemoryStruct{
    char *memory;
    size_t size;
};

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct*)userp;

    mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL){
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

int do_request(const char* host,const char* p_data, const uint32_t data_size,
        char** p_retdata, uint32_t* ret_size)
{
    CURL *curl;
//    CURLcode res;
    int res;
    struct MemoryStruct chunk;
    chunk.memory = (char*)malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl) {

        curl_easy_setopt(curl, CURLOPT_URL, host);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);

        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, p_data);

        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data_size);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK){
            fprintf(stderr, "curl_easy_perform() failed: %s, host is %s\n",
                    curl_easy_strerror((CURLcode)res), host);
        }
        else {
//            printf("%s\n", chunk.memory);
            long http_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
//            printf("http return code is %d", http_code);
            if (http_code < 400)
                res = 0;
            *p_retdata = chunk.memory;
            *ret_size = chunk.size;
        }

        curl_easy_cleanup(curl);
        curl_global_cleanup();

        return res;
    }
    return -1;
}

void release_buf(char* p_buf)
{
    if (p_buf)
        free(p_buf);
}
