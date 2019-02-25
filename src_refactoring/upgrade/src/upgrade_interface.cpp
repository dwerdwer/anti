#include <string>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/stat.h>
#include <algorithm>

#include "version_manager.h"
#include "upgrade_interface.h"
#include "upgrade_logger.h"
#include "rpcsrv_interface.h"
#include "kv_engine_public.h"

#define UPGRADE_CMD "bash upgrade.sh"

#define CONF_UPGRADE_DAEMON_PACKAGE_KEY         "daemon_package"
#define CONF_UPGRADE_VIRUSLIB_PACKAGE_KEY       "viruslib_package"
#define CONF_DOWNLOAD_VERSION_FILE_SCRIPT_KEY   "download_version_file_script"
#define CONF_UPGRADE_DAEMON_SCRIPT_KEY          "upgrade_daemon_script"
#define CONF_UPGRADE_VIRUSLIB_SCRIPT_KEY        "upgrade_viruslib_script"
#define CONF_UPGRADE_DAEMON_VIRUSLIB_SCRIPT_KEY "upgrade_daemon_viruslib_script"
#define CONF_UPGRADE_LOGFILE_KEY                "logfile"
#define CONF_VERSION_FILE_URL_KEY               "version_file_url"
#define CONF_VERSION_FILE_LOCAL_KEY             "version_file_local"
#define CONF_VERSION_FILE_DOWNLOAD_KEY          "version_file_download"

#define PUBLIC_API __attribute__ ((visibility ("default")))

struct module
{
    module_info_t info;
    std::string logfile;
    std::string download_version_file_script;
    std::string upgrade_daemon_script;
    std::string upgrade_viruslib_script;
    std::string upgrade_daemon_viruslib_script;
    std::string daemon_package;
    std::string viruslib_package;
    std::string version_file_url;
    std::string version_file_local;
    std::string version_file_download;
    VersionManager *daemon_mgr;
    VersionManager *viruslib_mgr;
};

// ret: -1: error, see ERRNO
//      0 : success
static int makedir(std::string dir, mode_t mode)
{
    struct stat st;

    if (dir.size() == 0)
    {
        printf("%s [%s] failed\n", __func__, dir.c_str());
        return -1;
    }

    if (dir[dir.size() - 1] == '/')
    {
        dir.erase(dir.size() - 1);
    }

    if (dir.size() == 0)
    {
        printf("%s [%s] size is 0\n", __func__, dir.c_str());
        return 0;
    }

    if (stat(dir.data(), &st))
    {
        return mkdir(dir.data(), mode);
    }

    if (!S_ISDIR(st.st_mode))
    {
        if (unlink(dir.data()))
        {
            printf("%s [%s] unlink failed\n", __func__, dir.c_str());
            return -1;
        }
        return mkdir(dir.data(), mode);
    }
    return 0;
}

static int makedir_p(std::string dir, mode_t mode)
{
    int ret = -1;
    ret = makedir(dir.data(), mode);
    if (ret == 0)
    {
        printf("%s [%s] success\n", __func__, dir.c_str());
        return 0;
    }

    // parse parent string
    if (dir[dir.size() - 1] == '/')
    {
        dir.erase(dir.size() - 1);
    }

    size_t off = dir.find_last_of('/');
    if (off != std::string::npos)
    {
        std::string parentdir = dir.substr(0, off + 1);
        if(parentdir.size() == 0 || parentdir.size() == 1)
        {
            printf("%s [%s] ok\n", __func__, dir.c_str());
            return 0;
        }
        ret = makedir_p(parentdir, mode);
        if(ret == -1)
        {
            printf("%s [%s] failed, errno:%d\n", __func__, parentdir.c_str(), errno);
            return ret;
        }
        else
        {
            printf("%s [%s] success\n", __func__, parentdir.c_str());
            return makedir(dir.data(), mode);
        }
    }
    printf("%s [%s] failed, errno:%d\n", __func__, dir.c_str(), errno);
    return -1;
}

/* exactly tok the index-th string */
static std::string tok(const std::string str, const std::string sep, size_t index)
{
    size_t ppos = 0, cpos = 0;
    do
    {
        if(cpos)
        {
            ppos = cpos + sep.size();
        }
        cpos = str.find(sep, ppos);

        if(cpos == std::string::npos)
        {
            cpos = str.size();
            break;
        }

    } while(index--);

    return str.substr(ppos, cpos - ppos);
}

static std::string tok_key(const std::string str, const std::string sep)
{
    if(str.size() == 0)
    {
        return "";
    }
    return tok(str, sep, 0);
}
static std::string tok_value(const std::string str, const std::string sep)
{
    if(str.size() == 0)
    {
        return "";
    }
    /* Maybe value contains one or more sep strings */
    return str.substr(tok_key(str, sep).size() + sep.size());
}

