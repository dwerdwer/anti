#include "utils_bits.h"

// TODO: add code for non-Intel architectures.
int32_t find_lssb_u32(uint32_t number)
{
    int32_t result = -1;
    if (0 < number)
    {
        __asm__ __volatile__("bsfl %1, %%eax"
                :"=a"(result):"m"(number));
    }
    return result;
}

int32_t find_mssb_u32(uint32_t number)
{
    int32_t result = -1;
    if (0 < number)
    {
        __asm__ __volatile__("bsrl %1, %%eax"
                :"=a"(result):"m"(number));
    }
    return result;
}



