
#ifndef UTILS_BITS_HHH
#define UTILS_BITS_HHH

#include <stdint.h>

// 'lssb' is short for 'least significant set bit'.
int32_t find_lssb_u32(uint32_t number);

// 'mssb' is short for 'most significant set bit'.
int32_t find_mssb_u32(uint32_t number);

#endif

