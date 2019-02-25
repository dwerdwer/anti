
#ifndef MODULE_CONFIG_HHH
#define MODULE_CONFIG_HHH

#include "module_defines.h"
#include "message_rule_defines.h"

#include <string>
#include <vector>

struct module_setting
{
    std::string module_name;
    std::string path_name;
    bool need_load;
    bool need_isolation;
    module_category_t category;
    uint32_t argument_count;
    const char **p_arguments;
	
	module_setting():need_load(false),need_isolation(false),p_arguments(NULL), category(CATEGORY_OTHERS),argument_count(0)
	{}
};
std::vector<module_setting> load_module_settings(const std::string &config_file);
void destroy_module_settings(std::vector<module_setting> &module_settings);

message_rules_hash_table load_message_rules(const std::string &config_file);


#endif

