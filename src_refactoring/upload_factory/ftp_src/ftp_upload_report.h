#ifndef FTP_UPLOAD_REPORT_HHH
#define FTP_UPLOAD_REPORT_HHH

#include <stdint.h>
#include <stdbool.h>
#include "uploader_factory.h"

typedef struct ftp_upload_handler ftp_upload_handler_t;

/*return:   succ p_upload     error NULL */
ftp_upload_handler_t *ftp_init_upload_handler();

void ftp_priv_upload_cb(ftp_upload_handler_t *p_upload, state_cb_t state_cb, 
                    active_cb_t active_cb, void *p_active_param);

/* If success function is blocked */
void ftp_priv_run_upload(ftp_upload_handler_t *p_upload);

void ftp_priv_stop_upload(ftp_upload_handler_t *p_upload);

/* p_param for state_cb_t
 * return:   succ 0     error -1    fail http code */
int ftp_priv_common_upload(ftp_upload_handler_t *p_upload, upload_info_t *p_info, void *p_param);

void ftp_release_upload_handler(ftp_upload_handler_t *p_upload);

#endif
