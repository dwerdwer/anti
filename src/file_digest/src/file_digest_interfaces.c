
#include "module_data.h"
#include "debug_print.h"
#include "file_digest_sqlite.h"
#include "utils/utils_library.h"
#include "file_digest_interfaces.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    uint32_t category;
    notify_scheduler_t notifier;
    void *p_params;
    void *p_digest;

    uint16_t message_count;
    char **message_types;

}module_digest_t;

LIB_PUBLIC module_t *create(uint32_t category, notify_scheduler_t notifier,
                            void *p_params, uint32_t arg_count, const char **p_args)
{
    if (1 != arg_count)
        return NULL;

    const char *p_db_path = p_args[0];

    module_digest_t *p_module = (module_digest_t*)malloc(sizeof(module_digest_t));

    if (NULL == p_module || NULL ==  p_db_path)
        return NULL;

    memset(p_module, 0, sizeof(module_digest_t));

    p_module->message_count = 2;

    p_module->message_types = (char**)malloc(p_module->message_count * sizeof(char*));
    
    uint16_t i = 0;
    for (i = 0; i < p_module->message_count; i++)
    {
        p_module->message_types[i] = (char*)malloc(64 * sizeof(char));

        memset(p_module->message_types[i], 0, 64 * sizeof(char));
    }
    strncpy(p_module->message_types[0], "FILE_DIGEST_DATA_SET", 64);
    strncpy(p_module->message_types[1], "FILE_DIGEST_DATA_GET", 64);

    p_module->category = category;
    p_module->notifier = notifier;
    p_module->p_params = p_params;

    if ((p_module->p_digest = create_file_digest_cache(p_db_path)) == NULL)
    {
        free(p_module);
        return NULL;
    }
    return (module_t*)p_module;
}

LIB_PUBLIC void get_inputted_message_type(module_t *p_module, const char *** const ppp_inputted_message_types,
                                          uint32_t *p_message_type_count)
{
    if (NULL == p_module)
        return;

    module_digest_t *p_module_digest = (module_digest_t*)p_module;

    *ppp_inputted_message_types = (const char**)p_module_digest->message_types;

    *p_message_type_count = p_module_digest->message_count;
}

LIB_PUBLIC void destroy(module_t *p_module)
{
    if (NULL != p_module)
    {
        module_digest_t *p_module_digest = (module_digest_t*)p_module;
        if (p_module_digest->message_types != NULL)
        {   
            uint16_t i = 0;
            for (i = 0; i <  p_module_digest->message_count; i++)
            {
                if (p_module_digest->message_types[i] != NULL)
                    free (p_module_digest->message_types[i]);

                p_module_digest->message_types[i] = NULL;
            }
            free(p_module_digest->message_types);
            p_module_digest->message_types = NULL;
        }       

        destroy_file_digest_cache(p_module_digest->p_digest);

        free(p_module);
        p_module = NULL;
    }
}

LIB_PUBLIC module_state_t run(module_t *p_module)
{
    if (NULL == p_module)
        return MODULE_ERROR;

    return MODULE_OK;	
}

LIB_PUBLIC module_state_t stop(module_t *p_module)
{
    if (NULL == p_module)
        return MODULE_ERROR;

    return MODULE_OK;	
}

LIB_PUBLIC module_data_t *assign(module_t *p_module, const module_data_t *p_data, bool is_sync)
{
    if (NULL == p_module || NULL == p_data) 
        return NULL;

    module_digest_t *p_module_digest = (module_digest_t*)p_module;

    // If set query exists
    uint32_t set_path_len = 0;
    const char* p_set_path = NULL;
    if(0 ==  get_module_data_property
       (p_data, p_module_digest->message_types[0], &p_set_path, &set_path_len))
    {
        if(cache_to_file_digest(p_module_digest->p_digest, p_set_path) != 0)
        {
            DEBUG_PRINT("cache_to_file_digest error\n");
            return NULL;
        }
    }
    // If get query exists
    uint32_t get_str_len = 0;
    const char* p_get_str = NULL;
    if(0 ==  get_module_data_property
       (p_data, p_module_digest->message_types[1], &p_get_str, &get_str_len))
    {
        file_digest_t *p_file_digest = NULL;

        if((p_file_digest = get_file_digest_by_md5(p_module_digest->p_digest, p_get_str)) == NULL)
        {
            DEBUG_PRINT("get_file_digest_by_md5 error\n");
            return NULL;
        }
        module_data_t *p_response_data = create_module_data();

        // TODO: business ......
        /*
           char tmp_cmd[] = "MSG_TYPE_XXX";
           set_module_data_property(p_data, g_p_message_id, tmp_cmd, strlen(tmp_cmd));
           set_module_data_property(p_response_data, p_module_digest->message_types[1], 
           (const char*)p_file_digest, sizeof(file_digest_t));

           module_message_t module_msg;
           module_msg.category = (module_category_t)pModule->uCategory;//static_cast<module_category_t>(p_module->category);
           module_msg.p_data = p_response_data;

           mdh_sync_params_t sync_data;
           sync_data.is_sync = true;
           p_module_digest->notifier(&module_msg, p_module_digest->p_params, &sync_data);
           */
        destroy_module_data(p_response_data);
    }
    return NULL;
}

