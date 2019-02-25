#include "interface_report.h"
#include "utils/utils_library.h"
#include "module_data.h"
//#include "Worker.h"
#include "Tools.h"
#include "upload_utils.h"
#include <string>
#include <cstring>
struct module
{	
	uint32_t uCategory;
//	Worker * pWorker;
	notify_scheduler_t pNotify;
	void * pParams;
	char *pTag[1];
	std::string url_str;
	
};
#include "report_log.h"

module_t *g_p_module;  // for EDR interface log

/*
LIB_PUBLIC module_t * create(uint32_t category, notify_scheduler_t notifier, void *p_params, uint32_t arg_count, const char **p_args){
	module * pModule = new module;
	if (pModule){
		pModule->uCategory = category;
		pModule->pWorker = new Worker();
		pModule->pNotify = notifier;
		pModule->pParams = p_params;
		if (arg_count > 0)
			pModule->url_str = p_args[0];
		g_p_module = pModule;

		pModule->pTag[0] = new char[32];
		memset(pModule->pTag[0], 0, 32);
		strcpy(pModule->pTag[0], "ScanVirus");

		return pModule;
	}
	return NULL;
}

LIB_PUBLIC void destroy(module_t *p_module){
	if (p_module){
		module * pModule = (module *)p_module;
		delete pModule->pWorker;
		pModule->pWorker = NULL;
		delete pModule;
		pModule = NULL;
	}
}

LIB_PUBLIC module_state_t run(module_t *p_module){
	module_state_t tmpState = MODULE_FATAL;
	if (p_module){
		module * pModule = (module *)p_module;
		pModule->pWorker->beginWrok();
		tmpState = MODULE_OK;
	}
	return tmpState;
}

LIB_PUBLIC module_state_t stop(module_t *p_module){
	module_state_t tmpState = MODULE_FATAL;
	if (p_module){
		module * pModule = (module *)p_module;
		pModule->pWorker->endWork();
		tmpState = MODULE_OK;
	}
	return tmpState;
}

LIB_PUBLIC module_data_t* assign(module_t *p_module, const module_data_t *p_data,bool is_sync){
	printf(">>> Report recv data from other module......\n");
	if (p_data && p_module){
		module * pModule = (module *)p_module;
		const char * pBuf = NULL;
		uint32_t nSize = 0;
		int32_t nFlag = get_module_data_property(p_data, "REPORTER_MESSAGE_DATA",&pBuf,&nSize);
		if (nFlag != -1){
			//收到的数据是protorbuf数据，先转换成base64数据，再添加到发送列表，缓存起来
			//sqlite返回数据时，没有给出缓冲区的长度，所以先编码成base64
			string datas = Tools::Encode((const unsigned char *)pBuf,nSize);
//			pModule->pWorker->doInsert(1,"http://192.168.10.80:8001/msgmgr/message/",datas.c_str(),100000,1,"");
			if (!p_module->url_str.empty())
				pModule->pWorker->doInsert(1, p_module->url_str.c_str(), datas.c_str(),100000,1,"");
			else
				report_log(p_module,"worker can not run because server_url is empty\n");
		}else{
			printf("\n Flag iis : %d\n", nFlag);
		}
	}
	else{
		printf("\n data is null\n");
	}

	if (is_sync){
		module_data_t * data = create_module_data();
		int status = 1;
		set_module_data_property(data,g_p_message_id, (const char*)&status, sizeof(status));
		return data;
	}
	else{
		return NULL;
	}
}

LIB_PUBLIC void get_inputted_message_type(module_t *p_module, const char *** const ppp_inputted_message_types, uint32_t *p_message_type_count){
	if (p_module) {
		module *pModule = (module *) p_module;
		*ppp_inputted_message_types = (const char **) pModule->pTag;
		*p_message_type_count = 1;
	}
}
*/
LIB_PUBLIC int UploadBuff(const char* pServerUrl,const char* pFileName, const char* pDataBuff, uint64_t uDataSize, uint32_t iTimeOut){
	int nSuccess = -1;
	if (pServerUrl && pFileName && pDataBuff){
		return UploadUtils::UploadBuffer(pServerUrl,pDataBuff,uDataSize,pFileName,iTimeOut);
	}
	return nSuccess;
}

LIB_PUBLIC int UploadBuffAsZip(const char* pServerUrl,const char* pFileName, const char* pZipName, const char* pDataBuff, uint64_t uDataSize, uint32_t iTimeOut){

	int nSuccess = -1;
	if (pServerUrl && pFileName && pDataBuff){
		nSuccess = UploadUtils::UploadBufferAsZip(pServerUrl,pDataBuff,uDataSize,pFileName,pZipName,iTimeOut);
		//return UploadUtils::UploadBuffer(pServerUrl,pDataBuff,uDataSize,pFileName,iTimeOut);
	}
	report_log(g_p_module,"uploadBuffAsZip url:%s filename:%s zipname:%s retcode:%d", pServerUrl, pFileName, pZipName, nSuccess);
	return nSuccess;

}

LIB_PUBLIC int UploadFile(const char* pServerUrl, const char* pFilePath, uint32_t iTimeOut){

	int nSuccess = -1;
	if (pServerUrl && pFilePath){
		return UploadUtils::UploadFile(pServerUrl,pFilePath,iTimeOut);
	}
	return nSuccess;
}

LIB_PUBLIC int UploadSyslog(const char* pServerUrl, const char* pLogData, uint64_t uDataSize, uint32_t iTimeOut)
{
	int nSuccess = -1;
	if (pServerUrl && pLogData) 
	{

		nSuccess = UploadUtils::UploadSysLog(pServerUrl, pLogData, uDataSize, iTimeOut);
		
	}
	report_log(g_p_module, "uploadSyslog url:%s datasize is %d retcode:%d\n", pServerUrl, uDataSize, nSuccess);
	return nSuccess;
	
}

