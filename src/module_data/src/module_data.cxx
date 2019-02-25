
#include "module_data.h"

#include <map>
#include <string>
#include <utility>

/////////////////////////////////////////////////////////////////
// Private Implementations
/////////////////////////////////////////////////////////////////
typedef std::map<std::string, std::string> properties; 


static void set_value(const std::string &value,
        const char **pp_value, uint32_t *p_value_length)
{
    if (NULL != pp_value && NULL != p_value_length)
    {
        *pp_value = value.data();
        *p_value_length = (uint32_t)(value.size());
    }
}

/////////////////////////////////////////////////////////////////
// Public Interfaces
/////////////////////////////////////////////////////////////////
module_data_t *create_module_data(void)
{
    properties *p_properties = NULL;
    try
    {
        p_properties = new properties;
    }
    catch(...)
    {
        // Do nothing
    }
    return reinterpret_cast<module_data_t*>(p_properties);
}

void destroy_module_data(module_data_t *p_data)
{
    properties *p_properties = reinterpret_cast<properties*>(p_data);
    delete p_properties;
}

module_data_t *copy_module_data(const module_data_t *p_data)
{
    module_data_t *p_result = NULL;
    properties *p_properties = reinterpret_cast<properties*>(const_cast<module_data*>(p_data));

    try
    {
        properties *p_temp = new properties(*p_properties);
        p_result = reinterpret_cast<module_data_t*>(p_temp);
    }
    catch(...)
    {
        // Do nothing.
    }

    return p_result;
}

int32_t set_module_data_property(module_data_t *p_data, 
        const char *p_name, const char *p_value, uint32_t value_length)
{
    int32_t result = -1;
    properties *p_properties = reinterpret_cast<properties*>(p_data);
    try
    {
        std::string name(p_name);
        std::string value(p_value, value_length);
        p_properties->operator[](std::move(name)) = std::move(value);
        result = 0;
    }
    catch(...)
    {
        // Do nothing.
    }
    return result;
}

int32_t get_module_data_property(const module_data_t *p_data,
        const char *p_name, const char **pp_value, uint32_t *p_value_length)
{
    int32_t result = -1;
    const properties *p_properties = reinterpret_cast<const properties*>(p_data);
    properties::const_iterator citer = p_properties->end();

    try
    {
        std::string name(p_name);
        citer = p_properties->find(name); 
    }
    catch(...)
    {
        // Do nothing
    }

    if (p_properties->end() != citer)
    {
        set_value(citer->second, pp_value, p_value_length);
        result = 0;
    }

    return result;
}