static const module_data_t **module_data_ptrs_alloc_impl(uint32_t count)
{
    return new const module_data_t *[count];
}

static void notify_center_agent(module_info_t *p_info, VersionManager *p_mgr)
{
    g_logger.log(__func__);
    module_data_t *p_data =  create_module_data();

    set_module_data_property(p_data, g_p_message_id, TO_CENTER_AGENT_VIRUSLIB_VERSION, strlen(TO_CENTER_AGENT_VIRUSLIB_VERSION));
    set_module_data_property(p_data, TO_CENTER_AGENT_VIRUSLIB_VERSION, p_mgr->curr_version().c_str(), p_mgr->curr_version().length());

    module_message_t module_msg;
    module_msg.category = (module_category_t)(p_info->category);
    module_msg.p_data = p_data;

    mdh_sync_params_t sync_data;
    sync_data.is_sync = false;
    sync_data.ptrs_alloc = module_data_ptrs_alloc_impl;
    sync_data.result.pp_ptrs = NULL;
    sync_data.result.count = 0;

    p_info->notifier(&module_msg, p_info->p_params, &sync_data);

    // Comments: scheduler write:
    // module_data_t *p_module_data = prepare_module_data("MSG_TYPE_ANY",
    //         "NO_HANDLER_MESSAGE", "1");
    if(sync_data.result.pp_ptrs)
    {
        delete[] sync_data.result.pp_ptrs;
    }
    destroy_module_data(p_data);
}

static void notify_rpcsrv(module_info_t *p_info, uint32_t id, uint32_t type, const void *buff, size_t bufflen)
{
    g_logger.log(__func__);
    module_data_t *p_data =  create_module_data();

    set_module_data_property(p_data, g_p_message_id, msg_result_cmd.c_str(), msg_result_cmd.size());
    set_module_data_property(p_data, msg_id_key.c_str(), (const char *)&id, sizeof(id));
    set_module_data_property(p_data, msg_type_key.c_str(), (const char *)&type, sizeof(type));
    set_module_data_property(p_data, msg_payload_key.c_str(), (const char *)buff, bufflen);

    module_message_t module_msg;
    module_msg.category = (module_category_t)(p_info->category | 1);
    module_msg.p_data = p_data;

    mdh_sync_params_t sync_data;
    sync_data.is_sync = true;
    sync_data.ptrs_alloc = module_data_ptrs_alloc_impl;
    sync_data.result.pp_ptrs = NULL;
    sync_data.result.count = 0;

    p_info->notifier(&module_msg, p_info->p_params, &sync_data);

    // Comments: scheduler write:
    // module_data_t *p_module_data = prepare_module_data("MSG_TYPE_ANY",
    //         "NO_HANDLER_MESSAGE", "1");
    if(sync_data.result.pp_ptrs)
    {
        delete[] sync_data.result.pp_ptrs;
    }
    destroy_module_data(p_data);
}

static int run_cmd(const char *fmt, ...)
{
    char cmd[1024];

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), fmt, ap);
    va_end(ap);

    g_logger.log("%s [%s]", __func__, cmd);
    int ret = system(cmd);
    g_logger.log("%s [%s] -> %d", __func__, cmd, WEXITSTATUS(ret));
    return WEXITSTATUS(ret);
}

static bool download_version_file(module_t *p_module)
{
    g_logger.log("%s %s", __func__, p_module->upgrade_daemon_script.c_str());
    int ret = run_cmd("bash %s %s %s >>%s 2>&1",
        p_module->download_version_file_script.c_str(),
        p_module->version_file_url.c_str(),
        p_module->version_file_download.c_str(),
        p_module->logfile.c_str());
    return ret == 1;
}

static int upgrade_daemon(module_t *p_module)
{
    g_logger.log("%s %s", __func__, p_module->upgrade_daemon_script.c_str());
    int maxfd = std::max((int)sysconf(_SC_OPEN_MAX), _POSIX_OPEN_MAX);
    g_logger.log("start close 'max:%d = std::max(sysconf:%d, posix_max:%d)", maxfd, sysconf(_SC_OPEN_MAX), _POSIX_OPEN_MAX);
    sleep(2);
    for(int i = 3; i < maxfd; i++)
    {
        close(i);
    }
    int ret = run_cmd("bash %s %s %s >>%s 2>&1",
        p_module->upgrade_daemon_script.c_str(),
        p_module->daemon_package.c_str(),
        p_module->version_file_download.c_str(),
        p_module->logfile.c_str());
    exit(0);
    return 0;
}

