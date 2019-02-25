#include <inttypes.h>

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#include "sysinfo_logger.h"
#include "uploader_edr.h"


typedef struct {
    int business_type;

    int64_t cache_id;
    json_cache_t *p_cache;

    edr::EDRUploader     *p_uploader;
} uploading_ctx;


static void uploading_ctx_free(uploading_ctx *p_context)
{
    if (p_context)
    {
        free(p_context);
    }
}

static void upload_info_free(upload_info_t *p_info)
{
    if (p_info)
    {
        free((void *)p_info->buf);
        delete p_info;
    }
}

#define BUSSINESS_TYPE_SNAP_SHOT    1
#define BUSSINESS_TYPE_PROC_ACTION  2
#define BUSSINESS_TYPE_SYS_TASK     3

static void after_upload(bool success, upload_info_t *p_info, void *p_user_data)
{
    if (!p_user_data || !p_info)
    {
        return;
    }
    uploading_ctx *p_context = (uploading_ctx *)p_user_data;

    sysinfo_log(p_context->p_uploader->p_m_info, "%s upload %s result(%s). url:%s\n", __func__,
        (p_context->business_type == SYS_UPLOAD_LOG_TYPE_SNAP_SHOT ? "snapshots" :
            p_context->business_type == SYS_UPLOAD_LOG_TYPE_PROC_ACTION ? "procaction" :
            "unknown"
            ),
        success ? "success" : "failed",
        p_info->url.data()
    );

    if (success)
    {
        if (p_context->cache_id)
        {
            // remove from db
            delete_jsonstr_by_id(p_context->p_cache, p_context->cache_id);
            sysinfo_log(p_context->p_uploader->p_m_info, "%s upload is success:%d. delete from db\n", __func__, success);
        }
        else
        {
            // do nothing
        }
    }
    else
    {
        if (p_context->cache_id)
        {
            // keep in db, do nothing
        }
        else
        {
            // cache to db
            set_jsonstr_to_cache(p_context->p_cache, p_info->buf);
            sysinfo_log(p_context->p_uploader->p_m_info, "%s upload is success:%d. insert into db\n", __func__, success);
        }
    }
    upload_info_free(p_info);
    uploading_ctx_free(p_context);
}

typedef struct {
    int64_t id;

    char *p_data;
    size_t data_len;
} cache_info_t;

static int get_data_from_cache(json_cache_t *p_cache, cache_info_t *p_info)
{
    int err = 0;

    if (!p_cache || !p_info)
    {
        err = -1;
    }

    if (err == 0)
    {
        p_info->id = get_jsonstr_cache_top_id(p_cache);
        if (p_info->id <= 0)
        {
            err = -1;
        }
    }
    if (err == 0)
    {
        p_info->p_data = get_jsonstr_by_id(p_cache, p_info->id, &p_info->data_len);
        if (!p_info->p_data)
        {
            err = -1;
        }
    }

    return err;
}

static void prepare_to_upload(void *p_handle, upload_info_t **pp_info, void **pp_user_data)
{
    if (!p_handle)
    {
        return;
    }

    edr::EDRUploader *p_uploader = (edr::EDRUploader *)p_handle;
    if (!p_uploader->is_srv_valid())
    {
        sysinfo_log(p_uploader->p_m_info, "%s server is invalid, return.\n", __func__);
        return;
    }

    cache_info_t cache_info;
    int business_type = 0;
    content_type_t content_type;
    const char *url_type = NULL;
    const char *name = NULL;
    const char *zip = NULL;
    bool iszip = false;
    json_cache_t *p_cache;

    if (get_data_from_cache(p_uploader->p_ss_cache, &cache_info) == 0)
    {
        business_type = SYS_UPLOAD_LOG_TYPE_SNAP_SHOT;
        content_type = UPLOAD_TYPE_BUF;
        url_type = "edrsnap";
        name = "file.json";
        zip = "snap.zip";
        iszip = true;
        p_cache = p_uploader->p_ss_cache;
        sysinfo_log(p_uploader->p_m_info, "%s get_data_from_cache -> snapshots.\n", __func__);
    }
    else if (get_data_from_cache(p_uploader->p_pa_cache, &cache_info) == 0)
    {
        business_type = SYS_UPLOAD_LOG_TYPE_PROC_ACTION;
        content_type = UPLOAD_TYPE_BUF;
        url_type = "edraction";
        name = "file.json";
        zip = "action.zip";
        iszip = true;
        p_cache = p_uploader->p_pa_cache;
        sysinfo_log(p_uploader->p_m_info, "%s get_data_from_cache -> procaction.\n", __func__);
    }
    else
    {
        return;
    }

    char url[1024];
    snprintf(url, sizeof(url), "http://%s:%d/edr/report/file.action?token=%s&type=%s",
        p_uploader->get_host().data(), p_uploader->get_port(), p_uploader->get_token().data(), url_type);

    uploading_ctx *p_context = (uploading_ctx *)calloc(1, sizeof(uploading_ctx));
    assert(p_context);
    upload_info_t *p_info = new upload_info_t;
    assert(p_info);

    // set p_info
    p_info->type = content_type;
    p_info->url = url;
    p_info->timeout = 10;
    p_info->buf = cache_info.p_data;
    p_info->buf_size = cache_info.data_len;
    p_info->name = name;
    p_info->zip = zip;
    p_info->iszip = iszip;

    // set p_context
    p_context->business_type = business_type;
    p_context->cache_id = cache_info.id;
    p_context->p_cache = p_cache;
    p_context->p_uploader = p_uploader;

    *pp_info = p_info;
    *pp_user_data = (void *)p_context;

    // return business_type : 0 : -1;
}


