
#include <iostream>
#include <map>
#include <stdio.h>
#include <cstring>
#include <sys/select.h>
#include <unistd.h>

#include "zip.h" // needs to be placed after iostream
#include "curl/curl.h"

#include "debug_print.h"
#include "http_uploader.h"
#include "utils/utils_library.h"

/* 5 seconds 
 * 若多处理过程中有一个挂起的 timeout 比 MULTI_WAIT_MSECS 短 则将使用较短 timeout
 */
#define MULTI_WAIT_MAX 5*1000

typedef struct
{
    CURL *curl;
    curl_slist *p_head;
    struct curl_httppost *p_post;
    
    upload_info_t *p_info;
    void *p_param;

    char *zip_buf; // zip only

}curlitem_t;

struct upload_handler
{
    upload_handler();
    ~upload_handler();

    void enter_lock(void);
    void leave_lock(void);
    
    CURLM *curl_m;
    
    bool run_flag;
    
    state_cb_t state_cb;
    active_cb_t active_cb;
    void *p_active_param;

    std::map<int64_t, curlitem_t> curl_cache_map;
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
    if (p_item->p_post) {
        curl_formfree(p_item->p_post);
        p_item->p_post = NULL;
    }
    if (p_item->p_head) {
        curl_slist_free_all(p_item->p_head);
        p_item->p_head = NULL;
    }
    if (p_item->zip_buf) {
        delete [] p_item->zip_buf;
        p_item->zip_buf = NULL;
    }
}

upload_handler::upload_handler()
{
    curl_global_init(CURL_GLOBAL_ALL);
    this->curl_m = curl_multi_init();
    this->run_flag = true;

    pthread_spin_init(&this->spinlock, 0);
}

upload_handler::~upload_handler()
{
    this->run_flag = false;

    pthread_spin_destroy(&this->spinlock);

    if (!this->curl_cache_map.empty()) // It makes sense to stop by force 
    {
        std::map<int64_t, curlitem_t>::iterator mit = this->curl_cache_map.begin();
        for (; mit != this->curl_cache_map.end(); ++mit)
            release_item_content(&(mit->second));
        this->curl_cache_map.clear();
    }
    curl_multi_cleanup(this->curl_m);
    curl_global_cleanup();
}

void upload_handler::enter_lock(void)
{
    pthread_spin_lock(&this->spinlock);
}
    
void upload_handler::leave_lock(void)
{
    pthread_spin_unlock(&this->spinlock);
}

static curlitem_t prepare_buf_upload(const char *url, const char *buf_name, 
                            const char *buf, uint64_t buf_size, uint32_t timeout)
{
    curlitem_t result;
    memset(&result, 0, sizeof(curlitem_t));
    
	if (NULL == url || NULL == buf_name || NULL == buf) {
        return result; 
    }
    result.curl = curl_easy_init();
    
    if(NULL == result.curl)
        return result;

    struct curl_httppost *lastptr = NULL;

    result.p_head = curl_slist_append(result.p_head, "Expect:");
    curl_easy_setopt(result.curl, CURLOPT_HTTPHEADER, result.p_head);
    
    curl_formadd(&result.p_post, &lastptr, CURLFORM_COPYNAME, "file",
                 CURLFORM_BUFFER, buf_name, CURLFORM_BUFFERPTR, buf,
                    // !!! for compatible must convert to long 
                 CURLFORM_BUFFERLENGTH, (long)buf_size, CURLFORM_END);

    curl_formadd(&result.p_post, &lastptr, CURLFORM_COPYNAME, "submit",
                 CURLFORM_COPYCONTENTS, "Submit", CURLFORM_END);
    
    curl_easy_setopt(result.curl, CURLOPT_URL, url);

    curl_easy_setopt(result.curl, CURLOPT_TIMEOUT, timeout);
    
    curl_easy_setopt(result.curl, CURLOPT_HTTPPOST, result.p_post); 

	return result;
}

