#ifdef linux
#include <sys/time.h>
#include <pthread.h>
#endif

#define MAX_LOG_LEN 1024

// p_log_path is absolute path
void    init_log(const char * p_log_path);

void    dbg_log(const char *fmt,...);

