
#include "cbd_scheduler.h"
#include "module_config.h"
#include "module_data.h"
#include "utils/utils_bits.h"

#include <cstring>

/////////////////////////////////////////////////////////////////
// Private Implementations
/////////////////////////////////////////////////////////////////
const std::string g_finding_string_separator("|");

static int32_t calculate_category_index(module_category_t category)
{
#ifdef _M_IX86
    int32_t index = 0;
    int32_t tmp_category = 0;
    while(CATEGORY_COUNT > index)
    {
        tmp_category = CATEGORY_BASE_VALUE << index;
        
        if (tmp_category == (tmp_category & category))
            return index;

        ++index;
    }
#else
    // Note: 'category' should be larger than 0.
    return find_lssb_u32(category / CATEGORY_BASE_VALUE);
#endif
}

static void dispatch_to_first(const module_info_list &list, const module_message_t &message)
{
    if (0 < list.size())
    {
        std::shared_ptr<module_info> p_module_info = list.front();
        p_module_info->assign(p_module_info->p_module, message.p_data, false);
    }
}

static void dispatch_to_all(const module_info_list &list, const module_message_t &message)
{
    module_info_list::const_iterator citer = list.begin();
    module_info_list::const_iterator citer_end = list.end();

    for (; citer != citer_end; ++citer)
    {
        std::shared_ptr<module_info> p_module_info = *citer;
        p_module_info->assign(p_module_info->p_module, message.p_data, false);
    }
}

static bool dispatch_message_by_rule(const message_rule &rule, 
        const module_info_lists &candidate_modules, module_message_t &message)
{
    // Note: 'to_category' can not be 0 here.
    if (0 < rule.to_category)
    {
        uint32_t category_index = calculate_category_index(rule.to_category);
        const module_info_list &list = candidate_modules[category_index];
        if (true == rule.need_broadcast)
        {
            dispatch_to_all(list, message);
        }
        else
        {
            dispatch_to_first(list, message);
        }
    }
    return true;
}

// Note: if module_info_lists will be modified, use lock to synchronize.
static bool dispatch_message_to_modules(const message_rules &rules, 
        const module_info_lists &candidate_modules, module_message_t &message)
{
    bool result = true;
    message_rules::const_iterator citer = rules.begin();
    message_rules::const_iterator citer_end = rules.end();
    for (; citer != citer_end; ++citer)
    {
        result &= dispatch_message_by_rule(*citer, candidate_modules, message);
    }
    return result;
}

typedef module_data_ptr_vec (*handle_func)(const module_info_list &list, 
        const module_message_t &message);

static void push_module_data(module_data_ptr_vec &vec, module_data_t* p_data)
{
    if (NULL != p_data)
    {
        vec.push_back(p_data);
    }
}

static module_data_ptr_vec handle_by_first(const module_info_list &list, 
        const module_message_t &message)
{
    module_data_ptr_vec result;
    if (0 < list.size())
    {
        std::shared_ptr<module_info> p_module_info = list.front();
        push_module_data(result, 
                p_module_info->assign(p_module_info->p_module, message.p_data, true));
    }
    return result;
}

static module_data_ptr_vec handle_by_all(const module_info_list &list, 
        const module_message_t &message)
{
    module_data_ptr_vec result;

    module_info_list::const_iterator citer = list.begin();
    module_info_list::const_iterator citer_end = list.end();
    for (; citer != citer_end; ++citer)
    {
        std::shared_ptr<module_info> p_module_info = *citer;
        push_module_data(result, 
                p_module_info->assign(p_module_info->p_module, message.p_data, true));
    }

    return result;
}

static module_data_ptr_vec handle_message_by_rule(const message_rule &rule, 
        const module_info_lists &candidate_modules, const module_message_t &message)
{
    module_data_ptr_vec result;

    // Note: 'to_category' can not be 0 here.
    if (0 < rule.to_category)
    {
        uint32_t category_index = calculate_category_index(rule.to_category);
        const module_info_list &list = candidate_modules[category_index];

        if (true == rule.need_broadcast)
        {
            result = handle_by_all(list, message);
        }
        else
        {
            result = handle_by_first(list, message);
        }
    }
    
    return result;
}

