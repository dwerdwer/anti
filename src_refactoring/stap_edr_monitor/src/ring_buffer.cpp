#include <stdlib.h>
#include "defs.h"
#include "ring_buffer.h"



#define DEFAULT_ARRAY_SIZE 1024

static int up_to_ceiling(int n)
{
    int res;
    for(res=2; res>0; res*=2)
        if(res >= n)
            break;
    return res;
}

static int index_format(int idx, int mask)
{
    return idx&mask;
}

void ring_buffer_destroy(ring_buffer_t *rb)
{
    if(rb)
    {
        // pthread_spin_destroy(&rb->lock);
        if(rb->buffer)
            free(rb->buffer);
        free(rb);
    }
}

ring_buffer_t *ring_buffer_create(int size)
{
    ring_buffer_t *rb = (ring_buffer_t *)calloc(1, sizeof(ring_buffer_t));
    if(!rb) return NULL;

    // pthread_spin_init(&rb->lock, PTHREAD_PROCESS_PRIVATE);

    rb->size = up_to_ceiling(size);
    rb->mask = rb->size - 1;
    rb->read_idx = rb->write_idx = 0;
    rb->buffer = (void **)calloc(rb->size, sizeof(void *));
    if(!rb->buffer)
    {
        ring_buffer_destroy(rb);
        return NULL;
    }
    return rb;
}

ring_buffer_t *ring_buffer_create_default()
{
    return ring_buffer_create(DEFAULT_ARRAY_SIZE);
}

void *ring_buffer_pop(ring_buffer_t *rb)
{
    if(!rb || !rb->buffer)  return NULL;

    void *res = NULL;

    // pthread_spin_lock(&rb->lock);
    if(rb->read_idx != rb->write_idx)
    {
        res = rb->buffer[rb->read_idx];
        rb->read_idx = index_format(rb->read_idx+1, rb->mask);
    }
    // pthread_spin_unlock(&rb->lock);

    return res;
}

int ring_buffer_push(ring_buffer_t *rb, void *ptr)
{
    if(!rb || !rb->buffer)  return -1;

    int ret = -1;

    // pthread_spin_lock(&rb->lock);
    if(index_format(rb->write_idx+1, rb->mask) != rb->read_idx)
    {
        rb->buffer[rb->write_idx] = ptr;
        rb->write_idx = index_format(rb->write_idx+1, rb->mask);
        ret = 0;
    }
    // pthread_spin_unlock(&rb->lock);

    return ret;
}

void ring_buffer_clear(ring_buffer_t *rb, free_callback free_cb)
{
    if(!rb || !rb->buffer || !free_cb)   return;

    while(rb->read_idx != rb->write_idx)
    {
        free_cb(ring_buffer_pop(rb));
    }
}

void ring_buffer_clear_destroy(ring_buffer_t *rb, free_callback free_cb)
{
    ring_buffer_clear(rb, free_cb);
    ring_buffer_destroy(rb);
}
