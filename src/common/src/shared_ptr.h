#ifndef SHARED_POINTER_HHH
#define SHARED_POINTER_HHH

#include <stdint.h>

typedef void (*raw_object_destroyer_t)(void *p_raw_ptr);

// Parameters:
// 'p_raw_ptr' should not be NULL.
// 'object_size' should be larger than 0.
// if 'object_destroyer' is NULL, use free instead.
void *create_shared_ptr(void *p_raw_ptr, uint32_t object_size, 
        raw_object_destroyer_t object_destroyer);

void destroy_shared_ptr(void *p_shared_ptr);

void *copy_shared_ptr(void *p_shared_ptr);

#endif

