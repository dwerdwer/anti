
#include "scheduler.h"
#include "module_defines.h"
#include "module_config.h"
#include "module_data.h"
#include "utils/utils_library.h"

#include <cstring>
#include <vector>
#include <queue>

#include <unistd.h>
#include <pthread.h>
#include <dlfcn.h>
#include <limits.h>

/////////////////////////////////////////////////////////////////
// Private Implementations
/////////////////////////////////////////////////////////////////

const std::string g_sheduler_failed("scheduler failed");

//const std::string g_library_path("../lib/");
//const std::string g_library_prefix("lib");
//const std::string g_library_suffix(".so");
//const std::string g_library_path("./");

//const std::string g_interface_create("create_");
//const std::string g_interface_destroy("destroy_");
//const std::string g_interface_run("run_");
//const std::string g_interface_stop("stop_");
//const std::string g_interface_assign("assign_");
//const std::string g_interface_get_imt("get_imt_");

const std::string g_interface_create("create");
const std::string g_interface_destroy("destroy");
const std::string g_interface_run("run");
const std::string g_interface_stop("stop");
const std::string g_interface_assign("assign");
const std::string g_interface_get_imt("get_inputted_message_type");

// Related to log message
const std::string g_scheduler_create("scheduler has been created.");
const std::string g_scheduler_destroy("scheduler has been destroyed.");
const std::string g_scheduler_run("scheduler is going to run.");
const std::string g_scheduler_stop("scheduler is going to stop.");

const std::string g_module_create(": has been created.");
const std::string g_module_create_error(": create module failed.");
const std::string g_module_destroy(": has been destroyed.");
const std::string g_module_run(": is going to run.");
const std::string g_module_run_error(": error occurs during running.");
const std::string g_module_run_fatal(": fatal error occurs during running.");
const std::string g_module_stop(": has been stopped.");
const std::string g_module_stop_error(": error occurs during stop.");
const std::string g_module_stop_fatal(": fatal error occurs during stop.");

const std::string g_memory_error_1("acquire memory failed.");

static bool load_server_config(){

    FILE *fp = NULL;
    fp = fopen("../etc/center_ip.txt", "r");
    if (!fp){

        printf("file not found\n");
        return false;
    }

    std::vector<std::string> params;
    while (!feof(fp)) {

        char buf[100] = {0};
        fgets(buf, 100 ,fp);
        if (strlen(buf) > 2 &&  buf[strlen(buf) - 2] == '\r')
            buf[strlen(buf) - 2] = '\0';
        else if (strlen(buf) > 2 && buf[strlen(buf) - 1] == '\n')
            buf[strlen(buf) - 1] = '\0';
        else
            ;
        params.push_back(buf);
    }

    fclose(fp);

    printf("params size is %d\n",params.size());
    if (params.size() < 3)
        return false;  // params count is less
    char runsh[PATH_MAX] = {0};
    std::string config_file = "../etc/";
    config_file.append(params[2]);

    sprintf(runsh, "%s %s %s %s", "sh replace_ip.sh",params[0].c_str() ,params[1].c_str() ,config_file.c_str());
    system(runsh);
    return true;
}

// TODO: main: use signal handler to invoke 'stop'

static bool get_interfaces(std::shared_ptr<module_info> &p_module_info,
        void *p_handle)
{
    bool result = false;

    p_module_info->create = (module_creator_t)get_symbol_address(p_handle,
            (g_interface_create).c_str());
    p_module_info->destroy = (module_destroyer_t)get_symbol_address(p_handle,
            (g_interface_destroy).c_str());
    p_module_info->run = (module_runner_t)get_symbol_address(p_handle, (g_interface_run).c_str());
    p_module_info->stop = (module_stopper_t)get_symbol_address(p_handle, (g_interface_stop).c_str());
    p_module_info->assign = (data_assigner_t)get_symbol_address(p_handle,
            (g_interface_assign).c_str());
    p_module_info->get_imt = (imt_getter_t)get_symbol_address(p_handle,
            (g_interface_get_imt).c_str());
    if (NULL != p_module_info->create &&
            NULL != p_module_info->destroy &&
            NULL != p_module_info->run &&
            NULL != p_module_info->stop &&
            NULL != p_module_info->assign)
    {
        result = true;
    }
    else
    {
        close_library(p_handle);
    }

    return result;
}

