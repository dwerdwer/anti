#include <unistd.h>
#include <pthread.h>
#include <sys/inotify.h>
#include <sys/time.h> // gettimeofday

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <map>
#include <queue>

#include "module_file_monitor_interface.h"
#include "file_system_monitor.h"
#include "debug_print.h"

#define PUBLIC_LIB __attribute__ ((visibility ("default")))

#define MAX_OPTSTR_LEN 256


#include "stap_file_monitor.h"
#define STAP_DEFAULT_EXE "SYSTEMTAP_STAPIO=./stapio ./staprun -R"
#define STAP_DEFAULT_STP "../lib/edr_stap_file_monitor.ko"
#define FILE_ACTION_TIME_OUT 3

#define DEFAULT_TTL_SPAN 2 // cache time to live unit second

typedef std::pair<time_t, std::string> ttl_member_t;

class CriticalSection
{
public:
    CriticalSection(pthread_spinlock_t *lock) : lock(lock)
    {
        if(lock)
        {
            pthread_spin_init(this->lock, 0);
        }
    }
    void release()
    {
        if(lock)
        {
            pthread_spin_destroy(this->lock);
            lock = NULL;
        }
    }
    ~CriticalSection()
    {
        if(lock)
        {
            pthread_spin_destroy(this->lock);
            lock = NULL;
        }
    }
private:
    pthread_spinlock_t *lock;
};

struct file_monitor
{
public:
    file_monitor(void)
    {
        pthread_spin_init(&this->stap_spinlock, 0);
    }

    ~file_monitor(void)
    {
        pthread_spin_destroy(&this->stap_spinlock);
    }
    // void enter_lock(void);

    // void leave_lock(void);

    void *p_data;
    event_callback_t event_cb;

    uint32_t option_count;
    char **monitor_lists;

    file_system_monitor *p_monitor;

    std::multimap<std::string, file_event_t *> monitor_cache;

    stap_monitor *stap_handle;
    std::multimap<std::string, stap_file_action *> stap_cache;
    std::queue<ttl_member_t> stap_ttl; // time to live

    std::vector<std::string> monitor_paths;
    pthread_rwlock_t path_lock;

    bool running;
    pthread_spinlock_t stap_spinlock;
};

// void file_monitor::enter_lock(void)
// {
//     pthread_spin_lock(&this->stap_spinlock);
// }

// void file_monitor::leave_lock(void)
// {
//     pthread_spin_unlock(&this->stap_spinlock);
// }

/* a library for string */
static bool str_endswith(const char *str, size_t str_len, const char *sub, size_t sub_len)
{
    if (str_len >= sub_len && sub_len) {
        return (strncmp(str+str_len-sub_len, sub, sub_len) == 0);
    }
    return false;
}

static bool str_endswith(std::string &str, const char *sub)
{
    return str_endswith(str.data(), str.size(), sub, strlen(sub));
}

static std::string str_replace(std::string str, const char *src, const char *dst)
{
    size_t pos = 0;
    size_t src_len = strlen(src);
    while (true) {
        pos = str.find(src, pos);
        if (pos == std::string::npos)
            break;
        str.replace(pos, src_len, dst);
    }
    return str;
}

/* a library for Linux-path */
#ifdef __linux__
static std::string path_format(std::string path)
{
    if(path.size() == 0)
        return path;
    path.erase(0, path.find_first_of("/"));
    path = str_replace(path, "//", "/");
    path = str_replace(path, "/./", "/");
    // path = str_replace(path, "/../", "/");
    size_t pos = 0;
    static std::string recurse_flag = "/../";
    while (true) {
        pos = path.find(recurse_flag, pos);
        if (pos == std::string::npos)
            break;
        size_t prev_pos = path.find_last_of("/", pos-1);
        // WARNING: this an invalid path
        if (prev_pos == std::string::npos)
            break;
        path.erase(prev_pos+1, (pos+recurse_flag.size()) -(prev_pos+1));
    }
    if (str_endswith(path, "/")) {
        path.erase(path.size()-1);
    }
    if (str_endswith(path, "/.")) {
        path.erase(path.size()-2);
    }
    if (str_endswith(path, "/..")) {
        pos = path.find_last_of("/", path.size()-4);
        if (pos != std::string::npos) {
            path.erase(pos);
        }
    }
    return path;
}
#endif /*__linux__*/

/* cache library */
static file_event_t *file_event_copy(const file_event_t *src)
{
    file_event_t *dst = (file_event_t *)malloc(sizeof(file_event_t));
    assert(dst);

    memcpy(dst, src, sizeof(file_event_t));
    dst->p_dest_path = strdup(src->p_dest_path);
    assert(dst->p_dest_path);

    return dst;
}

