#ifndef MODULE_DATA_HHH
#define MODULE_DATA_HHH

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct module_data module_data_t;

module_data_t *create_module_data(void);
void destroy_module_data(module_data_t *p_data);

module_data_t *copy_module_data(const module_data_t *p_data);

// Return 0 if operation is successful.
// If property exists, replace its value.
int32_t set_module_data_property(module_data_t *p_data, 
        const char *p_name, const char *p_value, uint32_t value_length);

int32_t get_module_data_property(const module_data_t *p_data,
        const char *p_name, const char **pp_value, uint32_t *p_value_length);

#ifdef __cplusplus
}
#endif

#endif

