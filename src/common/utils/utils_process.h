#ifndef UTILS_PROCESS_HHH
#define UTILS_PROCESS_HHH

#include <stdint.h>

int32_t daemonize(void);

int32_t change_working_dir(const void *p_working_dir);

int32_t suppress_std(void);

int32_t become_singleton(const char *p_name);

#endif

