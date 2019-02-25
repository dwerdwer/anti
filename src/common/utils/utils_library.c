
#include "utils_library.h"

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

#include <stdio.h>
#include <dlfcn.h>

void *open_library(const char *p_lib_path, ...)
{
    void *p_result = NULL;
    if (NULL != p_lib_path)
    {
        va_list args;
        va_start(args, p_lib_path); 
        int32_t flags = va_arg(args, int32_t);
        p_result = dlopen(p_lib_path, flags);
        printf("open: %s    info: %s\n", p_lib_path, dlerror());
        va_end(args);
    }
    return p_result;
}

void *get_symbol_address(void *p_handle, const char *p_symbol_name)
{
    void *p_result = NULL;
    if (NULL != p_handle && NULL != p_symbol_name)
    {
        p_result = dlsym(p_handle, p_symbol_name);
    }
    return p_result;
}

void close_library(void *p_handle)
{
    if (NULL != p_handle)
    {
        dlclose(p_handle);
    }
}