static bool load_module_interfaces(std::shared_ptr<module_info> &p_module_info,
        const module_setting &setting)
{
    bool result = false;

    void *p_handle = open_library(setting.path_name.c_str(), RTLD_LAZY | RTLD_LOCAL);

    if (NULL != p_handle)
    {
        result = get_interfaces(p_module_info, p_handle);
    }
    return result;
}

static module_data_t *prepare_module_data(const char *p_id,
        const char *p_body_name, const std::string &message_body)
{
    module_data_t *p_result = NULL;

    module_data_t *p_temp = NULL;

    if (NULL != (p_temp = create_module_data()) &&
        0 == set_module_data_property(p_temp, g_p_message_id, p_id, strlen(p_id)) &&
        0 == set_module_data_property(p_temp, p_body_name,
                                      message_body.c_str(), (uint32_t)message_body.length()) )
    {
        p_result = p_temp;
    }
    else if (NULL != p_temp)
    {
        destroy_module_data(p_temp);
    }
    else
    {
        // Do nothing.
    }

    return p_result;
}

static bool copy_module_message(const module_message_t *p_old_message,
                                module_message_t *p_new_message)
{
    bool result = false;
    module_data_t *p_temp = copy_module_data(p_old_message->p_data);
    if (NULL != p_temp)
    {
        p_new_message->category = p_old_message->category;
        p_new_message->p_data = p_temp;
        result = true;
    }
    return result;
}

static void destroy_module_message(module_message_t &message)
{
    delete [] (char*)(message.p_data);
}

typedef std::unordered_map<std::string, message_rules> message_rules_hash_table;

struct scheduler::scheduler_impl
{
private:
    struct thread_params
    {
        scheduler_impl *p_scheduler_impl;
        std::shared_ptr<module_info> p_module_info;
    };

public:
    bool _is_pending_stop;
    bool _is_continued;
    // TODO: use sync_object instead.
    pthread_spinlock_t _lock;
    std::queue<module_message> _messages;
    module_info_lists _module_infos;
    message_rules_hash_table _original_message_rules;

private:
    // TODO: use closure instead.
    typedef void (scheduler::scheduler_impl::* module_info_handler)(std::shared_ptr<module_info> &p_info);

    void traverse_module_infos(const module_info_handler handler)
    {
        module_info_lists::iterator lists_iter = _module_infos.begin();
        module_info_lists::iterator lists_iter_end = _module_infos.end();

        for(; lists_iter != lists_iter_end; ++lists_iter)
        {
            module_info_list::iterator list_iter = lists_iter->begin();
            module_info_list::iterator list_iter_end = lists_iter->end();
            for(; list_iter != list_iter_end; ++list_iter)
            {
                (this->*handler)(*list_iter);
            }
        }
    }

    void destroy_module(std::shared_ptr<module_info> &p_info)
    {
        scheduler_impl *p_scheduler_impl = (scheduler_impl*)(p_info->p_auxiliary_data);
        p_info->destroy(p_info->p_module);
        p_info->p_module = NULL;
        generate_log_message(p_scheduler_impl, p_info->name + g_module_destroy);
    }

    void stop_module(std::shared_ptr<module_info> &p_info)
    {
        scheduler_impl *p_scheduler_impl = (scheduler_impl*)(p_info->p_auxiliary_data);
        module_state_t state = p_info->stop(p_info->p_module);
        switch(state)
        {
        case MODULE_OK:
            generate_log_message(p_scheduler_impl, p_info->name + g_module_stop);
            break;
        case MODULE_ERROR:
            generate_log_message(p_scheduler_impl, p_info->name + g_module_stop_error);
            break;
        case MODULE_FATAL:
            generate_log_message(p_scheduler_impl, p_info->name + g_module_stop_fatal);
            destroy_module(p_info);
            break;
        default:
            break;
        }
    }

