#include "defs.h"
#include "packet_info.h"
#include "rules_filter.h"
#include "debug_print.h"
#include "json/json.h"

#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

static const uint32_t WILDCARD_ADDR = htonl(0xffffffff);
static const uint16_t WILDCARD_PORT = htons(0xffff);

class IPObject : public BaseObject
{
public:
    IPObject(std::string &dest_ip);
    IPObject(const char *p_dest_ip = NULL);
    ~IPObject() {};
    virtual bool match(void *p_iphdr);
    virtual std::string to_string();
    uint32_t daddr;  /* network-byte-order */
};

IPObject::IPObject(std::string &dest_ip) :
    IPObject(dest_ip.size() ? dest_ip.data() : NULL)
{
}

IPObject::IPObject(const char *p_dest_ip) :
    daddr(WILDCARD_ADDR)
{
    if(p_dest_ip && strchr(p_dest_ip, '*') == NULL)
    {
        this->daddr = inet_addr(p_dest_ip);
    }
}

bool IPObject::match(void *p_data)
{
    ip_hdr_t *p_iphdr = (ip_hdr_t *)p_data;
    bool ret = true;

    if(ret && p_iphdr == NULL)
    {
        ret = false;
    }
    if(ret && this->daddr != WILDCARD_ADDR 
        && this->daddr != p_iphdr->destination_address)
    {
        ret = false;
    }
    return ret;
}

std::string IPObject::to_string()
{
    char ip[16] = {0};
    inet_ntop(AF_INET, &this->daddr, ip, sizeof(ip));
    return std::string ("IPObject") + ' ' + std::string (ip);
}

class TCPObject : public IPObject
{
public:
    TCPObject(std::string &dest_ip, std::string &sport, std::string &dport);
    TCPObject(const char *p_dest_ip, const char *p_sport, const char *p_dport);
    virtual bool match(void *p_data);
    virtual std::string to_string();
    ~TCPObject() {};
    uint16_t sport;
    uint16_t dport;
};

TCPObject::TCPObject(const char *p_dest_ip, const char *p_sport, const char *p_dport) :
    IPObject(p_dest_ip), sport(WILDCARD_PORT), dport(WILDCARD_PORT)
{
    if(p_sport && strchr(p_sport, '*') == NULL)
    {
        sport = htons(std::atoi(p_sport));
    }
    if(p_dport && strchr(p_dport, '*') == NULL)
    {
        dport = htons(std::atoi(p_dport));
    }
}

TCPObject::TCPObject(std::string &dest_ip, std::string &sport, std::string &dport) :
    TCPObject(dest_ip.size() ? dest_ip.data() : NULL,
        sport.size() ? sport.data() : NULL,
        dport.size() ? dport.data() : NULL)
{
}

bool TCPObject::match(void *p_data)
{
    tcp_info_t *p_tcp = (tcp_info_t *)p_data;
    bool ret = true;

    if(ret && p_tcp == NULL)
    {
        ret = false;
    }
    if(ret && this->daddr != WILDCARD_ADDR
        && this->daddr != p_tcp->ip_hdr.destination_address)
    {
        ret = false;
    }
    if(ret && this->sport != WILDCARD_PORT
        && this->sport != p_tcp->tcp_hdr.source_port)
    {
        ret = false;
    }
    if(ret && this->dport != WILDCARD_PORT
        && this->dport != p_tcp->tcp_hdr.destination_port)
    {
        ret = false;
    }
#ifdef DEBUG
    static uint32_t debug_addr1 = inet_addr("192.168.6.84");
    static uint32_t debug_addr2 = inet_addr("192.168.6.174");
    static uint32_t debug_addr3 = inet_addr("192.168.6.235");
    if(p_tcp->ip_hdr.destination_address == debug_addr1
        || p_tcp->ip_hdr.destination_address == debug_addr2
        || p_tcp->ip_hdr.destination_address == debug_addr3)
    {
        debug_print("TCP %5d:%8x:%5d --match--> %5d:%8x:%5d , %p,ret:%d\n", 
            this->sport, this->daddr, this->dport,
            p_tcp->tcp_hdr.source_port,
            p_tcp->ip_hdr.destination_address, 
            p_tcp->tcp_hdr.destination_port,
            p_tcp,
            ret
            );
    }
#endif
    return ret;
}