static void file_event_free(file_event_t *event)
{
    if (event) {
        free((char *)event->p_dest_path);
        free(event);
    }
}

static stap_file_action *stap_file_action_copy(const stap_file_action *src)
{
    stap_file_action *dst = (stap_file_action *)malloc(sizeof(stap_file_action));
    assert(dst);

    memcpy(dst, src, sizeof(stap_file_action));
    dst->file_name = strdup(src->file_name);
    assert(dst->file_name);

    return dst;
}

static void stap_file_action_free(stap_file_action *pst)
{
    if (pst) {
        free((char *)pst->file_name);
        free(pst);
    }
}

/* monitor paths library */
#define STAP_CODE_DEBUG 1
#if STAP_CODE_DEBUG
#include <stdio.h>
static void paths_manager_debug(file_monitor *p_file_monitor)
{
    printf("-------------%s-------------\n", __func__);
    pthread_rwlock_rdlock(&p_file_monitor->path_lock);
    for (std::vector<std::string>::iterator itr=p_file_monitor->monitor_paths.begin();
            itr!=p_file_monitor->monitor_paths.end(); itr++) {
        printf("%s\n", (*itr).data());
    }
    pthread_rwlock_unlock(&p_file_monitor->path_lock);
    printf("-------------%s-------------\n", __func__);
}
#endif /* STAP_CODE_DEBUG */

static int paths_manager_insert(file_monitor *p_file_monitor, const char *path)
{
    bool is_exists = false;
    std::string format_path = path_format(path);
    pthread_rwlock_wrlock(&p_file_monitor->path_lock);
    for (std::vector<std::string>::iterator itr=p_file_monitor->monitor_paths.begin();
            itr!=p_file_monitor->monitor_paths.end(); itr++) {
        if (itr->compare(format_path) == 0) {
            is_exists = true;
        }
    }
    if (is_exists == false) {
        p_file_monitor->monitor_paths.push_back(format_path);
    }
    pthread_rwlock_unlock(&p_file_monitor->path_lock);
    paths_manager_debug(p_file_monitor);
    return is_exists ? 0 : 1;
}

static int paths_manager_erase(file_monitor *p_file_monitor, const char *path)
{
    bool is_erase= false;
    std::string format_path = path_format(path);
    pthread_rwlock_wrlock(&p_file_monitor->path_lock);
    for (std::vector<std::string>::iterator itr=p_file_monitor->monitor_paths.begin();
            itr!=p_file_monitor->monitor_paths.end(); ) {
        if (itr->compare(format_path) == 0) {
            itr = p_file_monitor->monitor_paths.erase(itr);
            is_erase = true;
        }
        else {
            itr++;
        }
    }
    pthread_rwlock_unlock(&p_file_monitor->path_lock);
#if STAP_CODE_DEBUG
    paths_manager_debug(p_file_monitor);
#endif /* STAP_CODE_DEBUG */
    return is_erase ? 1 : 0;
}

static bool paths_manager_in_monitor(file_monitor *p_file_monitor, const char *path)
{
    bool ret = false;
    std::string format_path = path_format(path);
    pthread_rwlock_rdlock(&p_file_monitor->path_lock);
    for (std::vector<std::string>::iterator itr=p_file_monitor->monitor_paths.begin();
            itr!=p_file_monitor->monitor_paths.end(); itr++) {
        std::string &s = *itr;
        if (s.size() > format_path.size())
            continue;
        if (format_path.compare(0, s.size(), s) == 0) {
            ret = true;
            break;
        }
    }
    pthread_rwlock_unlock(&p_file_monitor->path_lock);
    return ret;
}

PUBLIC_LIB void free_file_event(file_event_t *p_event)
{
    if(p_event->p_dest_path)
        free((char *)p_event->p_dest_path);
    free(p_event);
}