// Note: if module_info_lists will be modified, use lock to synchronize.
static module_data_ptr_vec handle_message_by_modules(const message_rules &rules, 
        const module_info_lists &candidate_modules, const module_message_t &message)
{
    module_data_ptr_vec result;
    message_rules::const_iterator citer = rules.begin();
    message_rules::const_iterator citer_end = rules.end();
    for (; citer != citer_end; ++citer)
    {
        module_data_ptr_vec ptrs = handle_message_by_rule(*citer, candidate_modules, message);
        // TODO: check if this work !!!
        result.insert(result.end(), ptrs.begin(), ptrs.end());
    }
    return result;
}

typedef message_rules_hash_table::const_iterator message_rules_const_iterator;

struct cbd_scheduler::cbd_scheduler_impl
{
    message_rules_hash_table _config_message_rules;

private:
    std::string generate_finding_string(const module_message_t &message)
    {
        const char *p_value = NULL;
        uint32_t value_length = 0;
        // Note: this should be always succesful.
        get_module_data_property(message.p_data, g_p_message_id, &p_value, &value_length);
        return std::string(p_value, value_length);
    }

    message_rules find_in_config_message_rules(const std::string &finding_string, 
            module_category_t category)
    {
        message_rules result;

        std::string specific_finding_string(std::to_string(category) + 
                g_finding_string_separator + 
                finding_string);

        message_rules_hash_table::const_iterator citer;
        citer = _config_message_rules.find(specific_finding_string);
        if (_config_message_rules.end() != citer && 0 < citer->second.size())
        {
            result = citer->second;
        }
        else if (_config_message_rules.end() != 
                (citer = _config_message_rules.find(finding_string)) &&
                0 < citer->second.size())
        {
            result = citer->second;
        }
        else
        {
            // Do nothing.
        }
        
        return result;
    }

    message_rules find_in_original_message_rules(
            const message_rules_hash_table &original_message_rules, 
            const std::string &finding_string)
    {
        message_rules result;
        message_rules_hash_table::const_iterator citer;
        citer = original_message_rules.find(finding_string);
        if (original_message_rules.end() != citer && 0 < citer->second.size())
        {
            result = citer->second;
        }
        return result;
    }

public:
    message_rules find_message_rule(const message_rules_hash_table &original_message_rules, 
            const module_message_t &message)
    {
        message_rules result;
        std::string finding_string = generate_finding_string(message);
        result = find_in_config_message_rules(finding_string, message.category);
        if (0 == result.size())
        {
            result = find_in_original_message_rules(original_message_rules, finding_string);
        }
        return result;
    }
};


/////////////////////////////////////////////////////////////////
// Public Interfaces
/////////////////////////////////////////////////////////////////

cbd_scheduler::cbd_scheduler(const std::string &config_file) :
    scheduler(config_file)
    ,_impl(new cbd_scheduler_impl)
{
    _impl->_config_message_rules = load_message_rules(config_file);
}

cbd_scheduler::~cbd_scheduler(void)
{

}

// If the return value of 'dispatch_module_message' is false, it indicates that 
// the scheduler encounter error(s) and should be stopped.
bool cbd_scheduler::dispatch_module_message(module_info_lists &candidate_modules, 
        const message_rules_hash_table &original_message_rules, module_message_t &message)
{
    bool result = true;
    message_rules rules = _impl->find_message_rule(original_message_rules, message);
    if (0 < candidate_modules.size() && 0 < rules.size())
    {
        result = dispatch_message_to_modules(rules, candidate_modules, message);
    }
    return result;
}

module_data_ptr_vec cbd_scheduler::handle_module_message(module_info_lists &candidate_modules,
            const message_rules_hash_table &original_message_rules, const module_message_t &message) 
{
    module_data_ptr_vec result;
    message_rules rules = _impl->find_message_rule(original_message_rules, message);
    if (0 < candidate_modules.size() && 0 < rules.size())
    {
        result = handle_message_by_modules(rules, candidate_modules, message);
    }
    return result;
}