static int upgrade_viruslib(module_t *p_module)
{
    g_logger.log("%s %s", __func__, p_module->upgrade_viruslib_script.c_str());
    int maxfd = std::max((int)sysconf(_SC_OPEN_MAX), _POSIX_OPEN_MAX);
    g_logger.log("start close 'max:%d = std::max(sysconf:%d, posix_max:%d)", maxfd, sysconf(_SC_OPEN_MAX), _POSIX_OPEN_MAX);
    sleep(2);
    for(int i = 3; i < maxfd; i++)
    {
        close(i);
    }
    int ret = run_cmd("bash %s %s >>%s 2>&1",
        p_module->upgrade_viruslib_script.c_str(),
        p_module->viruslib_package.c_str(),
        p_module->logfile.c_str());
    exit(0);
    return 0;
}

static int upgrade_daemon_viruslib(module_t *p_module)
{
    g_logger.log("%s %s", __func__, p_module->upgrade_daemon_viruslib_script.c_str());
    int maxfd = std::max((int)sysconf(_SC_OPEN_MAX), _POSIX_OPEN_MAX);
    g_logger.log("start close 'max:%d = std::max(sysconf:%d, posix_max:%d)", maxfd, sysconf(_SC_OPEN_MAX), _POSIX_OPEN_MAX);
    sleep(2);
    for(int i = 3; i < maxfd; i++)
    {
        close(i);
    }
    int ret = run_cmd("bash %s %s %s %s >>%s 2>&1",
        p_module->upgrade_daemon_viruslib_script.c_str(),
        p_module->daemon_package.c_str(),
        p_module->version_file_download.c_str(),
        p_module->viruslib_package.c_str(),
        p_module->logfile.c_str());
    exit(0);
    return 0;
}

static module_data_t *assign_from_center_agent(module_t *p_module, const module_data_t *p_data, bool is_sync)
{
    g_logger.log(__func__);
    if(download_version_file(p_module))
    {
        return NULL;
    }
    bool if_upgrade_daemon = p_module->daemon_mgr->check_upgrade(p_module->version_file_download);
    bool if_upgrade_viruslib = p_module->viruslib_mgr->check_upgrade(p_module->version_file_download);
    if(if_upgrade_daemon && if_upgrade_viruslib)
    {
        upgrade_daemon_viruslib(p_module);
    }
    else if(if_upgrade_daemon)
    {
        upgrade_daemon(p_module);
    }
    else if(if_upgrade_viruslib)
    {
        upgrade_viruslib(p_module);
    }
    else
    {
        g_logger.log("%s no daemon viruslib would be upgrade.", __func__);
    }

    return NULL;
}

