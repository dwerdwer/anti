
#include "daemon_module_defines.h"
#include "daemon_module_config.h"
#include "utils/utils_process.h"
#include "utils/utils_library.h"

#include <string>

#include <dlfcn.h>

const char *g_p_process_holder = "/tmp/vc_daemon.pid";
const std::string g_config_file = "../etc/vc_daemon_config.xml";

//const std::string g_library_path("../lib/");
//const std::string g_library_prefix("lib");
//const std::string g_library_suffix(".so");

//const std::string g_interface_create("create_");
//const std::string g_interface_destroy("destroy_");
//const std::string g_interface_run("run_");
//const std::string g_interface_stop("stop_");

const std::string g_interface_create("create_module");
const std::string g_interface_destroy("destroy_module");
const std::string g_interface_run("run");
const std::string g_interface_stop("stop");

struct module_info
{
    daemon_module_t *p_module;
    dm_creator_t create;
    dm_destroyer_t destroy;
    dm_runner_t run;
    dm_stopper_t stop;
    std::string name;
};

static int32_t become_daemon(void)
{
    int32_t result = 0;
    result |= daemonize();
    result |= suppress_std();

    return result;
}

static bool get_interfaces(module_info &info, void *p_handle)
{
    bool result = false;

    info.create = (dm_creator_t)get_symbol_address(p_handle, 
            g_interface_create.c_str());   
    info.destroy = (dm_destroyer_t)get_symbol_address(p_handle, 
            g_interface_destroy.c_str());
    info.run = (dm_runner_t)get_symbol_address(p_handle, 
            g_interface_run.c_str());
    info.stop = (dm_stopper_t)get_symbol_address(p_handle, 
            g_interface_stop.c_str());
    if (NULL != info.create &&
            NULL != info.destroy &&
            NULL != info.run &&
            NULL != info.stop)
    {
        result = true;
    }
    else
    {
        close_library(p_handle);
    }

    return result;

}

static bool load_module_interfaces(module_info &info, const std::string &module_path_name)
{
    bool result = false;
    
    void *p_handle = open_library(module_path_name.c_str(), RTLD_LAZY | RTLD_LOCAL);
    if (NULL != p_handle)
    {
        result = get_interfaces(info, p_handle);
    }
    return result;
}

static bool create_module(module_info &info, 
        uint32_t argument_count, const char **p_arguments)
{
    bool result = false;
    info.p_module = info.create(argument_count, p_arguments);
    if (NULL != info.p_module)
    {
        result = true;
    }
    return result;
}

static bool initialize_module_info(const dm_setting &setting, module_info &info)
{
    bool result = false;
    if (true == load_module_interfaces(info, setting.path_name))
    {
        info.name = setting.module_name;
        result = create_module(info, setting.argument_count, setting.p_arguments);
    }
    return result;
}

static dm_state_t launch_module(void)
{
    dm_state_t result = DAEMON_MODULE_OK;
    dm_setting setting;
    module_info info;
    if (true == load_dm_setting(g_config_file, setting) &&
            true == initialize_module_info(setting, info) )
    {
        destroy_dm_setting(setting);
        result = info.run(info.p_module);
        // No need to invoke 'stop', since this module runs in main thread.
        info.destroy(info.p_module);
    }
    return result;
}

// TODO: set umask to 0022, change working directory to root, and clean all fd or file-handle
int32_t main(void)
{
    int32_t result = 0;
    if (0 == become_daemon())
    {
        if (0 == become_singleton(g_p_process_holder)) 
        {
            result = launch_module();
        }
    }

    return result;
}

