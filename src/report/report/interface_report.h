#ifndef _report_
#define _report_
#include <stdint.h>
#include "module_data.h"
#include "module_interfaces.h"


#ifdef __cplusplus
extern "C"
{
#endif
/*
module_t * create(uint32_t category, notify_scheduler_t notifier, void *p_params, uint32_t arg_count, const char **p_args);

void destroy(module_t *p_module);

module_state_t run(module_t *p_module);

module_state_t stop(module_t *p_module);

module_data_t* assign(module_t *p_module, const module_data_t *p_data,bool is_sync);

void get_inputted_message_type(module_t *p_module, const char *** const ppp_inputted_message_types, uint32_t *p_message_type_count);
*/
/*
module_t * create_file_reporter(uint32_t category, notify_scheduler_t notifier, void *p_params, uint32_t arg_count, const char **p_args);

void destroy_file_reporter(module_t *p_module);

module_state_t run_file_reporter(module_t *p_module);

module_state_t stop_file_reporter(module_t *p_module);

module_data_t* assign_file_reporter(module_t *p_module, const module_data_t *p_data,bool is_sync);
 */





int UploadBuff(const char* pServerUrl,const char* pFileName, const char* pDataBuff, uint64_t uDataSize, uint32_t iTimeOut);

int UploadBuffAsZip(const char* pServerUrl,const char* pFileName, const char* pZipName, const char* pDataBuff, uint64_t uDataSize, uint32_t iTimeOut);

int UploadFile(const char* pServerUrl, const char* pFilePath, uint32_t iTimeOut);

int UploadSyslog(const char* pServerUrl, const char* pLogData, uint64_t uDataSize, uint32_t iTimeOut);

#ifdef __cplusplus
}
#endif

#endif
