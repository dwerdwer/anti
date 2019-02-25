#ifndef __CPU_INFO_H__
#define __CPU_INFO_H__
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct cpu_info_ {
    int         cpu_cores;
    uint64_t    cache_size_kb;
    // TODO: any other infos.. 
} cpu_info;

#define CPU_INFO_PATH   "/proc/cpuinfo"

// a const global struct cpu_info_
const struct cpu_info_ *get_cpu_info();
int get_cpu_cores();

#ifdef __cplusplus
}
#endif

#endif // __CPU_INFO_H__