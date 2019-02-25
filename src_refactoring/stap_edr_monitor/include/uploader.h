#pragma once

#include <string>
#include <inttypes.h>

#include "edition.h"
#include "upload_struct.h"
#include "util-lock.h"
#include "sysinfo_interface.h"

namespace edr {

struct UploaderConf
{
    /* common*/
    module_info_t *p_m_info;

    /* Huawei */
    std::string log_path;
    std::string ip;
    uint16_t port;
    std::string user;
    std::string passwd;

    /* EDR */

};


class BaseUploader
{
public:
    virtual ~BaseUploader() = 0;
    virtual int run() = 0;
    virtual void stop() = 0;
    virtual void flush() = 0;
    virtual int push_task(task_record *p_data) = 0;
    virtual int push_task(json_file *p_data) = 0;
    virtual bool isstop() = 0;

    virtual void set_temp_path(const char *p_path);
    virtual void set_url(const char *p_host, uint16_t port);
    virtual void set_url(const char *p_token);
    virtual bool is_srv_valid();

    virtual std::string get_path();
    virtual std::string get_host();
    virtual std::string get_token();
    virtual uint16_t get_port();
protected:
    std::string path;
    std::string host;
    std::string token;
    uint16_t port;
    util::rwlock_t lock;
};

class UploaderFactory
{
public:
    edr::BaseUploader *create_uploader(edr::edition_t edition, UploaderConf *conf);
};

} /* namespace edr */
