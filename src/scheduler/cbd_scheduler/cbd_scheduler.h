
#ifndef CBD_SCHEDULER_HHH
#define CBD_SCHEDULER_HHH

#include "scheduler.h"

#include <string>

// 'cbd' is short for 'configuration based dispatcher'
class cbd_scheduler : public scheduler
{
    public:
        cbd_scheduler(const std::string &config_file); 
        ~cbd_scheduler(void);

    public:
        // N/A

    private:
        // If the return value of 'dispatch_module_message' is false, it indicates that 
        // the scheduler encounter error(s) and should be stopped.
        bool dispatch_module_message(module_info_lists &candidate_modules, 
                const message_rules_hash_table &original_message_rules, 
                module_message &message);
        module_data_ptr_vec handle_module_message(module_info_lists &candidate_modules, 
                const message_rules_hash_table &original_message_rules, 
                const module_message &message);

    private:
        struct cbd_scheduler_impl;
        std::unique_ptr<cbd_scheduler_impl> _impl; 
};

#endif