static module_data_t *assign_from_rpcsrv(module *p_module, const module_data_t *p_data, bool is_sync)
{
    g_logger.log("%s\n", __func__);
    const char *type = NULL;
    uint32_t typesz = 0;
    int err = get_module_data_property(p_data, msg_type_key.c_str(), &type, &typesz);
    if(err || type == NULL)
    {
        g_logger.log("%s get type error\n", __func__);
        return NULL;
    }

    const char *id = NULL;
    uint32_t idsz = 0;
    err = get_module_data_property(p_data, msg_id_key.c_str(), &id, &idsz);
    if(err || id == NULL)
    {
        g_logger.log("%s type %" PRIu32 ", get id error\n",
            __func__, (*(uint32_t *)type));
        return NULL;
    }

    const char *payload = NULL;
    uint32_t payloadsz = 0;
    err = get_module_data_property(p_data, msg_payload_key.c_str(), &payload, &payloadsz);
    if(err)
    {
        return NULL;
    }

    char retv = 0;

    bool if_upgrade_daemon = false, if_upgrade_viruslib = false;

    /* check type */
    switch(*(uint32_t *)type)
    {
    case KV_ENGINE_MSG_TYPE_UPDATE_DAEMON:
        g_logger.log("%s type %" PRIu32 " == %" PRIu32 ", continue\n",
            __func__, (*(uint32_t *)type), KV_ENGINE_MSG_TYPE_UPDATE_DAEMON);
        notify_rpcsrv(&p_module->info, *(uint32_t *)id, *(uint32_t *)type, &retv, sizeof(retv));
        if(download_version_file(p_module))
        {
            return NULL;
        }
        if(p_module->daemon_mgr->check_upgrade(p_module->version_file_download))
        {
            upgrade_daemon(p_module);
        }
        else
        {
            g_logger.log("%s KV_ENGINE_MSG_TYPE_UPDATE_DAEMON, check_upgrade -> don't upgrade\n",
                __func__);
        }
        break;
    case KV_ENGINE_MSG_TYPE_UPDATE_VIRUSLIB:
        g_logger.log("%s type %" PRIu32 " == %" PRIu32 ", continue\n",
            __func__, (*(uint32_t *)type), KV_ENGINE_MSG_TYPE_UPDATE_VIRUSLIB);
        notify_rpcsrv(&p_module->info, *(uint32_t *)id, *(uint32_t *)type, &retv, sizeof(retv));
        if(download_version_file(p_module))
        {
            return NULL;
        }
        if(p_module->viruslib_mgr->check_upgrade(p_module->version_file_download))
        {
            upgrade_viruslib(p_module);
        }
        else
        {
            g_logger.log("%s KV_ENGINE_MSG_TYPE_UPDATE_VIRUSLIB, check_upgrade -> don't upgrade\n",
                __func__);
        }
        break;
    case KV_ENGINE_MSG_TYPE_UPDATE_DAEMON_VIRUSLIB:
        g_logger.log("%s type %" PRIu32 " == %" PRIu32 ", continue\n",
            __func__, (*(uint32_t *)type), KV_ENGINE_MSG_TYPE_UPDATE_DAEMON_VIRUSLIB);
        notify_rpcsrv(&p_module->info, *(uint32_t *)id, *(uint32_t *)type, &retv, sizeof(retv));
        if(download_version_file(p_module))
        {
            return NULL;
        }
        if_upgrade_daemon = p_module->daemon_mgr->check_upgrade(p_module->version_file_download);
        if_upgrade_viruslib = p_module->viruslib_mgr->check_upgrade(p_module->version_file_download);
        if(if_upgrade_daemon && if_upgrade_viruslib)
        {
            upgrade_daemon_viruslib(p_module);
        }
        else if(if_upgrade_daemon)
        {
            upgrade_daemon(p_module);
        }
        else if(if_upgrade_viruslib)
        {
            upgrade_viruslib(p_module);
        }
        else
        {
            g_logger.log("%s KV_ENGINE_MSG_TYPE_UPDATE_DAEMON_VIRUSLIB, check_upgrade -> don't upgrade\n",
                __func__);
        }
        break;
    default :
        g_logger.log("%s type %" PRIu32 " != %" PRIu32 ", ignore\n",
            __func__, (*(uint32_t *)type), KV_ENGINE_MSG_TYPE_UPDATE_DAEMON);
        return NULL;
    }

    g_logger.log("%s end\n", __func__);
    return NULL;
}

PUBLIC_API module_data_t* assign(module_t *p_module, const module_data_t *p_data, bool is_sync)
{
    assert(p_module);
    g_logger.log(__func__);

    const char *cmd = NULL;
    uint32_t cmdsz = 0;
    int err = get_module_data_property(p_data, g_p_message_id, &cmd, &cmdsz);
    if(err != 0)
    {
        return NULL;
    }
    if(strncmp(cmd, UPGRADE_MSG_ID, cmdsz) == 0)
    {
        return assign_from_center_agent(p_module, p_data, is_sync);
    }
    else if(strncmp(cmd, msg_req_cmd.c_str(), cmdsz) == 0)
    {
        return assign_from_rpcsrv(p_module, p_data, is_sync);
    }
    // else if(strncmp(cmd, "CENTER_MESSAGE_VIRUS_LIB_DATE", strlen("CENTER_MESSAGE_VIRUS_LIB_DATE")) == 0)
    // {
    //     notify_center_agent(&p_module->info, p_module->viruslib_mgr);
    // }
    return NULL;
}

PUBLIC_API void get_inputted_message_type(module_t *p_module, const char *** const ppp_inputted_message_types,
                                     uint32_t *p_message_type_count)
{
    static const char *msg_types[] =
    {
        UPGRADE_MSG_ID,
        TO_CENTER_AGENT_VIRUSLIB_VERSION,
        msg_req_cmd.c_str(),
    };

    assert(p_module);
    // g_logger.log(__func__);

    *ppp_inputted_message_types = msg_types;
    *p_message_type_count = sizeof(msg_types) / sizeof(msg_types[0]);
}

