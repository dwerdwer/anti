#ifndef __SIMPLE_RING_BUFFER_H__
#define __SIMPLE_RING_BUFFER_H__
// #include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct single_read_single_write_ring_buffer_ {
    volatile int read_idx;
    volatile int write_idx;

    // pthread_spinlock_t lock;

    void **buffer;
    int size;
    int mask;
} ring_buffer_t;

ring_buffer_t *ring_buffer_create(int size);
ring_buffer_t *ring_buffer_create_default();

void *ring_buffer_pop(ring_buffer_t *rb);
int ring_buffer_push(ring_buffer_t *rb, void *ptr);

// clear all buff[i]
typedef void (*free_callback)(void *free_ptr);
void ring_buffer_clear(ring_buffer_t *rb, free_callback free_cb);

void ring_buffer_destroy(ring_buffer_t *rb);

void ring_buffer_clear_destroy(ring_buffer_t *rb, free_callback free_cb);

#ifdef __cplusplus
}
#endif


#endif // __SIMPLE_RING_BUFFER_H__