std::string TCPObject::to_string()
{
    char ip[16] = {0};
    inet_ntop(AF_INET, &this->daddr, ip, sizeof(ip));
    return std::string ("TCPObject") + ' ' + ip + ' ' + \
            std::to_string(this->sport) + ' ' + std::to_string(this->dport);
}

class UDPObject : public TCPObject
{
};

class HTTPObject : public TCPObject
{

};

class DNSObject : public TCPObject
{
public:
    DNSObject(std::string &query_domain, std::string &dns_server, std::string &response_ip);
    DNSObject(const char *p_query_domain = "", const char *p_dns_server = "", const char *p_response_ip = "");
    virtual bool match(void *p_data);
    virtual std::string to_string();
    ~DNSObject() {};
    std::string query_domain;
    uint32_t response_ip;
    uint32_t dns_server;
};

DNSObject::DNSObject(const char *p_query_domain, const char *p_dns_server, const char *p_response_ip) :
    TCPObject(p_response_ip, NULL, NULL),
    response_ip(WILDCARD_ADDR), dns_server(WILDCARD_ADDR)
{
    if(p_query_domain)
    {
        query_domain = p_query_domain;
    }
    if(p_dns_server && strchr(p_dns_server, '*') == NULL)
    {
        this->dns_server = inet_addr(p_dns_server);
    }
    if(p_response_ip)
    {
        this->response_ip = inet_addr(p_response_ip);
    }
}

DNSObject::DNSObject(std::string &query_domain, std::string &dns_server, std::string &response_ip) :
    DNSObject(query_domain.size() ? query_domain.data() : NULL,
        dns_server.size() ? dns_server.data() : NULL,
        response_ip.size() ? response_ip.data() : NULL)
{
}

bool DNSObject::match(void *p_data)
{
    dns_info_t *p_dns = (dns_info_t *)p_data;
    bool ret = true;

    if(ret && p_dns == NULL)
    {
        ret = false;
    }
    if(ret && strstr(p_dns->p_dns_query, this->query_domain.data()) == NULL)
    {
        ret = false;
    }
    if(ret && this->dns_server != WILDCARD_ADDR
        && this->dns_server != p_dns->ip_hdr.destination_address)
    {
        ret = false;
    }
    if(ret && this->response_ip != WILDCARD_ADDR
        && this->response_ip != p_dns->response_address)
    {
        ret = false;
    }
    return ret;
}

std::string DNSObject::to_string()
{
    char ip[16] = {0};
    inet_ntop(AF_INET, &this->daddr, ip, sizeof(ip));

    char dns_server[16] = {0};
    inet_ntop(AF_INET, &this->dns_server, dns_server, sizeof(dns_server));
    return std::string ("DNSObject") + \
            "query_domain:" + this->query_domain +\
            ", response_ip" + ip + \
            ", dns_server:" + dns_server;
}

class ICMPObject : public IPObject
{
public:
    ICMPObject(std::string &dest_ip);
    ICMPObject(const char *p_dest_ip);
    virtual std::string to_string();
};

ICMPObject::ICMPObject(const char *p_dest_ip)
{

}

ICMPObject::ICMPObject(std::string &dest_ip) :
    IPObject(dest_ip.size() ? dest_ip.data() : NULL)
{

}

std::string ICMPObject::to_string()
{
    char ip[16] = {0};
    inet_ntop(AF_INET, &this->daddr, ip, sizeof(ip));

    return std::string ("ICMPObject") + ' ' + ip;;
}

rules_info_t::rules_info_t()
    : rules_array(NULL), rules_count(0), max_file_size(0), finish_time(0)
{
}


rules_info_t::~rules_info_t()
{
    if (this->rules_array)
    {
        for (size_t i = 0; i < this->rules_count; i++)
        {
            object_rules_t *p_rule = &this->rules_array[i];
            if(p_rule == NULL)
            {
                continue;
            }
            for (std::vector<BaseObject *>::iterator itr = p_rule->objects.begin();
                itr != p_rule->objects.end(); ++itr)
            {
                if(*itr)
                {
                    delete *itr;
                }
            }
        }
        delete [] this->rules_array;
        this->rules_array = NULL;
        rules_count = 0;
    }
}

