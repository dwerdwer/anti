#pragma once
#ifndef _MODULE_DEF_H
#define _MODULE_DEF_H
#include <time.h>
#include "module_interfaces.h"

typedef enum
{
	LOGIN_NULL = 0,
	LOGINING,
	LOGINED,
}login_status_t;

struct module{
	std::vector<module_data_t*>     m_data_vct;
	void*                           p_params;
	notify_scheduler_t              p_notifier;
	Mutex                           m_mutex;
	login_status_t					login_status;
	bool 							stop_flag;
	uint32_t						category;
	uint32_t 						m_token;
	time_t 							last_heart_time;
	uint32_t						elapsed_time;
	uint32_t 						run_mode; // 0, 正常， 1 测试

	//登录 注册信息
	std::string 					host;
	std::string 					node_id;
	int 							product_id;
	int 							product_os;
	int 							product_runmode;
	std::string 					product_type;
	std::string 					product_version;
	std::string						product_version2;
	std::string 					sysversion;
	std::string						machine_name;
	std::string 					ip_addrs;  //16进制转字符串
	std::string 					mac_addr;

	//heart_beat data
	int								node_status;
	int 							cur_task_id;
	int 							task_result;

	//for test
	int 							login_count;
	clock_t 						start_first_login_time;
	long	 						first_login_time;
	clock_t 						start_second_login_time;
	long 							second_login_time;
	clock_t 						start_reg_time;
	long	 						reg_time;
    int                             rand_size;
};

#endif
