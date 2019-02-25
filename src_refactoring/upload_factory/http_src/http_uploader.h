#ifndef HTTP_UPLOADER_HHH
#define HTTP_UPLOADER_HHH

#include "uploader_factory.h"
#include "upload_report.h"

/* product class */
class http_uploader:public uploader
{
public:
    http_uploader();
    ~http_uploader();
    virtual void set_upload_cb(state_cb_t state_cb, 
                               active_cb_t active_cb, void *p_active_param);
    
    virtual void run_upload(void);
    virtual void stop_upload(void);

    virtual int common_upload(upload_info_t *p_info, void *p_param);

private:
    upload_handler_t *p_upload;
};


#endif /* HTTP_UPLOADER_HHH */