    void run_module(std::shared_ptr<module_info> &p_info)
    {
        if (true == p_info->need_isolation)
        {
            pthread_t thread;
            pthread_create(&thread, NULL, run_module_internal, &p_info);
        }
        else
        {
            run_module_internal(&p_info);
        }
    }

    bool store_module_info(std::shared_ptr<module_info> &p_info, const module_setting &setting)
    {
        bool result = false;

        uint32_t index = 0;
        uint32_t category = 0;
        while(CATEGORY_COUNT > index)
        {
            category = CATEGORY_BASE_VALUE << index;
            if (category == (category & setting.category))
            {
                try
                {
                    _module_infos[index].push_back(p_info);
                    result = true;
                }
                catch(std::bad_alloc &e)
                {
                    // TODO: find a way to log memory-failure message.
                    //generate_log_message(this, g_memory_error_1);
                }
            }
            ++index;
        }

        return result;
    }

    std::shared_ptr<module_info> create_module_info(const module_setting &setting,
                                                    void *p_params)
    {
        std::shared_ptr<module_info> p_result(new module_info);
        if (true == (bool)p_result &&
            true == initialize_module_info(p_result, setting, p_params))
        {
            p_result->need_isolation = setting.need_isolation;
            p_result->name = setting.module_name;
            p_result->p_auxiliary_data = this;
        }
        else if (true == (bool)p_result)
        {
            p_result.reset();
        }
        else
        {
            // Do nothing.
        }
        return p_result;
    }

    bool load_module_info(const module_setting &setting, void *p_notify_params)
    {
        bool result = false;
        std::shared_ptr<module_info> p_info = create_module_info(setting, p_notify_params);
        if (true == (bool)p_info)
        {
            result = store_module_info(p_info, setting);
            generate_log_message(this, setting.module_name + g_module_create);
        }
        else
        {
            generate_log_message(this, setting.module_name + g_module_create_error);
        }
        return result;
    }

    void convert_to_message_rule(module_category_t category, const char *p_inputted_message_type)
    {
        if (NULL != p_inputted_message_type)
        {
            uint32_t index = 0;
            uint32_t test_category = 0;
            while(CATEGORY_COUNT > index)
            {
                test_category = CATEGORY_BASE_VALUE << index;
                if (test_category == (test_category & category))
                {
                    try
                    {
                        message_rule rule;
                        rule.to_category = static_cast<module_category_t>(test_category);
                        rule.need_broadcast = true;
                        std::string rule_string(p_inputted_message_type);
                        _original_message_rules[rule_string].push_back(rule);
                    }
                    catch(std::bad_alloc &e)
                    {
                        // TODO: find a way to log memory-failure message.
                        //generate_log_message(this, g_memory_error_1);
                    }
                }
                ++index;
            }
        }
    }

    void load_imt(std::shared_ptr<module_info> &p_info)
    {
        const char ** pp_inputted_message_types = NULL;
        uint32_t message_type_count = 0;
        p_info->get_imt(p_info->p_module, &pp_inputted_message_types, &message_type_count);

        uint32_t i = 0;
        for(; i < message_type_count; ++i)
        {
            convert_to_message_rule(p_info->category, pp_inputted_message_types[i]);
        }
    }

    void load_module_imt(void)
    {
        traverse_module_infos(&scheduler::scheduler_impl::load_imt);
    }

    bool load_module(const module_setting &setting, void *p_notify_params)
    {
        bool result = load_module_info(setting, p_notify_params);
        if (true == result)
        {
            load_module_imt();
        }
        return result;
    }

    bool load_modules_internal(const std::vector<module_setting> &module_settings,
                               void *p_notify_params)
    {
        bool result = false;
        std::vector<module_setting>::const_iterator citer = module_settings.begin();
        std::vector<module_setting>::const_iterator citer_end = module_settings.end();
        for(; citer != citer_end; ++citer)
        {
            if (true == citer->need_load)
            {
                result |= load_module(*citer, p_notify_params);
            }
        }
        return result;
    }

public:
    void destroy_modules(void)
    {
        traverse_module_infos(&scheduler::scheduler_impl::destroy_module);
    }

