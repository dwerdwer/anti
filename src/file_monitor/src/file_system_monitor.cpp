
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <dirent.h>
#include <queue>
#include <sys/stat.h>
#include <sys/inotify.h>

#include "debug_print.h"
#include "file_system_monitor.h"

using namespace std;

vector <string> view_dir(const char *base_path) 
{
    vector <string> ret_vec;

    if (NULL == base_path || access(base_path, 0) == -1) 
        return ret_vec;

    string current_dir;
    if( '/' == base_path[strlen(base_path) - 1] ) {
        current_dir.append(base_path, strlen(base_path) - 1);
    }
    else {
        current_dir.append(base_path, strlen(base_path));
    }
    ret_vec.push_back(current_dir);

    queue <string> dir_queue;
    dir_queue.push(current_dir);

    struct stat current_stat;
    dirent *p_dirent = NULL;

    DIR *p_dir = NULL;

    while (!dir_queue.empty()) 
    {
        current_dir = dir_queue.front();
        lstat(current_dir.c_str(), &current_stat);

        if ( S_ISDIR(current_stat.st_mode) && 
             NULL != ( p_dir = opendir(current_dir.c_str()) ) ) 
        {
            while ((p_dirent = readdir(p_dir)))
            {
                if ( (p_dirent->d_type == DT_DIR) 
                     && (strcmp(p_dirent->d_name, ".") != 0)
                     && (strcmp(p_dirent->d_name, "..") != 0) ) 
                {
                    ret_vec.push_back(current_dir + "/" + p_dirent->d_name);
                    dir_queue.push(current_dir + "/" + p_dirent->d_name);
                }
            }
            closedir(p_dir);
            p_dir = NULL;
        }
        dir_queue.pop();
    }
    return ret_vec;
}

file_system_monitor::file_system_monitor() :
    flag(1), file_descriptor(-1), notify_cb(NULL), p_user_data(NULL), thread_id(0)
{
}

file_system_monitor::~file_system_monitor()
{
}

int file_system_monitor::add_storage(const char *dir_path, int len) 
{
    int result = -1;
    if (NULL == dir_path)
        return result;

    vector<string> tmp_vec = view_dir(dir_path);
    if (tmp_vec.size() > 0)
    {
        int wd = -1;
        int monitor_flag = IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO | IN_ISDIR;

        vector<string>::iterator it = tmp_vec.begin();

        for (; it != tmp_vec.end(); it++)
        {
            wd = inotify_add_watch(this->file_descriptor, (*it).c_str(), monitor_flag);
            if (wd >= 0)
            {
                this->m_mutex.enter();
                this->watch_vec.push_back(node_info_t (*it, wd));
                this->m_mutex.leave();
            }
        }
        result = 0;
    }
    return result;
}

static int start_with(const char *dir_path, const char *p_tag) 
{
    if (dir_path && p_tag && strlen(dir_path) > 0 && strlen(p_tag) > 0) 
    {
        if (strstr(dir_path, p_tag) == dir_path)
            return 1;
    }
    return 0;
}

int file_system_monitor::remove_storage(const char *dir_path, int len) 
{
    int result = -1;
    if (dir_path)
    {
        this->m_mutex.enter();
        
        vector<node_info_t>::iterator it = watch_vec.begin();
       
        while (it != watch_vec.end()) 
        {
            if (start_with((*it).path.c_str(), dir_path)) 
            {
                inotify_rm_watch(this->file_descriptor, (*it).wd);
                close((*it).wd);
                debug_print(">>> file_system_monitor:: remove director %s\n", (*it).path.c_str());
                it = this->watch_vec.erase(it);
                result = 0;
            } else {
                it++;
            }
        }
        this->m_mutex.leave();
    }
    return result;
}

int file_system_monitor::add_to_whitelist(const char *dir_path, int len)
{
    int result = -1;
    if (dir_path && '/' == dir_path[0] && len > 0)
    {
        this->m_mutex.enter();
        this->white_list_vec.push_back(dir_path);
        this->m_mutex.leave();
        result = 0;
    }
    return result;
}