static void stap_file_action_cb_impl(const void *action, void *p_data)
{
    stap_file_action *pst = (stap_file_action *)action;
    file_monitor *p_file_monitor = (file_monitor *)p_data;
    CriticalSection lock(&p_file_monitor->stap_spinlock);
    // debug_print("stap check in   --> %s\n", pst->file_name);

    /* if this action is not in monitor path
     * ignore it
     * */
    if(pst->file_name == NULL) {
        return ;
    }
    std::string key = path_format(pst->file_name);
    if (!paths_manager_in_monitor(p_file_monitor, key.data())) {
        return ;
    }

    // bool send_flag = false;
    debug_print("stap in monitor --> %s\n", key.data());

    /* if a file_action exists
     *   @ set pid to file_action
     *   @ notify
     *   @ erase cache
     * */
    while (true) {
        std::multimap<std::string, file_event_t *>::iterator itr=p_file_monitor->monitor_cache.find(key);
        if (itr == p_file_monitor->monitor_cache.end()) {
            // std::cout << __func__ << " monitor cache load end" << std::endl;
            break;
        }
        if (pst->time < itr->second->time + FILE_ACTION_TIME_OUT) {
            itr->second->pid = pst->pid;
            p_file_monitor->event_cb(1, itr->second, p_file_monitor->p_data);
            // send_flag = true;
        }
        /* NOTE: itr->second is unusable after event_cb */

        // if (true == send_flag) {
        //     free(itr->second); // keep file path
        // }
        // else {
        //     file_event_free(itr->second);
        // }
        p_file_monitor->monitor_cache.erase(itr);
        // send_flag = false;
    }
    //p_file_monitor->enter_lock();
    // whatever, store stap_file_action
    p_file_monitor->stap_cache.insert(make_pair(key, stap_file_action_copy(pst)));

    //time to live
    struct timeval tv;
    gettimeofday(&tv, NULL);

    time_t current_time = tv.tv_sec;
    p_file_monitor->stap_ttl.push(make_pair(current_time, key));

    ttl_member_t front_member = p_file_monitor->stap_ttl.front();

    std::multimap<std::string, stap_file_action *>::iterator sit;

    while (front_member.first + DEFAULT_TTL_SPAN <= current_time)
    {
        while (true)
        {
            sit = p_file_monitor->stap_cache.find(front_member.second);
            if (sit == p_file_monitor->stap_cache.end()) {
                break;
            }
            stap_file_action_free(sit->second);
            p_file_monitor->stap_cache.erase(sit);
        }
        p_file_monitor->stap_ttl.pop();
        front_member = p_file_monitor->stap_ttl.front();
    }
    //p_file_monitor->leave_lock();
}

static void on_inotify_monitor_cb_impl(int event_type, int argc, const char *args[], void *p_data)
{
    if (NULL == p_data) {
        debug_print("p_data is empty\n");
        return;
    }
    file_monitor *p_file_monitor = (file_monitor*)p_data;
    if (p_file_monitor->event_cb == NULL) {
        debug_print("event_cb is empty\n");
        return;
    }
    // CriticalSection lock(&p_file_monitor->stap_spinlock);

    file_event_t *event = (file_event_t *)malloc(sizeof(file_event_t));
    memset(event, 0, sizeof(file_event_t));

    event->time = time(NULL);

    switch(event_type)
    {
    case (IN_CREATE | IN_ISDIR):
            event->type = EVENT_NEW_DIR;
            event->p_dest_path = args[0];
            //event->p_src_path = "";
        break;
    case (IN_DELETE | IN_ISDIR):
            event->type = EVENT_REMOVE_DIR;
            event->p_dest_path = args[0];
            //event->p_src_path = "";
        break;
    case IN_MOVED_TO:
            //event->type = EVENT_MOVE_FILE_OR_DIRECTORY;
            //event->p_dest_path = args[0];
            //event->p_src_path = args[1];
        break;
    case IN_CREATE:
            event->type = EVENT_NEW_FILE;
            event->p_dest_path = args[0];
            //event->p_src_path = "";
        break;
    case IN_DELETE:
            event->type = EVENT_REMOVE_FILE;
            event->p_dest_path = args[0];
            //event->p_src_path = "";
        break;
    case IN_MODIFY:
            event->type = EVENT_MODIFY_FILE;
            event->p_dest_path = args[0];
            //event->p_src_path = "";
        break;
    default:
        break;
    }
    if(event->p_dest_path == NULL) {
        return ;
    }
    event->p_dest_path = strdup(event->p_dest_path);

    /* Relation PID:
     * if a stap_action exists,
     *    @ set stap.pid -> event->pid
     *    @ cb
     *    @ erase stap_cache
     * */
    std::string path = path_format(event->p_dest_path);
    debug_print("event in monitor --> %s\n", path.data());
    CriticalSection lock(&p_file_monitor->stap_spinlock);

    //p_file_monitor->enter_lock();
    while (true) {
        std::multimap<std::string, stap_file_action *>::iterator itr=p_file_monitor->stap_cache.find(path);
        if (itr == p_file_monitor->stap_cache.end()) {
            break;
        }
        if (itr->second && itr->second->pid && event) {
            if (itr->second->time + FILE_ACTION_TIME_OUT > event->time) {
                event->pid = itr->second->pid;
                p_file_monitor->event_cb(1, event, p_file_monitor->p_data);
                /* NOTE: event is unusable after event_cb */
                event = NULL;
            }
        }
        stap_file_action_free(itr->second);
        p_file_monitor->stap_cache.erase(itr);
    }
    //p_file_monitor->leave_lock();
    // if no stap_action exists, store it
    if (event) {
        p_file_monitor->monitor_cache.insert(std::pair<std::string, file_event_t *> (path, event));

        time_t current_time = event->time;
        std::multimap<std::string, file_event_t *>::iterator mit = p_file_monitor->monitor_cache.begin();
        /* Don't worry about performance, only valid before the systemtap runs */
        while (mit != p_file_monitor->monitor_cache.end())
        {
            if (mit->second->time + DEFAULT_TTL_SPAN < current_time) {
                file_event_free(mit->second);
                p_file_monitor->monitor_cache.erase(mit++);
            } else {
                mit++;
            }
        }
    }
}

