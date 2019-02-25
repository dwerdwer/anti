
#ifndef FILE_SYSTEM_MONITOR_H
#define FILE_SYSTEM_MONITOR_H

#include <sys/inotify.h>
#include <vector>
#include <string>
#include "user_mutex.h"

typedef void (*event_notify_t)(int event_type, int argc, const char * args[], void *p_user_data);

struct node_info_t
{
    node_info_t(std::string s, int des) :
        path(s), wd(des) {}
    std::string path;
    int wd; // watch descriptor
};

class file_system_monitor
{
public:
    file_system_monitor();

    ~file_system_monitor();

    bool begin_monitor(int argc, char * args[], event_notify_t notify_cb, void *p_user_data);
    
    int add_storage(const char *dir_path, int len); 

    int remove_storage(const char *dir_path, int len);
   
    int add_to_whitelist(const char *dir_path, int len);

    int remove_from_whitelist(const char *dir_path, int len); 

    void reset_whitelist();

    void end_monitor();

    friend void* start_monitor(void *thread_param); 

private:
    std::vector<node_info_t> watch_vec;
    std::vector<std::string> dir_vec;
    std::vector<std::string> white_list_vec;
    
    int flag;
    int file_descriptor;
   
    event_notify_t notify_cb;
    void * p_user_data;
   
    mutex_t m_mutex;
    pthread_t thread_id;

    std::string get_dir(int wd);

    int get_wd(std::string dir_path);

    int remove_dir(int wd);
};

#endif
