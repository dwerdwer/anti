
#ifndef DAEMON_MODULE_CONFIG_HHH
#define DAEMON_MODULE_CONFIG_HHH

#include <string>
#include <cstdint>

// 'dm' is short for "daemon module".
struct dm_setting
{
    std::string module_name;
    std::string path_name;
    uint32_t argument_count;
    const char **p_arguments;
};

bool load_dm_setting(const std::string &config_file, dm_setting &setting);

void destroy_dm_setting(dm_setting &setting);

#endif

