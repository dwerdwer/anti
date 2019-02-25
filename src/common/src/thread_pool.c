
#include "thread_pool.h"

#include <stdlib.h>
#include <stdbool.h>

#include <pthread.h>

//////////////////////////////////////////////////////////////
// Private Implementations
//////////////////////////////////////////////////////////////

typedef struct thread_context
{
    pthread_t thread;
    void *p_thread_params;
    bool is_running;
}thread_context_t;

typedef struct thread_pool
{
    thread_data_scheduler_t schedule;
    void *p_schedule_params;
    thread_runner_t run;
    void *p_thread_attr;
    thread_data_sender_t send;
    thread_context_t *p_thread_contexts;
    uint8_t thread_count;
}thread_pool_t;

static bool check_params(const thread_pool_params_t *p_params)
{
    bool result = false;
    if (NULL != p_params &&
            NULL != p_params->schedule &&
            NULL != p_params->p_schedule_params &&
            NULL != p_params->run &&
            NULL != p_params->send &&
            NULL != p_params->pp_thread_params_array &&
            0 < p_params->thread_count)
    {
        result = true;
    }
    return result;
}

static void initialize_thread_contexts(thread_context_t *p_contexts, 
        void **pp_params_array, uint8_t count)
{
    uint8_t i = 0;
    for (; i < count; ++i)
    {
        p_contexts[i].p_thread_params = pp_params_array[i];
        p_contexts[i].is_running = false;
    }
}

static thread_pool_t *create_thread_pool_internal(const thread_pool_params_t *p_params)
{
    thread_pool_t *p_result = NULL;

    thread_pool_t *p_thread_pool_temp = (thread_pool_t*)malloc(sizeof(thread_pool_t));
    thread_context_t *p_thread_contexts_temp = 
        (thread_context_t*)calloc(sizeof(thread_context_t), p_params->thread_count);

    if (NULL != p_thread_pool_temp && NULL != p_thread_contexts_temp)
    {
        p_thread_pool_temp->schedule = p_params->schedule;
        p_thread_pool_temp->p_schedule_params = p_params->p_schedule_params;
        p_thread_pool_temp->run = p_params->run;
        p_thread_pool_temp->p_thread_attr = p_params->p_thread_attr;
        p_thread_pool_temp->send = p_params->send;
        initialize_thread_contexts(p_thread_contexts_temp, 
                p_params->pp_thread_params_array, p_params->thread_count);
        p_thread_pool_temp->p_thread_contexts = p_thread_contexts_temp;
        p_thread_pool_temp->thread_count = p_params->thread_count;

        p_result = p_thread_pool_temp;
    }
    else
    {
        free(p_thread_contexts_temp);
        free(p_thread_pool_temp);
    }

    return p_result;
}

static void stop_all(thread_pool_t *p_thread_pool)
{
    thread_context_t *p_contexts = p_thread_pool->p_thread_contexts;
    uint8_t i = 0;
    for (; i < p_thread_pool->thread_count; ++i)
    {
        if (true == p_contexts[i].is_running)
        {
            pthread_cancel(p_contexts[i].thread);
            p_contexts[i].is_running = false;
        }
    }
}

static int32_t send_data_to_new_thread(thread_pool_t *p_thread_pool, 
        thread_context_t *p_context, void *p_data, uint32_t data_length)
{
    int32_t result = -1;
    if (0 == pthread_create(&(p_context->thread), 
                (pthread_attr_t*)(p_thread_pool->p_thread_attr), 
                p_thread_pool->run, p_context->p_thread_params) &&
            0 == p_thread_pool->send(p_context->p_thread_params, p_data, data_length) )
    {
        p_context->is_running = true;
        result = 0;
    }
    return result;
}

static int32_t send_data_to_thread(thread_pool_t *p_thread_pool, 
        uint8_t thread_index, void *p_data, uint32_t data_length)
{
    int32_t result = -1;

    thread_context_t *p_context = p_thread_pool->p_thread_contexts + thread_index;
    if (true == p_context->is_running)
    {
        result = p_thread_pool->send(p_context->p_thread_params, p_data, data_length);
    }
    else
    {
        result = send_data_to_new_thread(p_thread_pool, p_context, p_data, data_length);
    }

    return result;
}

//////////////////////////////////////////////////////////////
// Public Interfaces
//////////////////////////////////////////////////////////////
// TODO: add new function object to stop thread.
// Call it in destroy_thread_pool to avoid to exit each thread forcefully.
thread_pool_t *create_thread_pool(const thread_pool_params_t *p_params)
{
    thread_pool_t *p_result = NULL;
    if (true == check_params(p_params))
    {
        p_result = create_thread_pool_internal(p_params);
    }
    return p_result;
}

void destroy_thread_pool(thread_pool_t *p_thread_pool)
{
    if (NULL != p_thread_pool)
    {
        stop_all(p_thread_pool);
        free(p_thread_pool->p_thread_contexts);
        free(p_thread_pool);
    }
}

int32_t send_data_to_thread_pool(thread_pool_t *p_thread_pool, 
        void *p_data, uint32_t data_length)
{
    int32_t result = -1;
    if (NULL != p_thread_pool)
    {
        uint8_t thread_index = p_thread_pool->schedule(p_thread_pool->p_schedule_params,
                p_data, data_length);
        if (thread_index < p_thread_pool->thread_count)
        {
            result = send_data_to_thread(p_thread_pool, thread_index, p_data, data_length);
        }
    }
    return result;
}