    bool load_modules(const std::string &config_file, void *p_notify_params)
    {
        bool result = false;
        std::vector<module_setting> module_settings = load_module_settings(config_file);
        if (0 < module_settings.size())
        {
            result = load_modules_internal(module_settings, p_notify_params);
            destroy_module_settings(module_settings);
        }
        return result;
    }

    void run_modules(void)
    {
        traverse_module_infos(&scheduler::scheduler_impl::run_module);
    }

    void stop_modules(void)
    {
        traverse_module_infos(&scheduler::scheduler_impl::stop_module);
    }

    bool pop_module_message(module_message_t &msg)
    {
        bool result = false;
        if (0 < _messages.size())
        {
            pthread_spin_lock(&_lock);
            msg = _messages.front();
            _messages.pop();
            pthread_spin_unlock(&_lock);
            result = true;
        }
        else
        {
            // sleep 1s
            usleep(1000000);
        }
        return result;
    }

    void push_module_message(const module_message_t &message)
    {
        pthread_spin_lock(&_lock);
        // Note: let std::bad_alloc throw out.
        if (false == _is_pending_stop)
        {
            try
            {
                _messages.push(message);
            }
            catch (std::bad_alloc &e)
            {
                // TODO: find a way to log memory-failure message.
            }
        }
        pthread_spin_unlock(&_lock);
    }

public:
    static void generate_log_message(scheduler_impl *p, const std::string &message_body)
    {
        module_data_t *p_data = prepare_module_data("MSG_TYPE_LOG", "LOG_MESSAGE", message_body);
        if (NULL != p_data)
        {
            module_message_t message;
            message.category = CATEGORY_SCHEDULER;
            message.p_data = p_data;
            p->push_module_message(message);
        }
    }

    static void *run_module_internal(void *p_params)
    {
        std::shared_ptr<module_info> *pp_info = (std::shared_ptr<module_info> *)p_params;
        scheduler_impl *p_scheduler_impl = (scheduler_impl*)((*pp_info)->p_auxiliary_data);
        module_state_t state = (*pp_info)->run((*pp_info)->p_module);
        switch(state)
        {
        case MODULE_OK:
            generate_log_message(p_scheduler_impl, (*pp_info)->name + g_module_run);
            break;
        case MODULE_ERROR:
            generate_log_message(p_scheduler_impl, (*pp_info)->name + g_module_run_error);
            break;
        case MODULE_FATAL:
            generate_log_message(p_scheduler_impl, (*pp_info)->name + g_module_run_fatal);
            p_scheduler_impl->destroy_module(*pp_info);
            break;
        default:
            break;
        }
        return NULL;
    }

    static void notify_module_message(scheduler_impl *p_scheduler_impl,
                                      const module_message_t *p_module_message)
    {
        module_message_t message;
        bool create_result = copy_module_message(p_module_message, &message);
        if (true == create_result)
        {
            p_scheduler_impl->push_module_message(message);
        }
    }

    static void prepare_with_no_handler(module_data_ptr_vec &ptr_vec)
    {
        if (0 == ptr_vec.size())
        {
            // If no handler to deal with message, add a failed message
            module_data_t *p_module_data = prepare_module_data("MSG_TYPE_ANY",
                    "NO_HANDLER_MESSAGE", "1");
            ptr_vec.push_back(p_module_data);
        }
    }

    static void prepare_mdh_result(mdh_sync_params_t *p_sync, module_data_ptr_vec &ptr_vec)
    {
        prepare_with_no_handler(ptr_vec);
        // Alloc array memory to fill result in.
        p_sync->result.pp_ptrs = p_sync->ptrs_alloc(ptr_vec.size());
        memcpy(p_sync->result.pp_ptrs, ptr_vec.data(), ptr_vec.size() * sizeof(module_data_t*));
        p_sync->result.count = ptr_vec.size();
    }

