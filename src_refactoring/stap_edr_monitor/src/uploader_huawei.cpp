#include <inttypes.h>
#include <sys/time.h>

#include <unistd.h>     // rm, rmdir, unlink
#include <sys/stat.h>   // mkdir, stat
#include <sys/types.h>  // opendir
#include <dirent.h>     // readdir

#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <fstream>
#include <sstream>

#include "util-path.h"
#include "debug_print.h"
#include "sysinfo_logger.h"
#include "uploader_huawei.h"

#define CONF_ZIP_PATH           "zip_path"
#define CONF_MACHINE_CODE       "machine_code"
#define CONF_FILE_SIZE          "file_size"
#define CONF_FOLDER_SIZE        "folder_size"

#define DEFAULT_LOG_PATH        "/tmp/edr_log"
#define DEFAULT_ZIP_FOLDER      "/tmp/edr_zip"
#define DEFAULT_MACHINE_CODE    "MachineCode"
#define DEFAULT_UNIT_CAPACITY   (1 * 1024 * 1024)
#define DEFAULT_FOLDER_CAPACITY (3 * 1024 * 1024)

static const char *log_types[] = {
    "Proc_info",
    "Proc_module",
    "Proc_action",
    "File_action",
    "Net_action",
    "Dns_action"
};

static int clear_dir(std::string dir)
{
    DIR *d = opendir(dir.data());
    struct dirent *drt;
    while((drt = readdir(d)) != NULL)
    {
        if(strcmp(drt->d_name, ".") == 0
            || strcmp(drt->d_name, "..") == 0)
        {
            continue;
        }
        if(drt->d_type == DT_DIR)
        {
            clear_dir(dir + '/' + drt->d_name);
            debug_print("rmdir(%s/%s)..\n", dir.data(), drt->d_name);
            rmdir((dir + '/' + drt->d_name).data());
        }
        else
        {
            debug_print("rm(%s/%s)..\n", dir.data(), drt->d_name);
            unlink((dir + '/' + drt->d_name).data());
        }
    }
    closedir(d);
    return 0;
}

// ret: -1: error, see ERRNO
//      0 : success
static int makedir(std::string dir, mode_t mode)
{
    struct stat st;

    if (dir.size() == 0)
    {
        return -1;
    }

    if (dir[dir.size() - 1] == '/')
    {
        dir.erase(dir.size() - 1);
    }

    if (dir.size() == 0)
    {
        return 0;
    }
 
    if (stat(dir.data(), &st))
    {
        return mkdir(dir.data(), mode);
    }

    if (!S_ISDIR(st.st_mode))
    {
        if (unlink(dir.data()))
        {
            return -1;
        }
        return mkdir(dir.data(), mode);
    }
    return 0;
}

static int makedir_p(std::string dir, mode_t mode)
{
    if (makedir(dir.data(), mode) == 0)
    {
        return 0;
    }

    // parse parent string
    if (dir[dir.size() - 1] == '/')
    {
        dir.erase(dir.size() - 1);
    }
    size_t off = dir.find_last_of('/');
    if (off != std::string::npos)
    {
        // recursion
        makedir_p(dir.substr(0, off + 1), mode);
        return makedir(dir.data(), mode);
    }

    return -1;
}

__attribute__ ((unused)) static int conf_value_is_true(const char *value)
{
    int ret = 0;
    if (!value)
        ret = 0;
    else if (strcasestr(value, "yes"))
        ret = 1;
    else if (strcasestr(value, "enable"))
        ret = 1;
    else if (atoi(value) > 0)
        ret = 1;
    else if (strcasestr(value, "no"))
        ret = 0;
    else if (strcasestr(value, "disable"))
        ret = 0;
    else
        ret = 0;

    return ret;
}

static const char *conf_get_value(const char **p_pargs, uint32_t arg_count, const char *key)
{
    for (uint32_t i = 0; i<arg_count; i++)
    {
        if (NULL != strcasestr(p_pargs[i], key))
        {
            char *value = (char *)strchr(p_pargs[i], ':');
            if (value)
            {
                return value + 1;
            }
        }
    }
    return NULL;
}

static void upload_info_free(upload_info_t *p_info)
{
    if (p_info)
    {
        free((void *)p_info->buf);
        delete p_info;
    }
}

static void after_upload(bool ret, upload_info_t *p_info, void *p_user_data)
{
    upload_info_free(p_info);

    edr::HuaweiUploader *p_uploader = (edr::HuaweiUploader *)p_user_data;
    sysinfo_log(p_uploader->p_m_info, "%s ret:%d\n", __func__, ret);

    if(p_uploader)
    {
        p_uploader->stop();
    }
}

static void prepare_to_upload(void *p_handle, upload_info_t **pp_info, void **pp_user_data)
{
    *pp_info = NULL;
    *pp_user_data = NULL;

    return;
}

