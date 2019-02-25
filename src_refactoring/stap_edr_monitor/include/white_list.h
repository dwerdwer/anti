#ifndef __WHITE_LIST_H__
#define __WHITE_LIST_H__
#include <inttypes.h>
#include <time.h>
#include <map>
#include <string>

class white_list{
public:
    static white_list *get_instance();

    void set_config_file(const char *file, const char *proc, 
        const char *modu, const char *host);
    void load_config();
    bool is_file_in_white_list(const char *file_name);
    bool is_proc_in_white_list(const char *proc_name);
    bool is_modu_in_white_list(const char *modu_name);
    bool is_host_in_white_list(const char *host_name);
private:
    white_list();

    std::string config_file;
    std::string config_proc;
    std::string config_modu;
    std::string config_host;

    bool is_str_in_map(std::map<uint32_t, std::string> &map, const char *needle);

    std::map<uint32_t, std::string> file;
    time_t file_last_load;
    std::map<uint32_t, std::string> proc;
    time_t proc_last_load;
    std::map<uint32_t, std::string> modu;
    time_t modu_last_load;
    std::map<uint32_t, std::string> host;
    time_t host_last_load;
};


#endif /* __WHITE_LIST_H__ */
