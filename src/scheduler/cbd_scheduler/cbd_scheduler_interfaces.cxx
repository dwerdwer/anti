
#include "cbd_scheduler.h"
#include "daemon_module_defines.h"
#include "utils/utils_library.h"

extern "C" LIB_PUBLIC 
daemon_module_t *create_module(uint32_t arg_count, const char **p_args)
{
    void *p_result = NULL;
    if (1 <= arg_count && NULL != p_args[0])
    {
        try
        {
            p_result = new cbd_scheduler(p_args[0]);
        }
        catch(...)
        {
            p_result = NULL;
        }
    }
    return static_cast<daemon_module_t*>(p_result);
}

extern "C" LIB_PUBLIC 
void destroy_module(daemon_module_t *p_scheduler)
{
    if (NULL != p_scheduler)
    {
        cbd_scheduler *p = reinterpret_cast<cbd_scheduler*>(p_scheduler);
        delete p;
    }
}

extern "C" LIB_PUBLIC 
dm_state_t run(daemon_module_t *p_scheduler)
{
    dm_state_t result = DAEMON_MODULE_OK;
    if (NULL != p_scheduler)
    {
        cbd_scheduler *p = reinterpret_cast<cbd_scheduler*>(p_scheduler);
        try
        {
            p->run();
        }
        catch(...)
        {
            result = DAEMON_MODULE_ERROR;
        }
    }
    return result;
}

extern "C" LIB_PUBLIC 
dm_state_t stop(daemon_module_t *p_scheduler)
{
    dm_state_t result = DAEMON_MODULE_OK;
    if (NULL != p_scheduler)
    {
        cbd_scheduler *p = reinterpret_cast<cbd_scheduler*>(p_scheduler);
        try
        {
            p->stop();
        }
        catch(...)
        {
            result = DAEMON_MODULE_ERROR;
        }
    }
    return result;
}


