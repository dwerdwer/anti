
#include "uploader_factory.h"
#include "http_uploader.h"
#include "ftp_uploader.h"

using namespace std;

upload_info_t::upload_info_t()
{
   this->type       = (content_type_t)0;
   this->timeout    = 0;
   this->buf        = NULL;
   this->buf_size   = 0;
   this->iszip      = false;
}

upload_info_t::~upload_info_t() {  }

uploader::~uploader() {  }

uploader* uploader_factory::create_uploader(uploader_type_t type)
{
    uploader *p_uploader = NULL;

    switch (type)
    {
    case HTTP_UPLOADER:
        p_uploader = new http_uploader;
        break;
    case FTP_UPLOADER:
        p_uploader = new ftp_uploader;
        break;
    default:
        break;
    }
    return p_uploader;
}
