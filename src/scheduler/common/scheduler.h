
#ifndef SCHEDULER_HHH
#define SCHEDULER_HHH

#include "module_interfaces.h"
#include "module_defines.h"
#include "message_rule_defines.h"

#include <list>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

struct module_info
{
    module_t *p_module;
    module_creator_t create;
    module_destroyer_t destroy;
    module_runner_t run;
    module_stopper_t stop;
    data_assigner_t assign;
    imt_getter_t get_imt;
    std::string name;
    module_category_t category;
    bool need_isolation;
    void *p_auxiliary_data;
};

typedef std::list<std::shared_ptr<module_info> > module_info_list;
typedef std::vector<module_info_list> module_info_lists;

typedef std::vector<module_data_t*> module_data_ptr_vec;

class scheduler
{
public:
    scheduler(const std::string &config_file) throw (const char*);
    void run(void);
    void stop(void);

public:
    virtual ~scheduler(void) = 0;

private:
    // If the return value of 'dispatch_module_message' is false, it indicates that 
    // the scheduler encounter error(s) and should be stopped.
    virtual bool dispatch_module_message(module_info_lists &candidate_modules, 
            const message_rules_hash_table &original_message_rules, 
            module_message &message) = 0;
    virtual module_data_ptr_vec handle_module_message(module_info_lists &candidate_modules,
            const message_rules_hash_table &original_message_rules, 
            const module_message &message) = 0;

private:
    struct scheduler_impl;
    std::unique_ptr<scheduler_impl> _impl; 
};

#endif