int edr::HuaweiUploader::logfile_rotate(std::string srcfile)
{
    if(!path_exists(srcfile.data()))
    {
        return -1;
    }
    char dstfile[128];
    char time[128];
    struct timeval tv;
    struct tm tm;
    gettimeofday(&tv, NULL);
    strftime(time, sizeof(time), "%Y_%m_%d_%H_%M_%S", localtime_r(&tv.tv_sec, &tm));
    snprintf(dstfile, sizeof(dstfile), "%s_%s", srcfile.data(), time);

    int ret = rename(srcfile.data(), dstfile);
    if(ret < 0)
    {
        sysinfo_log(this->p_m_info, "%s rename(%s -> %s) retv:%d, errno:%s\n",
            __func__, srcfile.data(), dstfile, ret, strerror(errno));
    }
    else
    {
        sysinfo_log(this->p_m_info, "%s rename(%s -> %s) retv:%d\n",
            __func__, srcfile.data(), dstfile, ret);
    }

    return ret;
}

bool edr::HuaweiUploader::is_srv_valid()
{
    bool ret = true;
    this->lock.read();
    ret = this->host.size() && this->port;
    this->lock.unlock();

    return ret;
}

void edr::HuaweiUploader::flush()
{
    if(this->uploading)
    {
        debug_print("flush uploading\n");
        return;
    }
    this->uploading = true;
    debug_print("flush starting\n");

    for(size_t i=0; i<sizeof(log_types)/sizeof(log_types[0]); i++)
    {
        logfile_rotate(this->storage_path + '/' + log_types[i]);
    }

    char time[128];
    struct timeval tv;
    struct tm tm;
    gettimeofday(&tv, NULL);
    strftime(time, sizeof(time), "%Y_%m_%d_%H_%M_%S", localtime_r(&tv.tv_sec, &tm));

    char zipname[256];
    snprintf(zipname, sizeof(zipname),
            "%s/%s_%s.zip",
            this->zip_path.data(), this->machine_code.data(), time);

    const char *dst_name = strrchr(zipname, '/');                                                                        
    if (dst_name)
    {
        dst_name++;
    }
    else
    {
        dst_name = zipname;
    }

    char url[1024];
    snprintf(url, sizeof(url), "ftp://%s:%d/%s",
        this->ip.data(), this->port, dst_name);

    // set p_info
    upload_info_t *p_info = new upload_info_t;
    p_info->type = UPLOAD_TYPE_BUF;
    p_info->url = url;
    p_info->user = this->user;
    p_info->passwd = this->passwd;
    p_info->timeout = 10;
    p_info->buf = NULL;
    p_info->buf_size = 0;
    p_info->name = this->storage_path;
    p_info->zip = zipname;
    p_info->iszip = true;

    this->p_upload_impl->common_upload(p_info, this);
}

void edr::HuaweiUploader::stop()
{
    this->running = false;
    if (this->p_upload_impl)
    {
        this->p_upload_impl->stop_upload();
        clear_dir(this->storage_path);
    }
}

int edr::HuaweiUploader::run()
{
    int ret = -1;
    if (this->p_upload_impl)
    {
        this->running = true;
        this->p_upload_impl->run_upload();
        ret = 0;
    }
    return ret;
}

edr::HuaweiUploader::~HuaweiUploader()
{
    if (this->p_upload_impl)
    {
        delete this->p_upload_impl;
    }
    clear_dir(this->storage_path);
    rmdir(this->storage_path.data());
    clear_dir(this->zip_path);
    rmdir(this->zip_path.data());

    sysinfo_log(this->p_m_info, "%s ok.\n", __func__);
}

