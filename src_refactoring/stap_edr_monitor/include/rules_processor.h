#pragma once

#include <string>
#include "rules_filter.h"
#include "edition.h"

/* abstract product class */
class RuleProcessor
{
public:
    virtual rules_info_t* load_rules(const char *list_path, const char *conf_path) = 0;
    virtual int get_ftp_login(rules_info_t *p_rules, std::string &ip, std::string &user, std::string &passwd) = 0;
    virtual bool match(rules_info_t *p_rules, object_type_t type, void *p_info) = 0;
    virtual std::string get_log_path(rules_info_t *p_rules) = 0;
    virtual bool get_permission(rules_info_t *p_rules, int pid, rule_action_t action) = 0;
    virtual time_t get_finish_time(rules_info_t *p_rules) = 0;
    virtual bool upload_file(int pid) = 0;
    virtual ~RuleProcessor() = 0;
};

/* factory class */
class ProcessorFactory
{
public:
    ProcessorFactory();
    ~ProcessorFactory();

    RuleProcessor *create_processor(edr::edition_t edition);
};
