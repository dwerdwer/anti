#ifndef UPLOAD_REPORT_HHH
#define UPLOAD_REPORT_HHH

#include <stdint.h>
#include <stdbool.h>
#include "uploader_factory.h"

typedef struct upload_handler upload_handler_t;

/*return:   succ p_upload     error NULL */
upload_handler_t *init_upload_handler();

void priv_upload_cb(upload_handler_t *p_upload, state_cb_t state_cb, 
                    active_cb_t active_cb, void *p_active_param);

/* If success function is blocked */
void priv_run_upload(upload_handler_t *p_upload);

void priv_stop_upload(upload_handler_t *p_upload);

/* p_param for state_cb_t
 * return:   succ 0     error -1    fail http code */
int priv_common_upload(upload_handler_t *p_upload, upload_info_t *p_info, void *p_param);

void release_upload_handler(upload_handler_t *p_upload);

#endif