void rules_info_t::dump_rules_info()
{
    debug_print("rules_count:       %lu\n", this->rules_count);

    for (size_t i = 0; i < this->rules_count; i++)
    {
        debug_print("\n");
        debug_print("rules[%lu].id:              %s\n", i, this->rules_array[i].id.c_str());

        debug_print("rules[%lu].type:            %d\n", i, this->rules_array[i].type);

        debug_print("rules[%lu].object_count:    %lu\n", i, this->rules_array[i].objects.size());
        
        size_t j = 0;
        for (std::vector<BaseObject *>::iterator itr = this->rules_array[i].objects.begin();
            itr != this->rules_array[i].objects.end(); ++itr)
        {
            debug_print("   objects[%lu]:        %s\n", 
                        j++, (*itr)->to_string().data());
        }
        if (this->rules_array[i].upload)
        {
            debug_print("rules[%lu].upload:          true\n", i);
        }
        else
        {
            debug_print("rules[%lu].upload:          false\n", i);
        }
        debug_print("rules[%lu].finish_time:     %lu\n", i, this->rules_array[i].finish_time);
    }

    debug_print("\nmax_file_size:   %d\n", this->max_file_size);
    debug_print("output_path:       %s\n", this->output_path.c_str());
    debug_print("srv_ip:            %s\n", this->srv_ip.c_str());
    debug_print("user:              %s\n", this->user.c_str());
    debug_print("passwd:            %s\n", this->passwd.c_str());
}

static std::vector<std::string> split(std::string str,std::string pattern)
{
    std::string::size_type pos;
    std::vector<std::string> result;
    str += pattern;
    int size=str.size();
 
    for(int i = 0; i < size; i++)
    {
        pos = str.find(pattern, i);
        if(pos == std::string::npos)
        {
            break;
        }
        result.push_back(str.substr(i, pos - i));
        i = pos + pattern.size() - 1;
    }
    return result;
}

static BaseObject *parse_object(object_type_t type, std::string s)
{
    BaseObject *obj = NULL;

    std::vector<std::string> vec = split(s, ":");

    if((type == OBJ_TCP || type == OBJ_UDP)
        && vec.size() == 3)
    {
        obj = new TCPObject(vec[1], vec[0], vec[2]);
    }
    else if(type == OBJ_DNS && vec.size() == 3)
    {

        obj = new DNSObject(vec[0], vec[1], vec[2]);
    }
    else if(type == OBJ_ICMP && vec.size() == 1)
    {
        obj = new ICMPObject(vec[0]);
    }
    return obj;
}

int rules_info_t::assignment_rules(Json::Value &json_data)
{
    int result = -1;
    int upload_flag = 0;
    
    Json::Value &rules = json_data["rules"];
    for (size_t i = 0; i < this->rules_count; i++)
    {
        Json::Value &rule = rules[(Json::ArrayIndex)i];
        object_rules_t *p_rule = &this->rules_array[i];

        p_rule->id = rule["rule_id"].asString();

        p_rule->type = 
            (object_type_t)rule["object_type"].asInt();

        for (size_t j = 0, sz = rule["objects"].size(); 
            j < sz; j++)
        {
            BaseObject *obj = parse_object(p_rule->type, rule["objects"][(Json::ArrayIndex)j].asString());
            if(obj == NULL)
            {
                return -1;
            }
            p_rule->objects.push_back(obj);
        }
        p_rule->action = 
            (rule_action_t)rule["action"].asInt();

        upload_flag = rule["upload_file"].asInt();

        if (1 == upload_flag)
        {
            p_rule->upload = true; 
        }
        else
        {
            p_rule->upload = false; 
        }
        p_rule->finish_time = 
            (time_t)rule["finish_time"].asInt();
        if(this->finish_time < p_rule->finish_time)
        {
            this->finish_time = p_rule->finish_time;
        }
    }

    result = 0;
    return result;
}