PUBLIC_LIB file_monitor_t* init_file_monitor(event_callback_t callback, void *p_user_data,
            uint32_t option_count, const char *(*pp_options)[2])
{
    if (NULL == pp_options || NULL == callback)
        return NULL;

    file_monitor *p_file_monitor = new file_monitor;
    assert(p_file_monitor);

    p_file_monitor->p_monitor = new file_system_monitor;
    assert(p_file_monitor->p_monitor);
    p_file_monitor->p_data = p_user_data;
    p_file_monitor->event_cb = callback;
    p_file_monitor->running = false;

    p_file_monitor->option_count = option_count;
    p_file_monitor->monitor_lists = (char**)calloc(option_count, sizeof(char*));

    size_t current_len = 0;

    for (uint32_t i = 0; i < option_count; i++)
    {
        p_file_monitor->monitor_lists[i] = (char*)calloc(MAX_OPTSTR_LEN, sizeof(char));

        if (NULL == pp_options[i][0]
           || NULL == pp_options[i][1] || '/' != pp_options[i][1][0])
        {
            /* free conut */
            p_file_monitor->option_count = i + 1;
            goto ErrEnd;
        }
        current_len = strlen(pp_options[i][1]);

        if ((strcmp(pp_options[i][0], "monitor_directory") == 0) && current_len < MAX_OPTSTR_LEN)
        {
            if ( '/' == pp_options[i][1][current_len - 1] )
                strncpy(p_file_monitor->monitor_lists[i], pp_options[i][1], current_len - 1);
            else
                strncpy(p_file_monitor->monitor_lists[i], pp_options[i][1], current_len);
        }
    }
    for (uint32_t i = 0; i < option_count; i++) {
        p_file_monitor->monitor_paths.push_back(path_format(p_file_monitor->monitor_lists[i]));
    }

    stap_interface_t itf;
    itf.p_callback = stap_file_action_cb_impl;
    itf.p_user_data = p_file_monitor;

    p_file_monitor->stap_handle = init_stap_monitor(STAP_DEFAULT_EXE, STAP_DEFAULT_STP, &itf);
    if(!p_file_monitor->stap_handle)
    {
        destroy_file_monitor(p_file_monitor);
        return NULL;
    }

    debug_print("%s success\n", __func__);
    return p_file_monitor;
ErrEnd:
    debug_print("%s fail\n", __func__);
    destroy_file_monitor(p_file_monitor);
    return NULL;
}

/* If success function is blocked */
PUBLIC_LIB bool run_file_monitor(file_monitor_t *p_file_monitor)
{
    bool result = false;
    if(NULL == p_file_monitor || NULL == p_file_monitor->p_monitor)
        return result;

    result = true;
    if(p_file_monitor->running == true)
    {
        return result;
    }
    p_file_monitor->running = true;
    while(p_file_monitor->running)
    {
        result = p_file_monitor->p_monitor->begin_monitor(p_file_monitor->option_count,
                            p_file_monitor->monitor_lists, on_inotify_monitor_cb_impl,
                            (void*)p_file_monitor, p_file_monitor->stap_handle);
        fprintf(stderr, "%s monitor endwith result is true:%d\n", __func__, result);
        if(!result && p_file_monitor->running)
        {
            fin_stap_monitor(p_file_monitor->stap_handle);

            stap_interface_t itf;
            itf.p_callback = stap_file_action_cb_impl;
            itf.p_user_data = p_file_monitor;

            p_file_monitor->stap_handle = init_stap_monitor(STAP_DEFAULT_EXE, STAP_DEFAULT_STP, &itf);
            if(!p_file_monitor->stap_handle)
            {
                destroy_file_monitor(p_file_monitor);
                return NULL;
            }
            fprintf(stderr, "%s restart stapmonitor cause stapmonitor errors\n", __func__);
        }

    }
    return result;
}

