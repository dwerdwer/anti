#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <dirent.h>
#include <queue>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <fcntl.h>
#include <unistd.h>

#include "debug_print.h"
#include "util-string.h"
#include "file_system_monitor.h"

using namespace std;

#define FGETS_MAX_SIZE 10240

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
    flag(1), ifd(-1), notify_cb(NULL), p_user_data(NULL){  }

file_system_monitor::~file_system_monitor(){  }

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
            wd = inotify_add_watch(this->ifd, (*it).c_str(), monitor_flag);
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
                inotify_rm_watch(this->ifd, (*it).wd);
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

struct kv_p_2_i file_action_kvs[] = {
    {"open",    STAP_FILE_ACTION_OPEN},
    {"unlink",  STAP_FILE_ACTION_UNLINK},
    {"rename",  STAP_FILE_ACTION_RENAME},
    {"chmod",   STAP_FILE_ACTION_CHMOD},
    {"chown",   STAP_FILE_ACTION_CHOWN},
    {"mkdir",   STAP_FILE_ACTION_MKDIR},
    {"rmdir",   STAP_FILE_ACTION_RMDIR}
};

static const char *parse_file_file_name(const char *file, const char *pwd)
{
    /* file, pwd is arround by " ", format like this
     * "/home/haha.txt"   "/home/john"  "a.c"
     * */
    size_t file_len = strlen(file);
    if(file_len < 3)
        return NULL;
    if(file[0] != '\"' || file[file_len-1] != '\"')
        return NULL;

    // if file is an absolute path, return it immediately
    if(file[1] == '/') {
        // ignore the first and the last char "
        return strndup(file+1, file_len-2);
    }

    // this file is an relative path, piece the "pwd" and the "file" together

    size_t pwd_len = strlen(pwd);
    // if pwd is unvalid, just only return file
    if( (file_len < 3) ||
        (pwd[0] != '\"' || pwd[pwd_len-1] != '\"') ||
        (pwd[1] != '/')
      ) {
        return strndup(file+1, file_len-2);
    }
    // piece "pwd" && "file" together
    char *ret = (char *)malloc((file_len-2)+(1)+(pwd_len-2)+1);

    strncpy(ret              , pwd+1,  pwd_len-2);
    strncpy(ret+(pwd_len-2)  , "/",    1);
    strncpy(ret+(pwd_len-2)+1, file+1, file_len-2);
    ret[(pwd_len-2)+1+(file_len-2)] = '\0';

    return ret;
}

static int parse_file_action(const char *action_str)
{
    size_t i;
    for(i=0; i<sizeof(file_action_kvs)/sizeof(file_action_kvs[0]); i++) {
        if(strcmp(action_str, file_action_kvs[i].p) == 0)
            return file_action_kvs[i].i;
    }
    return -1;
}

static void parse_file(char **toks, int num_toks, stap_interface_t *p_itf)
{
    if(num_toks != INDEX_FILE_MAX)
        return ;
    stap_file_action act;
    memset(&act, 0, sizeof(act));
    act.time = (time_t)atol(toks[INDEX_FILE_TIMESTAMP]);
    act.ppid = atoi(toks[INDEX_FILE_PARENT_PID]);
    if(!act.ppid)
        goto parse_file_end;
    act.pid = atoi(toks[INDEX_FILE_PID]);
    if(!act.pid)
        goto parse_file_end;
    act.file_name = parse_file_file_name(toks[INDEX_FILE_FILE_NAME], toks[INDEX_FILE_PWD]);
    if(!act.file_name)
        goto parse_file_end;
    act.action = parse_file_action(toks[INDEX_FILE_ACTION]);
    if(act.action < 0)
        goto parse_file_end;

    if(p_itf->p_callback) {
        p_itf->p_callback(&act, p_itf->p_user_data);
    }

parse_file_end:
    free((char *)act.file_name);
    return ;
}

static void parse_line(const char *line, stap_interface_t *p_itf)
{
    int num_toks = 0;
    char **toks = str_split(line, " \t\r\n", MAX_TOKS, &num_toks, 0);
    if(!toks || !num_toks) {
        debug_print("%s: str_split parse[%s] failed\n", __func__, line);
        return ;
    }
    parse_file(toks, num_toks, p_itf);

    if(toks && num_toks)
        str_split_free(&toks, num_toks);
    return ;
}

