
#include "shared_ptr.h"

#include <string.h>
#include <stdlib.h>

#include <pthread.h>

//////////////////////////////////////////////////////////////
// Private Implementations
//////////////////////////////////////////////////////////////
typedef struct shared_ptr
{
    pthread_spinlock_t lock;
    uint32_t reference_count;
    uint32_t object_size;
    void *p_raw_object;
    raw_object_destroyer_t destroy;
    char object[];
}shared_ptr_t;

static void decrease_ref_count(shared_ptr_t *p_shared_ptr)
{
    pthread_spin_lock(&(p_shared_ptr->lock));
    --(p_shared_ptr->reference_count);
    pthread_spin_unlock(&(p_shared_ptr->lock));
}

static void destroy_raw_object(shared_ptr_t *p_shared_ptr)
{
    if (NULL != p_shared_ptr->destroy)
    {
        p_shared_ptr->destroy(p_shared_ptr->p_raw_object);
    }
    else
    {
        free(p_shared_ptr->p_raw_object);
    }
}

static void try_destroy_shared_object(shared_ptr_t *p_shared_ptr)
{
    if (0 == p_shared_ptr->reference_count)
    {
        memcpy(p_shared_ptr->p_raw_object, p_shared_ptr->object, p_shared_ptr->object_size);
        destroy_raw_object(p_shared_ptr);
        free(p_shared_ptr);
    }
}

//////////////////////////////////////////////////////////////
// Public Interfaces
//////////////////////////////////////////////////////////////

void *create_shared_ptr(void *p_raw_ptr, uint32_t object_size, 
        raw_object_destroyer_t object_destroyer)
{
    void *p_result = NULL;

    if (NULL != p_raw_ptr && 0 < object_size && NULL != object_destroyer)
    {
        shared_ptr_t *p_temp = (shared_ptr_t*)malloc(sizeof(shared_ptr_t) + object_size);
        if (NULL != p_temp &&
                0 == pthread_spin_init(&(p_temp->lock), 0))
        {
            memcpy(p_temp->object, p_raw_ptr, object_size);
            p_temp->reference_count = 1u;
            p_temp->object_size = object_size;
            p_temp->p_raw_object = p_raw_ptr;
            p_temp->destroy = object_destroyer;
            p_result = p_temp->object;
        }
        else if (NULL != p_temp)
        {
            free(p_temp);
        }
        else
        {
            // Do nothing.
        }
    }

    return p_result;
}

void destroy_shared_ptr(void *p_shared_ptr)
{
    if (NULL != p_shared_ptr)
    {
        shared_ptr_t *p = (shared_ptr_t*)((char*)p_shared_ptr - sizeof(shared_ptr_t));
        decrease_ref_count(p);
        try_destroy_shared_object(p);
    }
}

void *copy_shared_ptr(void *p_shared_ptr)
{
    void *p_result = NULL;
    if (NULL != p_shared_ptr)
    {
        shared_ptr_t *p = (shared_ptr_t*)((char*)p_shared_ptr - sizeof(shared_ptr_t));
        pthread_spin_lock(&(p->lock));
        ++(p->reference_count);
        pthread_spin_unlock(&(p->lock));
        p_result = p->object;
    }
    return p_result;
}