int rules_info_t::decode_ftp_login(std::string str)
{
    for(size_t i=0; i<str.size(); i++)
    {
        str[i] -= 1;
    }

    std::string s[3];
    size_t pos = 0, tpos = 0;
    size_t i = 0;
    for(i=0; i<sizeof(s)/sizeof(s[0]) - 1; i++)
    {
        tpos = str.find_first_of(':', pos);
        if(tpos == std::string::npos)
        {
            return -1;
        }
        s[i] = str.substr(pos, tpos-pos);
        pos = tpos + 1;
    }

    if(i != sizeof(s)/sizeof(s[0]) - 1)
    {
        return -1;
    }
    s[sizeof(s)/sizeof(s[0]) - 1] = str.substr(pos);

    this->srv_ip = std::move(s[0]);
    this->user   = std::move(s[1]);
    this->passwd = std::move(s[2]);

    return 0;
}

int rules_info_t::parse_json_file(const char *conf_path)
{
    int result = -1;

    std::ifstream json_stream(conf_path);

    std::ostringstream json_sin;
    
    json_sin << json_stream.rdbuf();

    std::string content = json_sin.str();

    Json::CharReaderBuilder builder;
    
    Json::CharReader *p_reader(builder.newCharReader());

    Json::Value json_data;

    JSONCPP_STRING err_str;
    bool parse_ret = p_reader->parse(content.c_str(), 
                                     content.c_str() + content.length(), 
                                     &json_data, &err_str);
    if (false == parse_ret) 
    {
        debug_print("json_reader.parse error %s\n", err_str.c_str());
        goto parse_end;
    }
    if (false == json_data["output_path"].isNull()) 
    {
        this->output_path = json_data["output_path"].asString();
    }
    if (false == json_data["ftp_login"].isNull()) 
    {
        decode_ftp_login(json_data["ftp_login"].asString());
    }
    if (true == json_data["max_size"].isInt()) 
    {
        this->max_file_size = json_data["max_size"].asInt();
    }
    this->rules_count = (size_t)json_data["rules"].size();

    this->rules_array = new object_rules_t[this->rules_count];

    if (0 != this->rules_count && NULL == this->rules_array)
    {
        DEBUG_PRINT("allocation rules_array error\n");
        goto parse_end;
    }
    result = assignment_rules(json_data);
    
parse_end:

    delete p_reader;
    p_reader = NULL;

    json_stream.close();
    json_stream.clear();

    return result;
}

void rules_info_t::parse_list_file(const char *white_list_path)
{
    std::fstream content(white_list_path);  
    
    std::string line;  

    while (getline(content, line))
    {   
        if (line.length() > 0)
            this->white_list.push_back(line);
    }  
}

rules_info_t::rules_info_t(const char *list_path, const char *conf_path) :
    rules_info_t()
{
    rules_load(list_path, conf_path);
}

int rules_info_t::rules_load(const char *list_path, const char *conf_path)
{
    debug_print("white_list_path:[%s], rule_path:[%s]\n", list_path, conf_path);
    if (list_path != NULL && access(list_path, F_OK) == 0)
    {
        this->parse_list_file(list_path);
    }
    if (conf_path != NULL && access(conf_path, F_OK) == 0)
    {
        this->parse_json_file(conf_path);
    }
    else
    {
        /* if no rules loaded, exit at main_module */
        return -1;
    }

    this->dump_rules_info();
    return 0;
}

object_rules_t *rules_info_t::rules_match(object_type_t type, void *p_info)
{
    object_rules_t *p_ret = NULL;
    
    if (NULL == p_info)
    {
        return NULL;
    }

    for(size_t i = 0; i < this->rules_count; i++)
    {
        object_rules_t *p_rule = &this->rules_array[i];
        if(p_rule == NULL)
        {
            continue;
        }
        if(p_rule->type != type)
        {
            continue;
        }
        for(std::vector<BaseObject *>::iterator itr = p_rule->objects.begin();
            itr != p_rule->objects.end(); ++itr)
        {
            if((*itr)->match(p_info))
            {
                p_ret = p_rule;
                break;
            }
        }
        if(p_ret)
        {
            break;
        }
    }

    return p_ret;
}

std::string rules_info_t::get_log_path()
{
    return this->output_path;
}

int rules_info_t::get_ftp_login(std::string &ip, std::string &user, std::string &passwd)
{
    if(this->srv_ip.size() == 0
        || this->user.size() == 0
        || this->passwd.size() == 0)
    {
        return -1;
    }
    ip = this->srv_ip;
    user = this->user;
    passwd = this->passwd;
    return 0;
}

time_t rules_info_t::get_finish_time()
{
    return this->finish_time;
}