PUBLIC_LIB bool add_dir_to_watcher(file_monitor_t *p_file_monitor, const char *p_directory)
{
    if (p_file_monitor && p_file_monitor->p_monitor && p_directory)
    {
        debug_print(">>> add directory to monitor!\n");
        if (0 == p_file_monitor->p_monitor->add_storage(p_directory, strlen(p_directory))) {
            paths_manager_insert(p_file_monitor, p_directory);
            debug_print(">>> add directory to monitor done!\n");
            return true;
        }
    }
    return false;
}

PUBLIC_LIB bool remove_dir_from_watcher(file_monitor_t *p_file_monitor,const char *p_directory)
{
    if (p_file_monitor && p_file_monitor->p_monitor && p_directory)
    {
        debug_print(">>> remove directory from monitor!\n");
        if (0 == p_file_monitor->p_monitor->remove_storage(p_directory, strlen(p_directory))) {
            paths_manager_erase(p_file_monitor, p_directory);
            debug_print(">>> remove directory from monitor done!\n");
            return true;
        }
    }
    return false;
}

PUBLIC_LIB bool add_to_white_list(file_monitor_t * p_file_monitor,const char * p_path)
{
    if (p_file_monitor && p_file_monitor->p_monitor && p_path)
    {
        debug_print(">>> add file or directory to white list!\n");
        if (0 == p_file_monitor->p_monitor->add_to_whitelist(p_path, strlen(p_path))) {
            debug_print(">>> add file or directory to white list done!\n");
            return true;
        }
    }
    return false;
}

PUBLIC_LIB bool remove_from_white_list(file_monitor_t *p_file_monitor, const char *p_path)
{
    if (p_file_monitor && p_file_monitor->p_monitor && p_path)
    {
        debug_print(">>> remove file or directory from white list!\n");
        if ( 0 == p_file_monitor->p_monitor->remove_from_whitelist(p_path, strlen(p_path))) {
            debug_print(">>> remove file or directory from white list done!\n");
            return true;
        }
    }
    return false;
}

/* fuck */
PUBLIC_LIB bool change_flag(file_monitor_t *p_file_monitor)
{
    if(p_file_monitor && p_file_monitor->p_monitor)
    {
        p_file_monitor->p_monitor->change_flag();
    }
    return true;
}

PUBLIC_LIB void stop_file_monitor(file_monitor_t *p_file_monitor)
{
    if (p_file_monitor)
    {
		p_file_monitor->running = false;
        if (p_file_monitor->p_monitor)
        {
            p_file_monitor->p_monitor->end_monitor();
        }
    }
}

PUBLIC_LIB void destroy_file_monitor(file_monitor_t *p_file_monitor)
{

    if (p_file_monitor)
    {
        stop_file_monitor(p_file_monitor);

        if (p_file_monitor->p_monitor)
        {
            delete p_file_monitor->p_monitor;
            p_file_monitor->p_monitor = NULL;
        }

        if (p_file_monitor->stap_handle)
        {
            fin_stap_monitor(p_file_monitor->stap_handle);
            p_file_monitor->stap_handle = NULL;
        }

        if (p_file_monitor->monitor_lists) {

            for (uint32_t i = 0; i < p_file_monitor->option_count; i++)
                free(p_file_monitor->monitor_lists[i]);

            free(p_file_monitor->monitor_lists);
            p_file_monitor->monitor_lists = NULL;
        }

        for (std::multimap<std::string, file_event_t *>::iterator itr=p_file_monitor->monitor_cache.begin();
                itr!=p_file_monitor->monitor_cache.end(); ) {
            file_event_free(itr->second);
            p_file_monitor->monitor_cache.erase(itr++);
        }

        //p_file_monitor->enter_lock();
        for (std::multimap<std::string, stap_file_action *>::iterator itr=p_file_monitor->stap_cache.begin();
                itr!=p_file_monitor->stap_cache.end(); ) {
            stap_file_action_free(itr->second);
            p_file_monitor->stap_cache.erase(itr++);
        }
        //p_file_monitor->leave_lock();
        delete p_file_monitor;
    }
}
