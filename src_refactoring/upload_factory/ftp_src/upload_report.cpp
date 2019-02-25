
#include <iostream>
#include <map>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include "zip.h" // needs to be placed after iostream
#include "curl/curl.h"

#include "debug_print.h"
#include "ftp_uploader.h"
#include "utils/utils_library.h"

/* 5 seconds 
 * 若多处理过程中有一个挂起的 timeout 比 MULTI_WAIT_MSECS 短 则将使用较短 timeout
 */
#define MULTI_WAIT_MAX 5*1000

struct curlitem_t
{
    CURL *curl;
    curl_slist *p_head;

    upload_info_t *p_info;
    void *p_param;

    char *zip_buf; // zip only
    FILE *fp;
    char *file;
    int err;
} ;

struct ftp_upload_handler
{
    ftp_upload_handler();
    ~ftp_upload_handler();

    void enter_lock(void);
    void leave_lock(void);

    CURLM *curl_m;

    bool run_flag;

    state_cb_t state_cb;
    active_cb_t active_cb;
    void *p_active_param;

    std::map<int64_t, curlitem_t *> curl_cache_map;
    private:
    pthread_spinlock_t spinlock;
};

/* note: member only */
static void release_item_content(curlitem_t *p_item)
{
    if (p_item->curl) {
        curl_easy_cleanup(p_item->curl);
        p_item->curl = NULL;
    }
    if (p_item->p_head) {
        curl_slist_free_all(p_item->p_head);
        p_item->p_head = NULL;
    }
    if (p_item->zip_buf) {
        delete [] p_item->zip_buf;
        p_item->zip_buf = NULL;
    }
    if (p_item->fp) {
        fclose(p_item->fp);
        p_item->fp = NULL;
    }
    if (p_item->file) {
        free(p_item->file);
        p_item->file = NULL;
    }
}

ftp_upload_handler::ftp_upload_handler()
{
    curl_global_init(CURL_GLOBAL_ALL);
    this->curl_m = curl_multi_init();
    this->run_flag = true;

    pthread_spin_init(&this->spinlock, 0);
}

ftp_upload_handler::~ftp_upload_handler()
{
    this->run_flag = false;

    pthread_spin_destroy(&this->spinlock);

    if (!this->curl_cache_map.empty()) // It makes sense to stop by force 
    {
        std::map<int64_t, curlitem_t *>::iterator mit = this->curl_cache_map.begin();
        for (; mit != this->curl_cache_map.end(); ++mit)
            release_item_content(mit->second);
        this->curl_cache_map.clear();
    }
    curl_multi_cleanup(this->curl_m);
    curl_global_cleanup();
}

void ftp_upload_handler::enter_lock(void)
{
    pthread_spin_lock(&this->spinlock);
}

void ftp_upload_handler::leave_lock(void)
{
    pthread_spin_unlock(&this->spinlock);
}
#if 1
typedef int (*on_file_cb) (std::string, void *);

static int on_file(std::string file, void *p_user_data)
{
    std::vector<std::string> *p_vec = (std::vector<std::string> *)p_user_data;
    p_vec->push_back(file);
    return 0;
}

static int read_dir(std::string file, void *p_user_data, on_file_cb cb)
{
    struct stat file_st;
    if (stat(file.c_str(), &file_st))
    {
        return -1; 
    }   
    if (S_ISDIR(file_st.st_mode))
    {
        DIR *p_dir = opendir(file.c_str());
        if (p_dir)
        {
            struct dirent *p_dirent;
            while((p_dirent = readdir(p_dir)) != NULL)
            {
                if (strcmp(p_dirent->d_name, ".") == 0
                    || strcmp(p_dirent->d_name, "..") == 0)
                {
                    continue;
                }   
                std::string sub_file = file + '/' + p_dirent->d_name;
                read_dir(sub_file, p_user_data, cb);
            }   
            closedir(p_dir);
        }   
    }   
    else
    {
        if(cb)
        {
            cb(file, p_user_data);
        }
    }   
    return 0;
}

static std::vector<std::string> list_dir(std::string file)
{
    std::vector<std::string> vec;
    read_dir(file, &vec, on_file);

    /* erase common path, remain relative path */
    size_t pos = file.find_last_of('/', file.size()-2);
    if(pos != std::string::npos)
    {
        for(std::vector<std::string>::iterator itr=vec.begin();
                itr!=vec.end(); ++itr)
        {
            std::string &s = *itr;
            s = s.substr(pos+1);
        }
    }
    return vec;
}