static int zip_to_buf(const char *p_buf, const int buf_size, 
                      const char *file_name, char *zip_buf, int *p_zip_size)
{
    int result = -1;
    
    if ( NULL == zip_buf || NULL == p_buf) {
        return result;
    }
    // TODO:replace this buf
    char *target_buf = new char[*p_zip_size]();
    
    HZIP current_zip = CreateZip(target_buf, (unsigned int)*p_zip_size, NULL);
    
    if (NULL == current_zip) {
        delete [] target_buf;
        return result;
    }
    if (ZipAdd(current_zip, file_name, (void*)p_buf, buf_size) == ZR_OK) 
    {
        char *p_index = NULL;
        unsigned long mem_len = 0;

        if (ZipGetMemory(current_zip, (void**)&p_index, &mem_len) == ZR_OK) 
        {
            if ((unsigned long)*p_zip_size >= mem_len) 
                memcpy(zip_buf, p_index, mem_len);

            *p_zip_size = mem_len;
            result = 0;
        }
    }
    delete [] target_buf;
    target_buf = NULL;

    CloseZip(current_zip);
    return result;
}

static curlitem_t prepare_zip_upload(const char *url, const char *file_name, 
                        const char *zip_name, const char *p_buf, uint64_t buf_size, uint32_t timeout)
{
    curlitem_t result;
    memset(&result, 0, sizeof(curlitem_t));
    
    if (NULL == url || NULL == file_name || NULL == zip_name || NULL == p_buf) {
        return result;
    }
    int zip_size = buf_size + 1024;
    char *zip_buf = new char[zip_size];
    
    if (NULL != zip_buf)
    {
        if (0 == zip_to_buf(p_buf, buf_size, file_name, zip_buf, &zip_size)) {
            result = prepare_buf_upload(url, zip_name, zip_buf, zip_size, timeout);
        }
        if (NULL == result.curl) {
            delete [] zip_buf;
            zip_buf = NULL;
        } else {
            result.zip_buf = zip_buf;
        }
    }
    return result;
}

static curlitem_t prepare_file_upload(const char *url, const char *file_path, uint32_t timeout)
{
    curlitem_t result;
    memset(&result, 0, sizeof(curlitem_t));
	
    if (NULL == url || NULL == file_path){
		return result;
	}

    result.curl = curl_easy_init();
    if (NULL == result.curl) {
        return result;
    }
    struct curl_httppost *lastptr = NULL;
    
    result.p_head = curl_slist_append(result.p_head, "Expect:");
    
    curl_formadd(&result.p_post, &lastptr, CURLFORM_COPYNAME, 
                 "file", CURLFORM_FILE, file_path, CURLFORM_END);
    curl_formadd(&result.p_post, &lastptr, CURLFORM_COPYNAME, 
                 "submit", CURLFORM_COPYCONTENTS, "Submit", CURLFORM_END);
    curl_easy_setopt(result.curl, CURLOPT_URL, url);
    
    curl_easy_setopt(result.curl, CURLOPT_HTTPHEADER, result.p_head);
    curl_easy_setopt(result.curl, CURLOPT_HTTPPOST, result.p_post);
    curl_easy_setopt(result.curl, CURLOPT_TIMEOUT, timeout);
    
    return result;
}

static curlitem_t prepare_log_upload(const char *url, const char *p_buf, uint64_t buf_size, uint32_t timeout)
{
    curlitem_t result;
    memset(&result, 0, sizeof(curlitem_t));
    
    if (NULL == url || NULL == p_buf) {
        return result;
    }
    result.curl = curl_easy_init();
    
    if (NULL == result.curl)
        return result;

    result.p_head = curl_slist_append(result.p_head, "content-type: text/plain");

    curl_easy_setopt(result.curl, CURLOPT_URL, url);
    curl_easy_setopt(result.curl, CURLOPT_HTTPHEADER, result.p_head);

    curl_easy_setopt(result.curl, CURLOPT_POSTFIELDS, p_buf);
    curl_easy_setopt(result.curl, CURLOPT_POSTFIELDSIZE, buf_size);

    curl_easy_setopt(result.curl, CURLOPT_TIMEOUT, timeout);

    return result;
}

LIB_PUBLIC upload_handler_t *init_upload_handler()
{
    upload_handler_t *p_upload = new upload_handler_t; 

    return p_upload;
}

LIB_PUBLIC void priv_upload_cb(upload_handler_t *p_upload, state_cb_t state_cb, 
                               active_cb_t active_cb, void *p_active_param)
{
    if (NULL == p_upload)
        return;

    p_upload->state_cb = state_cb;
    p_upload->active_cb = active_cb;
    p_upload->p_active_param = p_active_param;
}

