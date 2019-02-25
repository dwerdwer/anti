#pragma once

#include "sysinfo_interface.h"
#include "uploader.h"
#include "uploader_factory.h"
#include "cache_to_sqlite.h"


namespace edr {

class EDRUploader : public edr::BaseUploader
{
public:
    EDRUploader(UploaderConf *conf);
    virtual ~EDRUploader();
    virtual int run();
    virtual void stop();
    virtual void flush();
    virtual int push_task(task_record *p_data);
    virtual int push_task(json_file *p_data);
    virtual bool isstop();

    module_info_t *p_m_info;
    virtual bool is_srv_valid();

    // snap_shot == ss
    json_cache_t *p_ss_cache;
    // proc_action == pa
    json_cache_t *p_pa_cache;


    uploader *p_upload_impl;

private:
};

} /* namespace edr */
