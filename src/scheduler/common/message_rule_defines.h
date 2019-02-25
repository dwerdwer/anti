#ifndef MESSAGE_RULE_DEFINES_HHH
#define MESSAGE_RULE_DEFINES_HHH

#include <vector>
#include <unordered_map>

struct message_rule
{
    module_category_t to_category;
    bool need_broadcast;
};

typedef std::vector<message_rule> message_rules;
typedef std::unordered_map<std::string, message_rules> message_rules_hash_table;


#endif

