#include "rules_filter.h"
#include "rules_processor.h"
#include "debug_print.h"
#include "json/json.h"

#include <time.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

/* product class */
class HUAWEIRuleProcessor:public RuleProcessor
{
public:
    virtual rules_info_t *load_rules(const char *list_path, const char *conf_path);
    virtual bool match(rules_info_t *p_rules, object_type_t type, void *p_info);
    virtual int get_ftp_login(rules_info_t *p_rules, std::string &ip, std::string &user, std::string &passwd);
    virtual std::string get_log_path(rules_info_t *p_rules);
    virtual bool get_permission(rules_info_t *p_rules, int pid, rule_action_t type);
    virtual time_t get_finish_time(rules_info_t *p_rules);
    virtual ~HUAWEIRuleProcessor();
    virtual bool upload_file(int pid);

private:
    std::map<int, object_rules_t> objs;
};

HUAWEIRuleProcessor::~HUAWEIRuleProcessor()
{
}

rules_info_t *HUAWEIRuleProcessor::load_rules(const char *list_path, const char *conf_path)
{
    rules_info_t *p_rules = new rules_info_t;
    if (p_rules && p_rules->rules_load(list_path, conf_path) < 0)
    {
        delete p_rules;
        p_rules = NULL;
    }
    debug_print("%s %s\n", __func__, p_rules ? "OK" : "FAILED");
    return p_rules;
}

bool HUAWEIRuleProcessor::match(rules_info_t *p_rules, object_type_t type, void *p_info)
{
    int pid = 0;
    switch (type)
    {
    case OBJ_DNS:
        pid = ((dns_info_t *)p_info)->pid;
        break;
    case OBJ_TCP:
        pid = ((tcp_info_t *)p_info)->pid;
        break;
    case OBJ_UDP:
        pid = ((udp_info_t *)p_info)->pid;
        break;
    case OBJ_ICMP:
        pid = ((icmp_info_t *)p_info)->pid;
        break;
    default :
        debug_print("%s type:%d, pid error!",
            __func__, type);
        return false;
    }
 
    std::map<int, object_rules_t>::iterator itr = this->objs.find(pid);
    if(itr != this->objs.end())
    {
        // already matched, don't repeat
        return false;
    }

    object_rules_t *p_ret = p_rules->rules_match(type, p_info);
    if(p_ret == NULL)
    {
       return false;
    }

    // store p_ret
    this->objs.insert(std::make_pair(pid, *p_ret));
    
    // NOTE:
    // TODO: deep copy objects
    this->objs[pid].objects.clear();


    debug_print("process %d ---match---> %s\n", pid, p_ret->id.data());

    return true;
}

int HUAWEIRuleProcessor::get_ftp_login(rules_info_t *p_rules, std::string &ip, std::string &user, std::string &passwd)
{
    return p_rules->get_ftp_login(ip, user, passwd);
}

std::string HUAWEIRuleProcessor::get_log_path(rules_info_t *p_rules)
{
    if(!p_rules)
        return NULL;
    return p_rules->get_log_path();
}

bool HUAWEIRuleProcessor::get_permission(rules_info_t *p_rules, int pid, rule_action_t action)
{
    std::map<int, object_rules_t>::iterator itr = this->objs.find(pid);
    if(itr != this->objs.end())
    {
        bool ret = (itr->second.action == rule_action_t::ALL) || (itr->second.action & action);
        if(ret)
        {
            debug_print("%d %s %s\n", pid, __func__, ret ? "success" : "false");
        }
        return ret;
    }
    return false;
}

time_t HUAWEIRuleProcessor::get_finish_time(rules_info_t *p_rules)
{
    return p_rules->get_finish_time();
}

bool HUAWEIRuleProcessor::upload_file(int pid)
{
    bool ret = false;
    std::map<int, object_rules_t>::iterator itr = this->objs.find(pid);
    if(itr != this->objs.end())
    {
        ret = itr->second.upload;
        itr->second.upload = false;
    }
    return ret;
}


/* -------------------------------------- EmptyRuleProcessor -------------------------------------- */


class EmptyRuleProcessor:public RuleProcessor
{
public:
    virtual rules_info_t* load_rules(const char *list_path, const char *conf_path);
    virtual bool match(rules_info_t *p_rules, object_type_t type, void *p_info);
    virtual int get_ftp_login(rules_info_t *p_rules, std::string &ip, std::string &user, std::string &passwd);
    virtual std::string get_log_path(rules_info_t *p_rules);
    virtual bool get_permission(rules_info_t *p_rules, int pid, rule_action_t type);
    virtual time_t get_finish_time(rules_info_t *p_rules);
    virtual bool upload_file(int pid);
};

rules_info_t* EmptyRuleProcessor::load_rules(const char *list_path, const char *conf_path)
{
    debug_print("empty RuleProcessor\n");
    return NULL;
}

bool EmptyRuleProcessor::match(rules_info_t *p_rules, object_type_t type, void *p_info)
{
    return true;
}

int EmptyRuleProcessor::get_ftp_login(rules_info_t *p_rules, std::string &ip, std::string &user, std::string &passwd)
{
    return -1;
}

std::string EmptyRuleProcessor::get_log_path(rules_info_t *p_rules)
{
    return NULL;
}

bool EmptyRuleProcessor::get_permission(rules_info_t *p_rules, int pid, rule_action_t type)
{
    return true;
}

ProcessorFactory::ProcessorFactory() {  }

ProcessorFactory::~ProcessorFactory() {  }

RuleProcessor* ProcessorFactory::create_processor(edr::edition_t edition)
{
    RuleProcessor *p_filter = NULL;

    switch (edition)
    {
    case edr::edition_t::EDITION_EDR:
        p_filter = new EmptyRuleProcessor;
        break;
    case edr::edition_t::EDITION_HUAWEI:
        p_filter = new HUAWEIRuleProcessor;
        break;
    default:
        break;
    }
    return p_filter;
}

time_t EmptyRuleProcessor::get_finish_time(rules_info_t *p_rules)
{
    return 0;
}

bool EmptyRuleProcessor::upload_file(int pid)
{
    return false;
}