int file_system_monitor::remove_from_whitelist(const char *dir_path, int len)
{ 
    int result = -1;
    if (dir_path && strlen(dir_path) > 0)
    {
        debug_print(">>> + file_system_monitor:: remove from white list %s\n",dir_path);
        this->m_mutex.enter();
        
        vector<string>::iterator it = this->white_list_vec.begin();
        while (it != this->white_list_vec.end()) 
        {
            if (strcmp(dir_path, (*it).c_str()) == 0){
                it = white_list_vec.erase(it);
                result = 0;
            }else {
                it++;
            }
        }
        this->m_mutex.leave();
    }
    return result;
}

void file_system_monitor::reset_whitelist()
{
    this->m_mutex.enter();
    this->white_list_vec.clear();
    this->m_mutex.leave();
}

/* return false not found */
static bool in_whitelist(vector<string> &white_list_vec, 
                         mutex_t &m_mutex, const char *dir_path)
{
    bool result = false;
    if (NULL == dir_path)
        return result;
    
    m_mutex.enter();
    vector<string>::iterator it = white_list_vec.begin();

    for (; it != white_list_vec.end(); it++)
    {
        if (start_with(dir_path, (*it).c_str())) {
            result = true;
            break;
        }
    }
    m_mutex.leave();
    return result;
}

void* start_monitor(void *thread_param)
{
    file_system_monitor *p_monitor = (file_system_monitor*)thread_param;

    debug_print(">>> file_system_monitor::Start thread ......\n");

    if (p_monitor->dir_vec.empty() || NULL == p_monitor->notify_cb)
        return NULL;

    if( -1 == (p_monitor->file_descriptor = inotify_init()))
        return NULL;

    int wd = -1;
    int monitor_flag = IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO | IN_ISDIR;

    p_monitor->m_mutex.enter();
    vector<string>::iterator dir_it = p_monitor->dir_vec.begin();

    for (; dir_it != p_monitor->dir_vec.end(); dir_it++)
    {
        wd = inotify_add_watch(p_monitor->file_descriptor, (*dir_it).c_str(), monitor_flag);

        if (-1 != wd)
        {
            p_monitor->watch_vec.push_back(node_info_t (*dir_it, wd));
        }
    }
    p_monitor->m_mutex.leave();

    fd_set monitor_tmp;
    fd_set monitor_set;
    FD_ZERO(&monitor_set);
    FD_SET(p_monitor->file_descriptor, &monitor_set);

    struct timeval time_out;
    time_out.tv_sec = 1;
    time_out.tv_usec = 0;

    string dir_path = "";
    int length = 0;

    int read_index = 0;
    char arr_buffer[PATH_MAX] = { 0 };
    
    struct inotify_event *p_event = NULL;
    
    string move_from = "";
   
    while(p_monitor->flag)
    {
        monitor_tmp = monitor_set;
        
        if (select(p_monitor->file_descriptor + 1, &monitor_tmp, NULL, NULL, &time_out) > 0)
        {
            read_index = 0;
            memset(arr_buffer, 0, PATH_MAX);
            length = read(p_monitor->file_descriptor, arr_buffer, sizeof(arr_buffer) - 1);
            
            while( length > 0 )
            {
                p_event = (struct inotify_event*)&arr_buffer[read_index];
                dir_path = p_monitor->get_dir(p_event->wd);
                dir_path.append("/");

                if (p_event->len > 0)
                    dir_path.append(p_event->name);

                if (p_event->mask == (IN_CREATE | IN_ISDIR))
                {
                    wd = inotify_add_watch(p_monitor->file_descriptor, dir_path.c_str(), monitor_flag);
                    if (wd >= 0)
                    {
                        p_monitor->m_mutex.enter();
                                                        
                        p_monitor->watch_vec.push_back(node_info_t (dir_path, wd));
                        p_monitor->m_mutex.leave();

                        if (in_whitelist(p_monitor->white_list_vec, 
                                         p_monitor->m_mutex, dir_path.c_str()) == false)
                        {
                            const char *args[] = {dir_path.c_str()};
                            p_monitor->notify_cb(p_event->mask, 1, args, p_monitor->p_user_data);
                        }
                    }
                }
                else if (p_event->mask == (IN_DELETE | IN_ISDIR))
                {
                    wd = p_monitor->get_wd(dir_path);
                    if (wd != -1)
                    {
                        inotify_rm_watch(p_monitor->file_descriptor, wd);
                        p_monitor->remove_dir(wd);
                    }
                    if (in_whitelist(p_monitor->white_list_vec, 
                                     p_monitor->m_mutex, dir_path.c_str()) == false)
                    {
                        const char *args[] = {dir_path.c_str()};
                        p_monitor->notify_cb(p_event->mask, 1, args, p_monitor->p_user_data);
                    } 
                }  
                else if (p_event->mask & IN_MOVED_FROM)
                {
                    move_from = dir_path;
                }
                else if (p_event->mask & IN_MOVED_TO)
                {
                    if (in_whitelist(p_monitor->white_list_vec, 
                                     p_monitor->m_mutex, dir_path.c_str()) == false)
                    {
                        const char *args[] = {dir_path.c_str(), move_from.c_str()};
                        p_monitor->notify_cb(p_event->mask, 2, args, p_monitor->p_user_data);
                    }
                }
                else {
                    if (in_whitelist(p_monitor->white_list_vec, 
                                     p_monitor->m_mutex, dir_path.c_str()) == false)
                    {
                        const char *args[] = {dir_path.c_str()};
                        p_monitor->notify_cb(p_event->mask, 1, args, p_monitor->p_user_data);
                    }
                }
                read_index = read_index + sizeof(struct inotify_event) + p_event->len;
                length = length - sizeof(struct inotify_event) - p_event->len;
            }
        }else{
            usleep(500000);
        }
    }
   
    if (p_monitor->file_descriptor != -1)
    {
        p_monitor->m_mutex.enter();
        vector<node_info_t>::iterator it = p_monitor->watch_vec.begin();
       
        for (; it != p_monitor->watch_vec.end(); it++) 
            close((*it).wd);
        
        p_monitor->watch_vec.clear();
        
        close(p_monitor->file_descriptor);
        p_monitor->m_mutex.leave();
    }
    return NULL;
}