static void multi_read_return(upload_handler_t *p_upload)
{
    /* return part */
    std::map<int64_t, curlitem_t>::iterator mit;
    curlitem_t current_item;    // current curl item
    int     msgs_left       = 0;
    int     http_code       = 0;
    CURLMsg *multi_msg      = NULL;
    //bool flag               = false;        
    
    while((multi_msg = curl_multi_info_read(p_upload->curl_m, &msgs_left)))
    {
        if (CURLMSG_DONE == multi_msg->msg)
        {   
            http_code = 0;
            memset(&current_item, 0, sizeof(curlitem_t));

            p_upload->enter_lock();
            mit = p_upload->curl_cache_map.find((int64_t)multi_msg->easy_handle);

            if (mit != p_upload->curl_cache_map.end()) {
                memcpy(&current_item, &(mit->second), sizeof(curlitem_t));

                p_upload->curl_cache_map.erase(mit);
            }
            curl_multi_remove_handle(p_upload->curl_m, multi_msg->easy_handle);
            p_upload->leave_lock();

            curl_easy_getinfo(multi_msg->easy_handle, CURLINFO_RESPONSE_CODE, &http_code);

            debug_print("CURL RETURN CODE: %d HTTP CODE: %d\n", multi_msg->data.result, http_code);
            if (CURLE_OK == multi_msg->data.result && http_code < 400)
            {
                p_upload->state_cb(true, current_item.p_info, current_item.p_param);
            } else {
#ifdef DEBUG
                debug_print("FOR DEBUG\n");
                p_upload->state_cb(true, current_item.p_info, current_item.p_param);
#else
                p_upload->state_cb(false, current_item.p_info, current_item.p_param);
#endif
            }
            if (current_item.curl)
                release_item_content(&(current_item));
        }
    }
}

LIB_PUBLIC void priv_run_upload(upload_handler_t *p_upload)
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
            
            priv_common_upload(p_upload, p_active_info, p_cb_param);
        }
        usleep(500000); // 0.5s
        //debug_print("- - - BIG UPLOAD LOOP RUN - - -\n");
    }
}

LIB_PUBLIC void priv_stop_upload(upload_handler_t *p_upload)
{
    if (NULL == p_upload)
        return;

    p_upload->run_flag = false;
}

LIB_PUBLIC int priv_common_upload(upload_handler_t *p_upload, 
                             upload_info_t *p_info, void *p_param)
{
	int result = -1;
	if (NULL == p_upload || NULL == p_info || NULL == p_upload->state_cb) {
        return result; 
    }
    curlitem_t item;
    memset(&item, 0, sizeof(curlitem_t));
    //debug_print("TARGET URL: %s\n", p_info->url);
    switch (p_info->type)
    {
    case UPLOAD_TYPE_BUF:
       
        if (p_info->iszip) {
            item = prepare_zip_upload(p_info->url.c_str(), p_info->name.c_str(), p_info->zip.c_str(), 
                                      p_info->buf, p_info->buf_size, p_info->timeout);
        } else if (!p_info->name.empty()) {
            item = prepare_buf_upload(p_info->url.c_str(), p_info->name.c_str(), 
                                      p_info->buf, p_info->buf_size, p_info->timeout);
        } else {
            item = prepare_log_upload(p_info->url.c_str(), p_info->buf, p_info->buf_size, p_info->timeout);
        }
        break;
    case UPLOAD_TYPE_FILE:
        item = prepare_file_upload(p_info->url.c_str(), p_info->name.c_str(), p_info->timeout);
        break;
    default:
        break;
    }
    if (item.curl != NULL)
    {   
        item.p_info     = p_info;
        item.p_param    = p_param;

        p_upload->enter_lock();
        // remove after the send
        if ( 0 == curl_multi_add_handle(p_upload->curl_m, item.curl))
        {
            p_upload->curl_cache_map.insert(std::pair<int64_t, curlitem_t>((int64_t)item.curl, item));
            result = 0;
        }
        p_upload->leave_lock();
    }
	return result;
}

LIB_PUBLIC void release_upload_handler(upload_handler_t *p_upload)
{
    if (p_upload) {
        delete p_upload;
    }
}