#ifdef UPLOAD_DEBUG

#define UPLOAD_HISTORY "./debug_upload_history.txt"
#include <sys/time.h>
#include <time.h>
#include <string.h>
static void store_file(const char *p_name, const char *p_data)
{
    static int total_debug_file = 0;
    static int max_debug_file = 100;

    if (max_debug_file <= total_debug_file)
        return;

    struct timeval tv;
    if (gettimeofday(&tv, NULL))
    {
        return;
    }
    struct tm tm1;
    localtime_r(&tv.tv_sec, &tm1);
    char fname[1024];
    fname[0] = '\0';
    snprintf(fname + strlen(fname), sizeof(fname) - strlen(fname), "debug_%s_", p_name);
    strftime(fname + strlen(fname), sizeof(fname) - strlen(fname),
        "%Y-%m-%d_%H-%M-%S.txt", &tm1);
    FILE *p_file = fopen(fname, "w");
    if (!p_file)
    {
        return;
    }
    fputs(p_data, p_file);
    fclose(p_file);

    total_debug_file++;
}

#endif /* UPLOAD_DEBUG */


bool edr::EDRUploader::is_srv_valid()
{
    bool ret = true;
    this->lock.read();
    ret = (this->host.size() > 0
        && this->port > 0
        && this->token.size() > 0);
    this->lock.unlock();
    return ret;
}

void edr::EDRUploader::flush()
{
}

void edr::EDRUploader::stop()
{
    if (this->p_upload_impl)
    {
        this->p_upload_impl->stop_upload();
    }
}

int edr::EDRUploader::run()
{
    int ret = -1;
    if (this->p_upload_impl)
    {
        this->p_upload_impl->run_upload();
        ret = 0;
    }
    return ret;
}

edr::EDRUploader::~EDRUploader()
{
    if (this->p_ss_cache)
    {
        destroy_jsonstr_cache(this->p_ss_cache);
    }
    if (this->p_pa_cache)
    {
        destroy_jsonstr_cache(this->p_pa_cache);
    }
    if (this->p_upload_impl)
    {
        delete this->p_upload_impl;
    }
    sysinfo_log(this->p_m_info, "%s ok.\n", __func__);
}

edr::EDRUploader::EDRUploader(UploaderConf *conf) :
    p_m_info(conf ? conf->p_m_info : NULL)
{
    this->p_ss_cache = create_jsonstr_cache("./snap_shot_cache.db");
    assert(this->p_ss_cache);
    this->p_pa_cache = create_jsonstr_cache("./proc_action_cache.db");
    assert(this->p_pa_cache);

    uploader_factory fact;
    this->p_upload_impl = fact.create_uploader(HTTP_UPLOADER);
    assert(this->p_upload_impl);

    this->p_upload_impl->set_upload_cb(after_upload, prepare_to_upload, this);

    sysinfo_log(this->p_m_info, "%s ok.\n", __func__);
}

