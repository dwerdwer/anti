#ifndef FTP_UPLOADER_HHH
#define FTP_UPLOADER_HHH

#include "uploader_factory.h"
#include "ftp_upload_report.h"

/* product class */
class ftp_uploader:public uploader
{
public:
    ftp_uploader();
    ~ftp_uploader();
    virtual void set_upload_cb(state_cb_t state_cb, 
                               active_cb_t active_cb, void *p_active_param);
    
    virtual void run_upload(void);
    virtual void stop_upload(void);

    virtual int common_upload(upload_info_t *p_info, void *p_param);

private:
    ftp_upload_handler_t *p_upload;
};


#endif /* FTP_UPLOADER_HHH */
