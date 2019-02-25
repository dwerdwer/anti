
#ifndef UPLOADER_FACTORY_HHH
#define UPLOADER_FACTORY_HHH

#include <stdint.h>
#include <string>

#ifndef OUT
#define OUT
#endif

#ifndef IN
#define IN
#endif

typedef enum 
{
    UPLOAD_TYPE_BUF = 1,
    UPLOAD_TYPE_FILE,

}content_type_t;

struct upload_info_t
{
    upload_info_t();
    ~upload_info_t();
    
    content_type_t type;
    std::string url;
    std::string user;
    std::string passwd;
    uint32_t timeout;       // Unit second
    const char *buf;
    uint64_t buf_size;

    /*for different content type*/
    std::string name;

    bool iszip;
    std::string zip; 
};

typedef void (*state_cb_t)(bool succ, IN upload_info_t *p_upload, IN void *p_param);

/* active reading of external data */
typedef void (*active_cb_t)(IN void *p_active_param, 
                            OUT upload_info_t **pp_info, OUT void **pp_param);

/* abstract product class */
class uploader
{
public:
    virtual void set_upload_cb(state_cb_t state_cb, 
                               active_cb_t active_cb, void *p_active_param) = 0;
    /* If success function is blocked */
    virtual void run_upload(void) = 0;
    virtual void stop_upload(void) = 0;

    /* p_param for state_cb_t
    * return:   succ 0     error -1 */
    virtual int common_upload(upload_info_t *p_info, void *p_param) = 0;

    virtual ~uploader() = 0;
};

typedef enum
{
    HTTP_UPLOADER = 0,
    FTP_UPLOADER
}uploader_type_t;

/* factory class */
class uploader_factory
{
public:
    uploader* create_uploader(uploader_type_t type);
};

#endif /* UPLOAD_FACTORY_HHH */