#endif

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *p_user_data)
{
    /* in real-world cases, this would probably get this data differently
     * as this fread() stuff is exactly what the library already would do
     * by default internally */ 

    curlitem_t *p_item = (curlitem_t *)p_user_data;

    if (p_item->err == -1) {
        fprintf(stderr, "ftp read_callback failed %d\n", p_item->err);
        return -1;
    }
    else if (p_item->err == 1) {
        fprintf(stderr, "ftp read_callback end %d\n", p_item->err);
        return 0;
    }

    if (p_item->fp == NULL) {
        p_item->fp = fopen(p_item->file, "rb");
        if (p_item->fp == NULL) {
            p_item->err = -1;
            fprintf(stderr, "ftp open %s failed\n", p_item->file);
            return -1;
        }
    }

    size_t nread = fread(ptr, size, nmemb, p_item->fp);

    if (nread < 0) {
        p_item->err = -1;
        fclose(p_item->fp);
        fprintf(stderr, "ftp read_callback failed %d\n", p_item->err);
    }
    else if (nread != nmemb) {
        p_item->err = 1;
        fclose(p_item->fp);
        p_item->fp = NULL;
        fprintf(stderr, "ftp read_callback end %d\n", p_item->err);
    }

    printf("ftp read_callback %luBytes\n", nread);

    return (curl_off_t)nread;
}

static curlitem_t *prepare_zip_upload(curlitem_t *p_result, const char *url, 
        const char *file_name, const char *zip_name, uint32_t timeout, const char *user, const char *passwd)
{
    if (p_result == NULL) {
        return p_result;
    }
    memset(p_result, 0, sizeof(curlitem_t));

    if (url == NULL || file_name == NULL || zip_name == NULL){
        return NULL;
    }

#if 1
    /* NOTE: must enumerate files before CreateZip avoid contain dest zip file */
    std::vector<std::string> files = list_dir(file_name);

    HZIP hzip = CreateZip(zip_name, NULL);
    for(std::vector<std::string>::iterator itr=files.begin(); itr != files.end(); ++itr) {
        ZipAdd(hzip, itr->c_str(), itr->c_str());
    }
    CloseZip(hzip);
#else
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "zip -r %s %s", zip_name, file_name);

    int zip_ret = system(cmd);
    zip_ret = WEXITSTATUS(zip_ret);
#endif
    debug_print("zip [%s] -> [%s]\n", file_name, zip_name);

    struct stat file_info;
    curl_off_t fsize;

    p_result->file = strdup(zip_name);

    // struct curl_slist *headerlist = NULL;

    if (stat(zip_name, &file_info)) {
        return NULL;
    }

    p_result->curl = curl_easy_init();
    if (NULL == p_result->curl) {
        return p_result;
    }

    fsize = (curl_off_t)file_info.st_size;

    // char buf_1[256], buf_2[256];
    // const char *dst_name = strrchr(url, '/');
    // if (dst_name) {
    //     dst_name++;
    // }
    // else {
    //     dst_name = "error_url";
    // }

    // snprintf(buf_1, sizeof(buf_1), "RNFR %s_ftp_uploading", dst_name);
    // snprintf(buf_2, sizeof(buf_2), "RNTO %s", dst_name);
    // headerlist = curl_slist_append(headerlist, buf_1);
    // headerlist = curl_slist_append(headerlist, buf_2);

    /* we want to use our own read function */ 
    curl_easy_setopt(p_result->curl, CURLOPT_READFUNCTION, read_callback);

    /* enable uploading */ 
    curl_easy_setopt(p_result->curl, CURLOPT_UPLOAD, 1L);

    /* pass in that last of FTP commands to run after the transfer */ 
    // curl_easy_setopt(p_result->curl, CURLOPT_POSTQUOTE, headerlist);

    /* now specify which file to upload */ 
    curl_easy_setopt(p_result->curl, CURLOPT_READDATA, p_result);

    /* Set the size of the file to upload (optional).  If you give a *_LARGE
     * option you MUST make sure that the type of the passed-in argument is a
     * curl_off_t. If you use CURLOPT_INFILESIZE (without _LARGE) you must
     * make sure that to pass in a type 'long' argument. */ 
    curl_easy_setopt(p_result->curl, CURLOPT_INFILESIZE_LARGE,
            (curl_off_t)fsize);

    /* specify target */ 
    curl_easy_setopt(p_result->curl, CURLOPT_URL, url);
    debug_print("FTP URL:[%s]\n", url);

    if(user && passwd)
    {
        char userpswd[128];
        snprintf(userpswd, sizeof(userpswd), "%s:%s", user, passwd);
        curl_easy_setopt(p_result->curl, CURLOPT_USERPWD, userpswd);
    }

    curl_easy_setopt(p_result->curl, CURLOPT_TIMEOUT, timeout);

    return p_result;
}

LIB_PUBLIC ftp_upload_handler_t *ftp_init_upload_handler()
{
    ftp_upload_handler_t *p_upload = new ftp_upload_handler_t; 

    return p_upload;
}