edr::HuaweiUploader::HuaweiUploader(UploaderConf *conf) :
    p_m_info(conf ? conf->p_m_info : NULL), running(false), uploading(false), total_size(0)
{
    uploader_factory fact;
    this->p_upload_impl = fact.create_uploader(FTP_UPLOADER);
    assert(this->p_upload_impl);

    this->p_upload_impl->set_upload_cb(after_upload, prepare_to_upload, this);

    Json::CharReaderBuilder crb;
    this->p_reader = crb.newCharReader();
    this->ip = conf->ip;
    this->port = conf->port;
    this->user = conf->user;
    this->passwd = conf->passwd;

    // load config
    const char *val;

    val = conf_get_value(p_m_info->p_pargs, p_m_info->arg_count, CONF_ZIP_PATH);
    if (val == NULL)
    {
        val = DEFAULT_ZIP_FOLDER;
    }
    this->zip_path = val;
    if(makedir_p(val, 0777) < 0)
    {
        sysinfo_log(p_m_info, "%s makedir(%s) failed\n", __func__, val);
        perror(val);
    }

    val = conf_get_value(p_m_info->p_pargs, p_m_info->arg_count, CONF_MACHINE_CODE);
    if (val == NULL)
    {
        val = DEFAULT_MACHINE_CODE;
    }
    this->machine_code = val;

    this->storage_path = (conf && conf->log_path.size()) ? conf->log_path : DEFAULT_LOG_PATH;

    if(makedir_p(this->storage_path.data(), 0777) < 0)
    {
        sysinfo_log(p_m_info, "%s makedir(%s) failed\n", __func__, this->storage_path.data());
        perror(this->storage_path.data());
    }
    if(makedir_p(this->zip_path.data(), 0777))
    {
        sysinfo_log(p_m_info, "%s makedir(%s) failed\n", __func__, this->zip_path.data());
        perror(this->storage_path.data());
    }

    val = conf_get_value(p_m_info->p_pargs, p_m_info->arg_count, CONF_FILE_SIZE);
    if (val == NULL)
    {
        this->unit_capacity = DEFAULT_UNIT_CAPACITY;
    }
    else
    {
        this->unit_capacity = strtoull(val, NULL, 10);
    }

    val = conf_get_value(p_m_info->p_pargs, p_m_info->arg_count, CONF_FOLDER_SIZE);
    if (val == NULL)
    {
        this->total_capacity = DEFAULT_FOLDER_CAPACITY;
    }
    else
    {
        this->total_capacity = strtoull(val, NULL, 10);
    }

    sysinfo_log(this->p_m_info, "%s ok.\n", __func__);
}

size_t edr::HuaweiUploader::logfile_write(const char *filename, std::string &str, size_t *totalsz)
{

    std::string file = std::string() + this->storage_path + '/' + filename;
    FILE *fp = fopen(file.data(), "a");
    if (fp == NULL)
    {
        return -1;
    }

    size_t nwrite = fwrite(str.data(), 1, str.size(), fp);

    if(totalsz)
    {
        *totalsz = (size_t)ftell(fp);
    }

    debug_print("filename(%s), file:%lu/%luK, folder:%lu/%luK\n",
            filename, ftell(fp)/1024, (size_t)this->unit_capacity/1024, this->total_size/1024, this->total_capacity/1024);

    fclose(fp);

    if(nwrite > 0)
    {
        this->total_size += nwrite;
    }


    return nwrite;
}

size_t edr::HuaweiUploader::logfile_write(const char *filename, Json::Value &root, const char *type, size_t *totalsz)
{
    Json::Value json_info = root[type];
    if(json_info.isArray() == false || json_info.size() == 0 || json_info.empty())
    {
        return 0;
    }
    std::string str = Json::StyledWriter().write(json_info);

    return this->logfile_write(filename, str, totalsz);
}

int edr::HuaweiUploader::push_task(json_file *p_data)
{
    if (p_data == NULL)
    {
        return 0;
    }

    upload_info_t *p_info = new upload_info_t;
    assert(p_info);


    JSONCPP_STRING errs;
    Json::Value root;
    if (this->p_reader->parse(p_data->json_str.data(), p_data->json_str.data() + p_data->json_str.size(), &root, &errs) == false)
    {
        sysinfo_log(this->p_m_info, "%s Json::Reader.parse() failed\n",
            __func__);
        return -1;
    }

    for(size_t i=0; i<sizeof(log_types)/sizeof(log_types[0]); i++)
    {
        size_t sz = 0;
        this->logfile_write(log_types[i], root, log_types[i], &sz);
        if (sz >= (size_t)this->unit_capacity)
        {
            logfile_rotate(log_types[i]);
        }
    }

    if (this->total_size >= this->total_capacity)
    {
        this->flush();
    }

    return 0;
}

int edr::HuaweiUploader::push_task(task_record *p_data)
{
    this->files_lock.read();
    if (this->files.find(p_data->data) != this->files.end())
    {
        /* already log this file */
        this->files_lock.unlock();
        debug_print("file[%s] is already logged to dir [%s]\n",
            p_data->data.data(), this->storage_path.data());
        return 1;
    }
    this->files_lock.unlock();

    this->files_lock.write();
    this->files.insert(std::make_pair(p_data->data, time(NULL)));
    this->files_lock.unlock();

    std::ifstream ifs(p_data->data);
    if (!ifs)
    {
        sysinfo_log(this->p_m_info, "%s ifstream(%s) failed.\n",
            __func__, p_data->data.data());
        return -1;
    }
    std::ostringstream oss;
    oss << ifs.rdbuf();
    std::string data = oss.str();

    std::string filename(p_data->data);
    size_t pos = filename.find_last_of('/');
    if (pos != std::string::npos)
    {
        filename = filename.substr(pos+1);
    }
    this->logfile_write(filename.data(), data, NULL);
    return 0;
}

bool edr::HuaweiUploader::isstop()
{
    return !this->running;
}
