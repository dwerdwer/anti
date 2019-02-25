#pragma once

#include "json/json.h"

#include "sysinfo_interface.h"
#include "uploader.h"
#include "uploader_factory.h"
#include "cache_to_sqlite.h"
#include "util-lock.h"


namespace edr {

class HuaweiUploader : public edr::BaseUploader
{
public:
    HuaweiUploader(UploaderConf *conf);
    virtual ~HuaweiUploader();
    virtual int run();
    virtual void stop();
    virtual void flush();
    virtual int push_task(task_record *p_data);
    virtual int push_task(json_file *p_data);
    virtual bool isstop();

    virtual bool is_srv_valid();

    module_info_t *p_m_info;
    
private:
    int logfile_rotate(std::string srcfile);
    size_t logfile_write(const char *filename, std::string &data, size_t *totalsz);
    size_t logfile_write(const char *filename, Json::Value &root, const char *type, size_t *totalsz);

    Json::CharReader *p_reader;

    uploader *p_upload_impl;

    std::string storage_path;
    std::string zip_path;
    std::string machine_code;
    std::string ip;
    uint16_t port;
    std::string user;
    std::string passwd;

    std::map<std::string, time_t> files;
    util::rwlock_t files_lock;

    bool running;
    bool uploading;
    uint64_t unit_capacity;
    uint64_t total_size;
    uint64_t total_capacity;
};

} /* namespace edr */