LIB_PUBLIC void ftp_priv_upload_cb(ftp_upload_handler_t *p_upload, state_cb_t state_cb, 
        active_cb_t active_cb, void *p_active_param)
{
    if (NULL == p_upload)
        return;

    p_upload->state_cb = state_cb;
    p_upload->active_cb = active_cb;
    p_upload->p_active_param = p_active_param;
}

static void multi_read_return(ftp_upload_handler_t *p_upload)
{
    /* return part */
    std::map<int64_t, curlitem_t *>::iterator mit;
    curlitem_t *p_curr_item;
    int     msgs_left       = 0;
    CURLMsg *multi_msg      = NULL;
    bool flag;        

    while((multi_msg = curl_multi_info_read(p_upload->curl_m, &msgs_left)))
    {
        if (CURLMSG_DONE == multi_msg->msg)
        {   
            p_upload->enter_lock();
            mit = p_upload->curl_cache_map.find((int64_t)multi_msg->easy_handle);

            if (mit != p_upload->curl_cache_map.end()) {
                p_curr_item = mit->second;

                p_upload->curl_cache_map.erase(mit);
            }
            curl_multi_remove_handle(p_upload->curl_m, multi_msg->easy_handle);
            p_upload->leave_lock();

            if (CURLE_OK == multi_msg->data.result) {
                flag = true;
            }
            else {
                flag = false;
            }
            debug_print("CURL RETURN CODE: %d\n", multi_msg->data.result);
            p_upload->state_cb(flag, p_curr_item->p_info, p_curr_item->p_param);
            if (p_curr_item->curl) {
                release_item_content(p_curr_item);
                free(p_curr_item);
            }
        }
    }
}

LIB_PUBLIC void ftp_priv_run_upload(ftp_upload_handler_t *p_upload)
{
    if (NULL == p_upload)
        return;

    int running_number  = 0; // current runing number
    int wait_fd         = 0;

    upload_info_t *p_active_info = NULL;
    void *p_cb_param    = NULL;

    while(p_upload->run_flag)
    {
        do {
            wait_fd = 0;
            if (CURLM_OK != curl_multi_wait(p_upload->curl_m, NULL, 0, 
                        MULTI_WAIT_MAX, &wait_fd)) {
                break;
            }
            curl_multi_perform(p_upload->curl_m, &running_number);

            multi_read_return(p_upload);

        } while(running_number);

        running_number = 0;
        /* active access to external cached data */
        if (NULL != p_upload->active_cb) {
            p_cb_param = NULL;
            p_active_info = NULL;
            p_upload->active_cb(p_upload->p_active_param, &p_active_info, &p_cb_param);

            if(p_active_info) {
                ftp_priv_common_upload(p_upload, p_active_info, p_cb_param);
            }
        }
        usleep(500000); // 0.5s
        //debug_print("- - - BIG UPLOAD LOOP RUN - - -\n");
    }
}

LIB_PUBLIC void ftp_priv_stop_upload(ftp_upload_handler_t *p_upload)
{
    if (NULL == p_upload)
        return;

    p_upload->run_flag = false;
}

LIB_PUBLIC int ftp_priv_common_upload(ftp_upload_handler_t *p_upload, 
        upload_info_t *p_info, void *p_param)
{
    fprintf(stdout, "%s start..\n", __func__);
    int result = -1;
    if (NULL == p_upload || NULL == p_info) {
        fprintf(stdout, "%s ret %d\n", __func__, result);
        return result; 
    }
    curlitem_t *p_item = new curlitem_t;
    memset(p_item, 0, sizeof(curlitem_t));

    switch (p_info->type)
    {
        case UPLOAD_TYPE_BUF:
            prepare_zip_upload(p_item, p_info->url.c_str(), p_info->name.c_str(), p_info->zip.c_str(), p_info->timeout, p_info->user.size() ? p_info->user.data() : NULL, p_info->passwd.size() ? p_info->passwd.data() : NULL);
            break;
        default:
            fprintf(stderr, "%s:%d %s WARNING: unimplement type:%d\n",
                    __FILE__, __LINE__, __func__, p_info->type);
            break;
    }
    if (p_item->curl != NULL)
    {   
        p_item->p_info     = p_info;
        p_item->p_param    = p_param;

        p_upload->enter_lock();
        // remove after the send
        if ( 0 == curl_multi_add_handle(p_upload->curl_m, p_item->curl))
        {
            p_upload->curl_cache_map.insert(std::pair<int64_t, curlitem_t *>((int64_t)p_item->curl, p_item));
            result = 0;
        }
        p_upload->leave_lock();
    }
    fprintf(stdout, "%s ret %d\n", __func__, result);
    return result;
}

LIB_PUBLIC void ftp_release_upload_handler(ftp_upload_handler_t *p_upload)
{
    if (p_upload) {
        delete p_upload;
    }
}
