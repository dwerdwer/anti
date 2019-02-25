
#include "http_uploader.h"

http_uploader::http_uploader()
{
    this->p_upload = init_upload_handler();
}

http_uploader::~http_uploader()
{
    release_upload_handler(this->p_upload);
}

void http_uploader::set_upload_cb(state_cb_t state_cb, 
                                  active_cb_t active_cb, void *p_active_param)
{
    priv_upload_cb(this->p_upload, state_cb, active_cb, p_active_param);
}
    
void http_uploader::run_upload(void)
{
    priv_run_upload(this->p_upload);
}

void http_uploader::stop_upload(void)
{
    priv_stop_upload(this->p_upload);
}

int http_uploader::common_upload(upload_info_t *p_info, void *p_param)
{
    return priv_common_upload(this->p_upload, p_info, p_param);
}

