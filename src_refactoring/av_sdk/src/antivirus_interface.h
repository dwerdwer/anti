
#ifndef _AV_INTERFACE_
#define _AV_INTERFACE_

#include <stdint.h>
#include <string>

//typedef uint32_t (*ScanNotify)(const char * file,uint32_t flag,uint64_t record,const char * description, void *ptr_param);
//typedef uint32_t (*ListNotify)(const char * file_old, const char * file_new,const char * sha,uint64_t size,void *ptr_param);
typedef uint32_t (*DataNotify)(const char * data,uint32_t length,int32_t flag,void *ptr_param);

enum EFlags {
		Unzip		    = 0x01,
		Unpack		  = 0x02,
		StopOnOne	  = 0x10,
		ProgramOnly = 0x100,
		OriginalMd5 = 0x1000,
		UseFigner	  = 0x2000,
		UseCloud	  = 0x4000,
		Backup		  = 0x8000,
		ForceUnzip	= 0x100000
};

extern "C" {

  void * av_sdk_init();
  void av_sdk_uninit(void * ptr_sdk);
  uint32_t av_add_target(void * ptr_sdk,const char * ptr_path);
  uint32_t av_sdk_stop(void * ptr_sdk);
  uint32_t av_sdk_pause(void * ptr_sdk);
  uint32_t av_sdk_resume(void * ptr_sdk);
  uint32_t av_list_isolation(void * ptr_sdk,DataNotify ptr_notify,void * ptr_data);
  uint32_t av_restore_isolation(void * ptr_sdk,const char * ptr_name);
  uint32_t av_scan_target(void * ptr_sdk,uint32_t uint32_option,const char * ptr_path,DataNotify ptr_notify,void * ptr_data);
  uint32_t av_get_path(void *ptr_sdk,DataNotify ptr_notify,void * ptr_data);
}

#endif

