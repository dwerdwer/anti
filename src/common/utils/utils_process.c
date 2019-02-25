
#include "utils_process.h"
#include "utils_signal.h"
#include "utils_fd.h"

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>


/////////////////////////////////////////////////////////////////
// Private Implementations
/////////////////////////////////////////////////////////////////
struct flock singleton_lock =
{
    .l_type = F_WRLCK,
    .l_whence = SEEK_SET,
    .l_start = 0,
    .l_len = 0,
};

static int32_t become_child(void)
{
    int32_t pid = fork();
    if (0 < pid)
    {
        exit(0);    
    }
    return pid;
}

/////////////////////////////////////////////////////////////////
// Public Interfaces
/////////////////////////////////////////////////////////////////
int32_t daemonize(void)
{
    int32_t result = -1;
    if (0 == become_child() &&
            0 < setsid() &&
            0 == ignore_signal(SIGHUP, NO) &&
            0 == become_child() )
    {
        result = 0;
    }
    return result;
}

int32_t change_working_dir(const void *p_working_dir)
{
    return chdir((const char*)p_working_dir);
}

int32_t suppress_std(void)
{
    int32_t result = -1;
    int32_t temp = close_fd_completely(0);
    temp |= close_fd_completely(1);
    temp |= close_fd_completely(2);
    if (0 == temp &&
            0 == open("/dev/null", O_RDONLY) &&
            1 == open("/dev/null", O_WRONLY) &&
            2 == open("/dev/null", O_WRONLY) )
    {
        result = 0;
    }
    return result;
}

int32_t become_singleton(const char *p_name)
{
    int32_t result = -1;
    int32_t fd = -1;
    if (0 <= (fd = open(p_name, O_CREAT | O_WRONLY, 0600)) &&
            -1 != fcntl(fd, F_SETLK, &singleton_lock) )
    {
        result = 0;
        char buf[20];
        int32_t size = snprintf(buf, sizeof(buf), "%d", getpid());
        // Note: since the 'size' is very small and 'fd' is a regular file without O_NONBLOCK,
        // the write() is an atomical operation.
        result |= (0 < write(fd, buf, size)) ? 0 : -1;
        // Note: Since this is a file, not representing a kernel object, it can not be unlinked now.
        //result |= unlink(p_name);
    }
    return result;
}

