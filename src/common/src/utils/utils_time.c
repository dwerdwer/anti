
#include "utils_time.h"

#include <time.h>
#include <errno.h>

int32_t sleep_thread(int32_t seconds, int32_t nanoseconds)
{
    struct timespec time =
    {
        .tv_sec = seconds,
        .tv_nsec = nanoseconds,
    };

    int32_t result = -1;
    while(0 != result)
    {
        // Note: From POSIX manual, "The rqtp and rmtp arguments can point to the same object."
        result = nanosleep(&time, &time);
        if (0 != result && EINTR != errno)
        {
            break;
        }
    }

    return result;
}