int edr::EDRUploader::push_task(json_file *p_data)
{
    if (!p_data)
    {
        sysinfo_log(this->p_m_info, "%s WARNING: data is NULL..\n", __func__);
        return -1;
    }

#ifdef UPLOAD_DEBUG
    store_file(p_data->url_type.data(), p_data->json_str.data());
#endif /* UPLOAD_DEBUG */

    if (!this->is_srv_valid())
    {
        if (p_data->type == SYS_UPLOAD_LOG_TYPE_SNAP_SHOT)
        {
            set_jsonstr_to_cache(this->p_ss_cache, p_data->json_str.data());
        }
        else if (p_data->type == SYS_UPLOAD_LOG_TYPE_PROC_ACTION)
        {
            set_jsonstr_to_cache(this->p_pa_cache, p_data->json_str.data());
        }
        sysinfo_log(this->p_m_info, "%s server is invalid, save to db, return.\n", __func__);
        return 0;
    }

    uploading_ctx *p_context = (uploading_ctx *)calloc(1, sizeof(uploading_ctx));
    assert(p_context);
    upload_info_t *p_info = new upload_info_t;
    assert(p_info);

    char url[1024];
    this->lock.read();
    snprintf(url, sizeof(url), "http://%s:%d/edr/report/file.action?token=%s&type=%s",
        this->host.data(), this->port, this->token.data(), p_data->url_type.data());
    this->lock.unlock();

    // set p_info
    p_info->type = UPLOAD_TYPE_BUF;
    p_info->url = url;
    p_info->timeout = 10;
    p_info->buf = strdup(p_data->json_str.data());
    p_info->buf_size = p_data->json_str.size();
    p_info->name = p_data->json_name.data();
    p_info->zip = p_data->zip_name.data();
    p_info->iszip = true;

    // set p_context
    p_context->business_type = p_data->type;
    p_context->cache_id = 0;
    p_context->p_uploader = this;
    if (p_context->business_type == SYS_UPLOAD_LOG_TYPE_SNAP_SHOT)
        p_context->p_cache = this->p_ss_cache;
    else if (p_context->business_type == SYS_UPLOAD_LOG_TYPE_PROC_ACTION)
        p_context->p_cache = this->p_pa_cache;
    sysinfo_log(this->p_m_info, "%s pull %s to upload.\n", __func__, p_data->url_type.data());

    return this->p_upload_impl->common_upload(p_info, p_context);
}

int edr::EDRUploader::push_task(task_record *p_data)
{
    if (!p_data)
    {
        sysinfo_log(this->p_m_info, "%s WARNING: data is NULL..\n", __func__);
        return -1;
    }

#ifdef UPLOAD_DEBUG
    store_file(p_data->type.data(), p_data->data.data());
#endif /* UPLOAD_DEBUG */

    if (!this->is_srv_valid())
    {
        sysinfo_log(this->p_m_info, "%s server is invalid, return.\n", __func__);
        return 0;
    }
    sysinfo_log(this->p_m_info, "%s is ready to upload.\n", __func__);

    uploading_ctx *p_context = (uploading_ctx *)calloc(1, sizeof(uploading_ctx));
    assert(p_context);
    upload_info_t *p_info = new upload_info_t;
    assert(p_info);

    char url[1024];
    this->lock.read();
    snprintf(url, sizeof(url), "http://%s:%d/edr/report/file.action?token=%s&type=%s",
        this->host.data(), this->port, this->token.data(), p_data->type.data());
    this->lock.unlock();

    // set p_info
    switch (p_data->upload_type)
    {
    case SYS_UPLOAD_LOG_TYPE_MD5_LOG:
    case SYS_UPLOAD_LOG_TYPE_ACTION_RULE:
        p_info->type = UPLOAD_TYPE_BUF;
        break;
    case SYS_UPLOAD_LOG_TYPE_PROC_FILE:
    case SYS_UPLOAD_LOG_TYPE_MD5_FILE:
        p_info->type = UPLOAD_TYPE_FILE;
        break;
    }
    p_info->url = url;
    p_info->timeout = 10;
    p_info->buf = strdup(p_data->data.data());
    p_info->buf_size = p_data->data.size();

    // set p_context
    p_context->business_type = 3;
    p_context->p_uploader = this;

    return this->p_upload_impl->common_upload(p_info, p_context);
}

bool edr::EDRUploader::isstop()
{
    return false;
}
