#include "utils_fd.h"

#include <unistd.h>
#include <errno.h>

int32_t close_fd_completely(int32_t fd)
{
    int32_t result = -1;
    while(1)
    {
        result = close(fd);
        if (0 == result || EBADF == errno)
        {
            result = 0;
            break;
        }
    }
    return result;
}

