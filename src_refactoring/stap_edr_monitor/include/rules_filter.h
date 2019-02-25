#pragma once

#include <string>
#include <vector>

#include "json/json.h"
#include "packet_info.h"

typedef enum
{
    OBJ_UNKNOWN = 0,
    OBJ_TCP,
    OBJ_UDP,
    OBJ_DNS,
    OBJ_ICMP
}object_type_t;

typedef enum
{
    ALL             = 0x0,
    EPROC_INFO      = 0x1,
    EPROC_MOD       = 0x2,
    EPROC_ACTION    = 0x4,
    EPROC_FILE      = 0x8,
    EPROC_NET       = 0x20,
    EPROC_DNS       = 0x40
} rule_action_t;

class BaseObject
{
public:
    virtual bool match(void *p_data) = 0;
    virtual std::string to_string() { return "BaseObject-Default to_string"; }
    virtual ~BaseObject() = 0;
};

struct object_rules_t
{
    std::string id;
    std::vector<BaseObject *> objects;

    object_type_t type      = OBJ_UNKNOWN;
    rule_action_t action    = ALL;
    bool upload             = false;
    time_t finish_time      = 0;
};

struct rules_info_t
{
public:
    rules_info_t();
    rules_info_t(const char *list_path, const char *conf_path);
    int rules_load(const char *list_path, const char *conf_path);
    ~rules_info_t();
    object_rules_t *rules_match(object_type_t type, void *p_info);
    std::string get_log_path();
    int get_ftp_login(std::string &ip, std::string &user, std::string &passwd);
    time_t get_finish_time();

private:
    std::vector <std::string> white_list;

    object_rules_t *rules_array;
    size_t rules_count;

    int max_file_size;
    time_t finish_time;
    std::string output_path;
    std::string srv_ip;
    std::string user;
    std::string passwd;

    int decode_ftp_login(std::string encode_str);
    void dump_rules_info();
    int assignment_rules(Json::Value &json_data);
    int parse_json_file(const char *conf_path);
    void parse_list_file(const char *white_list_path);
    object_rules_t *rules_match_dns(dns_info_t *p_info);
    object_rules_t *rules_match_tcp(tcp_info_t *p_info);
};