void set_flag(int fd, int flags)
{
    int val = fcntl(fd, F_GETFL, 0);
    val |= flags;

    fcntl(fd, F_SETFL, val);
}

int monitor_handler(file_system_monitor *p_monitor, stap_monitor *p_stap)
{
    int result = -1;
    if (p_monitor->dir_vec.empty() || NULL == p_monitor->notify_cb)
        return result;

    if( -1 == (p_monitor->ifd = inotify_init()))
        return result;

    int wd = -1;
    int monitor_flag = IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO | IN_ISDIR;

    p_monitor->m_mutex.enter();
    vector<string>::iterator dir_it = p_monitor->dir_vec.begin();

    for (; dir_it != p_monitor->dir_vec.end(); dir_it++)
    {
        wd = inotify_add_watch(p_monitor->ifd, (*dir_it).c_str(), monitor_flag);

        if (-1 != wd)
        {
            p_monitor->watch_vec.push_back(node_info_t (*dir_it, wd));
        }
    }
    p_monitor->m_mutex.leave();

    fd_set current_set;
    fd_set monitor_set;
    FD_ZERO(&monitor_set);

    string dir_path = "";
    string move_from = "";
    int length = 0;

    int read_index = 0;
    char inotify_buffer[PATH_MAX] = { 0 };

    struct inotify_event *p_event = NULL;

    char *stap_buffer = (char*)calloc(FGETS_MAX_SIZE, sizeof(char));

    int stap_fd = fileno(p_stap->rfp);
    // set_flag(stap_fd, O_NONBLOCK);

    FD_SET(p_monitor->ifd, &monitor_set);
    FD_SET(stap_fd, &monitor_set);

    result = 0;
    // int read_ret = 0;
    while(1 == p_stap->running)
    {
        /* fuck */
        if(!p_monitor->flag)
        {
            sleep(1);
            continue;
        }
        current_set = monitor_set;
        select(MAX(p_monitor->ifd, stap_fd) + 1, &current_set, NULL, NULL, NULL);

        if (FD_ISSET(stap_fd, &current_set))
        {
            if(fgets(stap_buffer, FGETS_MAX_SIZE, p_stap->rfp))
            {
                parse_line(stap_buffer, &p_stap->itf);
            }
            else
            {
                fprintf(stderr, "ERROR: fgets errno:%d\n", errno);
                result = (errno == 0 ? 1 : errno);
                break;
            }
        }
        if (FD_ISSET(p_monitor->ifd, &current_set)) {
            length = read(p_monitor->ifd, inotify_buffer, sizeof(inotify_buffer));
        }
        while( length > 0 )
        {
            p_event = (struct inotify_event*)&inotify_buffer[read_index];
            dir_path = p_monitor->get_dir(p_event->wd);
            dir_path.append("/");

            if (p_event->len > 0)
                dir_path.append(p_event->name);

            if (p_event->mask == (IN_CREATE | IN_ISDIR))
            {
                wd = inotify_add_watch(p_monitor->ifd, dir_path.c_str(), monitor_flag);
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
                    inotify_rm_watch(p_monitor->ifd, wd);
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
        read_index = 0;
        memset(inotify_buffer, 0, PATH_MAX);
    }
    free(stap_buffer);
    stap_buffer = NULL;

    if (p_monitor->ifd != -1)
    {
        p_monitor->m_mutex.enter();
        vector<node_info_t>::iterator it = p_monitor->watch_vec.begin();

        for (; it != p_monitor->watch_vec.end(); it++)
		{
			inotify_rm_watch(p_monitor->ifd, it->wd);
            close(it->wd);
		}
        p_monitor->watch_vec.clear();

        close(p_monitor->ifd);
        p_monitor->m_mutex.leave();
    }

    return result;
}


bool file_system_monitor::begin_monitor(int argc, char *args[],
                    event_notify_t notify_cb, void *p_user_data, stap_monitor *p_stap)
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
    if (monitor_handler(this, p_stap) == 0)
    {
        return true;
    }
    return false;
}

/* fuck */
void file_system_monitor::change_flag()
{
    this->flag = !this->flag;
    printf("file_monitor_sdk change thread run flag, now running:%d\n", this->flag);
}

void file_system_monitor::end_monitor()
{
    this->flag = 0;
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