bool file_system_monitor::begin_monitor(int argc,char *args[], event_notify_t notify_cb,void * p_user_data) 
{
    this->notify_cb = notify_cb;
    this->p_user_data = p_user_data;

    for (int i = 0; i < argc; i++)
    {
        vector<string> tmp_vec = view_dir(args[i]);

        for (size_t j = 0; j < tmp_vec.size(); j++) {
            dir_vec.push_back(tmp_vec.at(j));
        }
    }
    if (pthread_create(&thread_id, NULL, start_monitor, this) == 0)
    {
        return true;
    }
    return false;
}

void file_system_monitor::end_monitor() 
{
    this->flag = 0;
    if (this->thread_id != 0){
        pthread_cancel(this->thread_id);
    }
}

string file_system_monitor::get_dir(int wd) 
{
    string result;
    this->m_mutex.enter();

    vector<node_info_t>::iterator it = this->watch_vec.begin();

    while(it != this->watch_vec.end())
    {
        if((*it).wd == wd) {
            // it = this->watch_vec.erase(it);
            result = (*it).path;
            break;
        }
        else {
            ++it;
        }
    }
    this->m_mutex.leave();

    return result;
}

int file_system_monitor::get_wd(string path)
{
    int result = -1;
    this->m_mutex.enter();

    vector<node_info_t>::iterator it = this->watch_vec.begin();

    while(it != this->watch_vec.end())
    {
        if((*it).path.compare(path) == 0) {
            it = this->watch_vec.erase(it);
            result = (*it).wd;
            break;
        }
        else {
            ++it;
        }
    }
    this->m_mutex.leave();

    return result;
}

int file_system_monitor::remove_dir(int wd) 
{
    int result = -1;
    this->m_mutex.enter();

    vector<node_info_t>::iterator it = this->watch_vec.begin();
    while(it != this->watch_vec.end())
    {
        if((*it).wd == wd) {
            it = this->watch_vec.erase(it);
            result = 0;
            break;
        }
        else {
            ++it;
        }
    }
    this->m_mutex.leave();
    return result;
}


