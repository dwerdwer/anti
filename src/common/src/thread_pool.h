#ifndef THREAD_POOL_HHH
#define THREAD_POOL_HHH

#include <stdint.h>

typedef struct thread_pool thread_pool_t;

typedef void *(*thread_runner_t)(void *p_thread_params);
// Return 0 if send successfully.
typedef int32_t (*thread_data_sender_t)(void *p_thread_params, 
        void *p_data, uint32_t data_length);
// Return the thread index/id in thread pool.
typedef uint8_t (*thread_data_scheduler_t)(void *p_schedule_params, 
        void *p_data, uint32_t data_length);

typedef struct thread_pool_params
{
    thread_data_scheduler_t schedule;
    void *p_schedule_params;
    thread_runner_t run;
    void *p_thread_attr;
    thread_data_sender_t send;
    void **pp_thread_params_array;
    uint8_t thread_count;
}thread_pool_params_t;

thread_pool_t *create_thread_pool(const thread_pool_params_t *p_params);

void destroy_thread_pool(thread_pool_t *p_thread_pool);

int32_t send_data_to_thread_pool(thread_pool_t *p_thread_pool, 
        void *p_data, uint32_t data_length);

#endif

