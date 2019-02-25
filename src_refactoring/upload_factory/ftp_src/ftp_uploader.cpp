
#include "ftp_uploader.h"

ftp_uploader::ftp_uploader()
{
    this->p_upload = ftp_init_upload_handler();
}

ftp_uploader::~ftp_uploader()
{
    ftp_release_upload_handler(this->p_upload);
}

void ftp_uploader::set_upload_cb(state_cb_t state_cb, 
                                  active_cb_t active_cb, void *p_active_param)
{
    ftp_priv_upload_cb(this->p_upload, state_cb, active_cb, p_active_param);
}
    
void ftp_uploader::run_upload(void)
{
    ftp_priv_run_upload(this->p_upload);
}

void ftp_uploader::stop_upload(void)
{
    ftp_priv_stop_upload(this->p_upload);
}

int ftp_uploader::common_upload(upload_info_t *p_info, void *p_param)
{
    return ftp_priv_common_upload(this->p_upload, p_info, p_param);
}