PUBLIC_API module_t * create(uint32_t category, notify_scheduler_t notifier, void *p_params, uint32_t arg_count, const char **pp_args)
{
    assert(arg_count && pp_args);

    module *p_module = new module;
    p_module->info.category = category;
    p_module->info.notifier = notifier;
    p_module->info.p_params = p_params;
    p_module->info.arg_count = arg_count;
    p_module->info.pp_args = pp_args;
    g_logger.set_prefix("UPGRADE");
    g_logger.set_module_info(&p_module->info);
    // g_logger.log(__func__);
    // p_module->version_manager = NULL;

    for(uint32_t i = 0; i < arg_count; i++)
    {
        g_logger.log("param [%s] parse to [%s]:[%s]",
            pp_args[i],
            tok_key(pp_args[i], ":").c_str(), tok_value(pp_args[i], ":").c_str());

        if(tok_key(pp_args[i], ":").compare(CONF_UPGRADE_DAEMON_SCRIPT_KEY) == 0)
        {
            p_module->upgrade_daemon_script = tok_value(pp_args[i], ":");
        }
        else if(tok_key(pp_args[i], ":").compare(CONF_UPGRADE_VIRUSLIB_SCRIPT_KEY) == 0)
        {
            p_module->upgrade_viruslib_script = tok_value(pp_args[i], ":");
        }
        else if(tok_key(pp_args[i], ":").compare(CONF_UPGRADE_DAEMON_VIRUSLIB_SCRIPT_KEY) == 0)
        {
            p_module->upgrade_daemon_viruslib_script = tok_value(pp_args[i], ":");
        }
        else if(tok_key(pp_args[i], ":").compare(CONF_UPGRADE_LOGFILE_KEY) == 0)
        {
            p_module->logfile = tok_value(pp_args[i], ":");
            /* make dir */
            size_t offset = p_module->logfile.find_last_of('/');
            if(offset != std::string::npos)
            {
                std::string basedir = p_module->logfile.substr(0, offset);
                makedir_p(basedir.c_str(), 0755);
            }
        }
        else if(tok_key(pp_args[i], ":").compare(CONF_UPGRADE_DAEMON_PACKAGE_KEY) == 0)
        {
            p_module->daemon_package = tok_value(pp_args[i], ":");
        }
        else if(tok_key(pp_args[i], ":").compare(CONF_UPGRADE_VIRUSLIB_PACKAGE_KEY) == 0)
        {
            p_module->viruslib_package = tok_value(pp_args[i], ":");
        }
        else if(tok_key(pp_args[i], ":").compare(CONF_VERSION_FILE_URL_KEY) == 0)
        {
            p_module->version_file_url = tok_value(pp_args[i], ":");
        }
        else if(tok_key(pp_args[i], ":").compare(CONF_VERSION_FILE_DOWNLOAD_KEY) == 0)
        {
            p_module->version_file_download = tok_value(pp_args[i], ":");
        }
        else if(tok_key(pp_args[i], ":").compare(CONF_VERSION_FILE_LOCAL_KEY) == 0)
        {
            p_module->version_file_local = tok_value(pp_args[i], ":");
            p_module->daemon_mgr = new DaemonVersionManager;
            p_module->viruslib_mgr = new VirusLibVersionManager;
            p_module->daemon_mgr->init(p_module->version_file_local);
            p_module->viruslib_mgr->init(p_module->version_file_local);
        }
        else if(tok_key(pp_args[i], ":").compare(CONF_DOWNLOAD_VERSION_FILE_SCRIPT_KEY) == 0)
        {
            p_module->download_version_file_script = tok_value(pp_args[i], ":");
        }
    }

    assert(p_module->download_version_file_script.size() && !access(p_module->download_version_file_script.c_str(), F_OK));
    assert(p_module->upgrade_daemon_script.size() && !access(p_module->upgrade_daemon_script.c_str(), F_OK));
    assert(p_module->upgrade_viruslib_script.size() && !access(p_module->upgrade_viruslib_script.c_str(), F_OK));
    assert(p_module->upgrade_daemon_viruslib_script.size() && !access(p_module->upgrade_daemon_viruslib_script.c_str(), F_OK));

    assert(p_module->version_file_local.size() && !access(p_module->version_file_local.c_str(), F_OK));
    assert(p_module->version_file_download.size());
    assert(p_module->daemon_package.size());
    assert(p_module->viruslib_package.size());
    assert(p_module->version_file_url.size());
    return p_module;
}

PUBLIC_API void destroy(module_t *p_module)
{
    assert(p_module);
    // g_logger.log(__func__);
    delete p_module->daemon_mgr;
    delete p_module->viruslib_mgr;
    delete p_module;
}

PUBLIC_API module_state_t run(module_t *p_module)
{
    assert(p_module);
    g_logger.log(__func__);
    notify_center_agent(&p_module->info, p_module->viruslib_mgr);
    return MODULE_OK;
}

PUBLIC_API module_state_t stop(module_t *p_module)
{
    assert(p_module);
    g_logger.log(__func__);
    return MODULE_OK;
}
