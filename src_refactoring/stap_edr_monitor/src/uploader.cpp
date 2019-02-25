
#include "uploader.h"
#include "uploader_edr.h"
#include "uploader_huawei.h"

void edr::BaseUploader::set_temp_path(const char *p_path)
{
    if (p_path)
    {
        this->lock.write();
        this->path = p_path;
        this->lock.unlock();
    }
}

void edr::BaseUploader::set_url(const char *p_host, uint16_t port)
{
    if (p_host && port)
    {
        this->lock.write();
        this->host = p_host;
        this->port = port;
        this->lock.unlock();
    }
}

void edr::BaseUploader::set_url(const char *p_token)
{
    if (p_token)
    {
        this->lock.write();
        this->token = p_token;
        this->lock.unlock();
    }
}

bool edr::BaseUploader::is_srv_valid()
{
    this->lock.read();
    bool ret = this->host.size() >= 7 && this->port;
    this->lock.unlock();
    return ret;
}

std::string edr::BaseUploader::get_path()
{
    this->lock.read();
    std::string ret(this->path);
    this->lock.unlock();

    return ret;
}

std::string edr::BaseUploader::get_host()
{
    this->lock.read();
    std::string ret(this->host);
    this->lock.unlock();

    return ret;
}

std::string edr::BaseUploader::get_token()
{
    this->lock.read();
    std::string ret(this->token);
    this->lock.unlock();

    return ret;
}

uint16_t edr::BaseUploader::get_port()
{
    this->lock.read();
    uint16_t port = this->port;
    this->lock.unlock();

    return port;
}

edr::BaseUploader *edr::UploaderFactory::create_uploader(edr::edition_t edition, UploaderConf *conf)
{
    switch (edition)
    {
        case edr::edition_t::EDITION_EDR:
            return new edr::EDRUploader(conf);
        case edr::edition_t::EDITION_HUAWEI:
            return new edr::HuaweiUploader(conf);
        case edr::edition_t::EDITION_NO:
            return NULL;
    }
    return NULL;
}
