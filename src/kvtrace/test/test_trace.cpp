#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdint.h>
#include "KVMessage.h"
#include "IKVMessage.h"

int main(int argc, char* argv[])
{
    void* handle;
    typedef  void (*pfn)(const char*, const char*, uint32_t, int);
    char *error;
    handle = dlopen("libkvtrace.so", RTLD_LAZY);

    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        exit(1);
    }
    pfn p_trace = NULL;
    p_trace= (pfn)dlsym(handle, "trace_log");
    if ((error = dlerror()) != NULL)
    {
        fprintf(stderr, "%s\n", error);
        exit(1);
    }

    IRequestMessage* msg = NewInstance_IRequestMessage(CMD_LOGIN, SUBCMD_LOGIN);
    msg->Add_StringValue(23, "3ei43948");
    msg->Add_IntValue(2, 5);
    std::string data = msg->Encode();
    p_trace("11111", data.c_str(), data.length(), 1);

    dlclose(handle);
    return 0;
}