    static void notify_scheduler(const module_message_t *p_module_message,
                                 void *p_params, mdh_sync_params_t *p_sync)
    {
        if (NULL != p_module_message &&
            // Note: property 'g_p_message_id' should be in module data.
            0 == get_module_data_property(p_module_message->p_data,
                                          g_p_message_id, NULL, NULL) &&
            NULL != p_params &&
            NULL != p_sync)
        {
            scheduler *p = (scheduler*)p_params;
            if (false == p_sync->is_sync)
            {
                notify_module_message(p->_impl.get(), p_module_message);
            }
            else
            {
                module_data_ptr_vec temp = p->handle_module_message(p->_impl->_module_infos,
                                                                    p->_impl->_original_message_rules, *p_module_message);
                prepare_mdh_result(p_sync, temp);
            }
        }
    }

    static bool create_module(std::shared_ptr<module_info> &p_module_info,
                              module_category_t category, uint32_t arg_count, const char **p_args, void *p_params)
    {
        bool result = false;
        p_module_info->p_module = p_module_info->create(category, notify_scheduler, p_params,
                                                        arg_count, p_args);
        if (NULL != p_module_info->p_module)
        {
            p_module_info->category = category;
            result = true;
        }
        return result;
    }

    static bool initialize_module_info(std::shared_ptr<module_info> &p_module_info,
                                       const module_setting &setting, void *p_params)
    {
        bool result = load_module_interfaces(p_module_info, setting);
        if (true == result)
        {
            result = create_module(p_module_info, setting.category,
                                   setting.argument_count, setting.p_arguments, p_params);
        }
        return result;
    }

};

/////////////////////////////////////////////////////////////////
// Public Interfaces
/////////////////////////////////////////////////////////////////
scheduler::scheduler(const std::string &config_file) throw (const char*) :
    _impl(new scheduler_impl)
{
    // Note: let 'std::bad_alloc' throw out.
    if (true == (bool)_impl)
    {
        _impl->_is_pending_stop = false;
        _impl->_is_continued = true;
        _impl->_module_infos.resize(CATEGORY_COUNT);
        int32_t init_result = pthread_spin_init(&(_impl->_lock), 0);
        bool load_server = load_server_config();
        if (!load_server)
            scheduler_impl::generate_log_message(_impl.get(), "server config file not found");
        bool load_result = _impl->load_modules(config_file, this);
        if (0 != init_result || false == load_result)
        {
            throw g_sheduler_failed.c_str();
        }
        else
        {
            scheduler_impl::generate_log_message(_impl.get(), g_scheduler_create);
        }
    }
}

scheduler::~scheduler(void)
{
    // Note: destroy all modules
    // TODO: first, check whether scheduler and its modules are running (by a flag? it is set when calling 'run()' and is reset when calling 'stop()'),
    // if they are running, do forcefully stop (calling 'stop()' is a gentle way);
    // then do the destroys as below
    pthread_spin_destroy(&(_impl->_lock));
    _impl->destroy_modules();
    scheduler_impl::generate_log_message(_impl.get(), g_scheduler_destroy);
}

void scheduler::run(void)
{
    // TODO: store running thread id;

    scheduler_impl::generate_log_message(_impl.get(), g_scheduler_run);

    _impl->run_modules();

    while(true == _impl->_is_continued)
    {
        module_message_t message;
        if (true == _impl->pop_module_message(message))
        {
            // Note: if logger module can not work or is missing, printf log message to 'stderr'.
            _impl->_is_continued = dispatch_module_message(_impl->_module_infos,
                                                           _impl->_original_message_rules, message);
            destroy_module_message(message);
        }
        else if (true == _impl->_is_pending_stop)
        {
            _impl->_is_continued = false;
            _impl->_is_pending_stop = false;
        }
        else
        {   // 0.1s
            usleep(100000);
        }
    }
}

void scheduler::stop(void)
{
    scheduler_impl::generate_log_message(_impl.get(), g_scheduler_stop);
    // TODO: get current thread id and compare it with that stored during running.
    // If they are not equal, do the following actions, otherwise only call 'stop_modules()'
    _impl->_is_pending_stop = true;
    while(true == _impl->_is_pending_stop);
    _impl->stop_modules();
}

