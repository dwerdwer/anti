
#include "utils_integer.h"

uint32_t round_up_u32(uint32_t input_value, uint32_t modular)
{
    uint64_t rounded_value = ((uint64_t)input_value + modular - 1) / modular * modular;
    return (uint32_t)((rounded_value > UINT32_MAX) ? UINT32_MAX : rounded_value);
}

uint16_t round_up_u16(uint16_t input_value, uint16_t modular)
{
    uint32_t rounded_value = ((uint32_t)input_value + modular - 1) / modular * modular;
    return (uint16_t)((rounded_value > UINT16_MAX) ? UINT16_MAX : rounded_value);
}


