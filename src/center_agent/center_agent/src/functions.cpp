#include <iostream>
#include <sys/syscall.h>
#include <vector>
#include <string.h>
#include <string>
#include <unistd.h>
#include <algorithm>
#include "Mutex.h"
#include "module_data.h"
#include "module_interfaces.h"
//#include "module_message_defines.h"
#include "module_def.h"
#include "functions.h"
#include "NetUtils.h"
#include "IKVMessage.h"
#include "KVMessage.h"
#include "if_info.h"
#include <errno.h>
#include "center_log.h"
#include "if_info.h"
#include "http_req.h"


using namespace std;
//static pid_t get_thread_id()
//{
//	return syscall(SYS_gettid);
//}

// n_flag : 是否成功， 1 为 成功， 0 失败

/*
static void req_proc(int n_flag, char* p_buf, int n_length, int n_target, void* p_user_data)
{
    module_t* p_center_agent = reinterpret_cast<module_t*>(p_user_data);
    if (n_target == 1000)
    {
        int login_ret = on_login(p_center_agent, n_flag, p_buf, n_length);

		if (login_ret == ERR_NOT_REGISTER)
		{
			// reg
			center_reg(p_center_agent, NULL);
		}
		else if (login_ret == ERR_OK)
		{
            printf("##login ok##\n");
			//success  TODO:
		}
		else  //其他情况 重复登陆
		{
			sleep(2);
			printf("##### login error %d #####\n", login_ret);
			center_login(p_center_agent, NULL);
		}

	}
	else if (n_target == 1001) {
		int reg_ret = on_reg(p_center_agent, n_flag, p_buf, n_length);
		if (reg_ret == ERR_OK || reg_ret == ERR_NEED_LOGIN)
		{
			center_log(p_center_agent, "register success, ret_code is %d\n", reg_ret);
			center_login(p_center_agent, NULL);
		}
		else //注册接口 其他错误码， 暂不处理, 提示用户
		{
			printf("register err, errcode is %d\n", reg_ret);
			center_log(p_center_agent, "register err, errcode is %d\n", reg_ret);
			//printf()
		}
	}
	else if (n_target == 1002) {
		//TODO: Judge return value
		//int heart_ret =
		int ret = on_heart_beat(p_center_agent, n_flag, p_buf, n_length);
		if (ret == ERR_NEED_LOGIN)
		{
			center_log(p_center_agent,"heart beat ret need login \n");
			center_login(p_center_agent, NULL);
		}
		else if (ret == -1)
		{
			center_log(p_center_agent,"heart beat err %d  start reconnect \n", ret);
			center_login(p_center_agent, NULL);
		}
	}
	else if (n_target == 1003) {
		//int logout_ret =
		on_logout(p_center_agent, n_flag, p_buf, n_length);
	}
}
*/

int login(module_t* p_module)
{
//    printf("login start\n");
    int login_ret = center_login(p_module, NULL);
	if (login_ret == ERR_NOT_REGISTER)
	{
		// reg
		int reg_ret = center_reg(p_module, NULL);
		if (reg_ret == ERR_OK || reg_ret == ERR_NEED_LOGIN)
		{
			center_log(p_module, "register success, ret_code is %d\n", reg_ret);
			int ret = center_login(p_module, NULL);
            if (ret == ERR_OK)
                return 0;
            else
                return ret;
		}
		else //注册接口 其他错误码， 暂不处理, 提示用户
		{
			printf("register err, errcode is %d\n", reg_ret);
			center_log(p_module, "register err, errcode is %d\n", reg_ret);
        }

	}
	else if (login_ret == ERR_OK)
	{
        return 0;
		//success  TODO:
	}
	else  //其他情况 重复登陆
	{
    	return -1;
    }
}

int center_login(module_t *p_module, module_data_t *p_module_data)
{
	//packet protobuf
	IRequestMessage* msg = NewInstance_IRequestMessage(CMD_LOGIN, SUBCMD_LOGIN);
	msg->Add_IntValue(KEY_PRODUCTID, p_module->product_id);
	msg->Add_IntValue(KEY_PRODUCTOS, p_module->product_os);
	msg->Add_StringValue(KEY_PRODUCTTYPE, p_module->product_type.c_str());
	msg->Add_StringValue(KEY_PRODUCTVERSION, p_module->product_version.c_str());
	msg->Add_StringValue(KEY_PRODUCTVERSION2, p_module->product_version2.c_str());
	msg->Add_StringValue(KEY_NODEID, p_module->node_id.c_str());
	msg->Add_StringValue(KEY_MACHINENAME, p_module->machine_name.c_str());

	// 获取本地ip 及 所有网卡信息
	unsigned long local_ip = 0;
	bool b_get_ip_success;
	std::string ser_host;
	unsigned int port = 0;
	url2domain(p_module->host.c_str(), ser_host, port);
	do{
		b_get_ip_success = get_local_ip(p_module, ser_host.c_str(), port, &local_ip);
		if (!b_get_ip_success)
		{
			center_log(p_module,"center_login get local ip host: %s  port:%d err:%d", ser_host.c_str(), port, errno);
			printf("center_login get_local_ip err is %d\n", errno);
			sleep(5);
		}
	} while(!b_get_ip_success);

	center_log(p_module,"center_login get local ip  success host: %s  port:%d", ser_host.c_str(), port);
	std::string interfaces = generate_json(local_ip);
	msg->Add_StringValue(KEY_INTERFACE, interfaces.c_str());
//	msg->Add_StringValue(KEY_IPADDR, p_module->ip_addrs.c_str());

//	msg->Dump();

	//packet protobuf end

	std::string encode_data = msg->Encode();

	center_log(p_module, "login url:%s msg:%s \n ",p_module->host.c_str(), msg->to_String().c_str());

	if (p_module->login_count == 0)
	{
		p_module->login_count = 1;
		p_module->start_first_login_time = clock();
	}
	else
	{
		p_module->login_count = 2;
		p_module->start_second_login_time = clock();
	}

	delete msg;
    //doReq(p_module->host.c_str(), p_module->node_id.c_str(),encode_data.c_str(),encode_data.length(), 1000, req_proc, p_module);
    char* ret_buf = NULL;
    uint32_t ret_size = 0;
    int res = do_request(p_module->host.c_str(), encode_data.c_str(), encode_data.length(),&ret_buf, &ret_size);
    if (res == CURLE_OK){
        res = on_login(p_module, ret_buf, ret_size);
        release_buf(ret_buf);
        ret_buf = NULL;
    }
	return res;
}



int center_reg(module_t *p_module, module_data_t *p_module_data)
{
	//packet protobuf
	IRequestMessage* msg = NewInstance_IRequestMessage(CMD_LOGIN, SUBCMD_REGISTER);
	msg->Add_IntValue(KEY_PRODUCTID, p_module->product_id);
	msg->Add_IntValue(KEY_PRODUCTOS, p_module->product_os);
	msg->Add_IntValue(KEY_PRODUCTRUNMODE, p_module->product_runmode);
	msg->Add_StringValue(KEY_PRODUCTTYPE, p_module->product_type.c_str());
	msg->Add_StringValue(KEY_PRODUCTVERSION, p_module->product_version.c_str());
	msg->Add_StringValue(KEY_PRODUCTVERSION2, p_module->product_version2.c_str());
	msg->Add_StringValue(KEY_OSVERSION, p_module->sysversion.c_str());
    
    std::transform(p_module->sysversion.begin(), p_module->sysversion.end(),p_module->sysversion.begin(),[](unsigned char c){ return std::tolower(c);});

	if (strstr(p_module->sysversion.c_str(), "rhel"))
		msg->Add_IntValue(KEY_OSNAME, 100);
	else if (strstr(p_module->sysversion.c_str(), "fedora"))
		msg->Add_IntValue(KEY_OSNAME, 101);
	else if (strstr(p_module->sysversion.c_str(), "mandriva"))
		msg->Add_IntValue(KEY_OSNAME, 102);
	else if (strstr(p_module->sysversion.c_str(), "suse"))
		msg->Add_IntValue(KEY_OSNAME, 103);
	else if (strstr(p_module->sysversion.c_str(), "centos"))
		msg->Add_IntValue(KEY_OSNAME, 104);
	else if (strstr(p_module->sysversion.c_str(),"ubuntu"))
		msg->Add_IntValue(KEY_OSNAME, 105);
	else if (strstr(p_module->sysversion.c_str(), "debian"))
		msg->Add_IntValue(KEY_OSNAME, 106);
    else if (strstr(p_module->sysversion.c_str(), "mint"))
        msg->Add_IntValue(KEY_OSNAME, 107);
    else if (strstr(p_module->sysversion.c_str(), "pclinux"))
        msg->Add_IntValue(KEY_OSNAME, 108);
    else if (strstr(p_module->sysversion.c_str(), "solaris"))
        msg->Add_IntValue(KEY_OSNAME, 109);
    else if (strstr(p_module->sysversion.c_str(), "kylin"))
        msg->Add_IntValue(KEY_OSNAME, 110);
    else 
        msg->Add_IntValue(KEY_OSNAME, 149);

	msg->Add_StringValue(KEY_NODEID, p_module->node_id.c_str());
	msg->Add_StringValue(KEY_MACHINENAME, p_module->machine_name.c_str());
	msg->Add_StringValue(KEY_GROUPNAME, "linux");   //linux system use ‘linux’

	// 获取本地ip 及 所有网卡信息
	unsigned long local_ip = 0;
	bool b_get_ip_success;
	std::string ser_host;
	unsigned int port = 80; //默认 80端口
	url2domain(p_module->host.c_str(), ser_host, port);
	do{
		b_get_ip_success = get_local_ip(p_module, ser_host.c_str(), port, &local_ip);
		if (!b_get_ip_success)
		{
			center_log(p_module," center_register get local ip host: %s  port:%d err:%d", ser_host.c_str(), port, errno);
			printf("center_register get_local_ip err is %d\n", errno);
			sleep(5);
		}
	} while(!b_get_ip_success);
	center_log(p_module,"center_register get local ip  success host: %s  port:%d", ser_host.c_str(), port);
	std::string interfaces = generate_json(local_ip);

	msg->Add_StringValue(KEY_INTERFACE, interfaces.c_str());
#ifdef __x86_64__
	msg->Add_IntValue(KEY_PRODUCTBITS, 0);
#elif __i386__
	msg->Add_IntValue(KEY_PRODUCTBITS, 1);
#endif

//	msg->Dump();
	//packet protobuf end

	std::string encode_data = msg->Encode();
	center_log(p_module, "register url:%s msg:%s\n ",p_module->host.c_str(), msg->to_String().c_str());
	delete msg;

	if (p_module->start_reg_time == 0)
		p_module->start_reg_time = clock();

	// TODO: send register request to server
//	doReq(p_module->host.c_str(), p_module->node_id.c_str(), encode_data.c_str(),encode_data.length(), 1001, req_proc, p_module);

    char* ret_buf = NULL;
    uint32_t ret_size = 0;
    int res = do_request(p_module->host.c_str(), encode_data.c_str(), encode_data.length(),&ret_buf, &ret_size);
    if (res == CURLE_OK){
        res = on_reg(p_module, ret_buf, ret_size);
        release_buf(ret_buf);
        ret_buf = NULL;
    }
	// response error  need login
	return res;
}

static uint64_t req_count = 0;

int center_report(module_t* p_module)
{
    std::string data =  generate_report_json();

    IRequestMessage *msg = NewInstance_IRequestMessage(CMD_COMMON_REPORT, SUBCMD_COMMON_REPORT);
    msg->Add_StringValue(KEY_REPORT_TYPE, "VirusLog");
    msg->Add_StringValue(KEY_REPORT_ACTION, "findvirus");
    msg->Add_StringValue(KEY_REPORT_INFO, data.c_str());
	
	std::string encode_data = msg->Encode();
    // msg->Dump();
    
    center_log(p_module, "report url:%s msg:%s\n ",p_module->host.c_str(), msg->to_String().c_str());
	delete msg;
	//doReq(p_module->host.c_str(), p_module->node_id.c_str(), encode_data.c_str(), encode_data.length(), 1002, req_proc, p_module);
    char* ret_buf = NULL;
    uint32_t ret_size = 0;
    int res = do_request(p_module->host.c_str(), encode_data.c_str(), encode_data.length(),&ret_buf, &ret_size);
    __sync_fetch_and_add(&req_count, 1);

    if (res == CURLE_OK){
    
        res = on_report(p_module, ret_buf, ret_size);
        release_buf(ret_buf);
    }
    else
    {
        printf("report err!!!, %d", res);
    
    }


}

int center_heart_beat(module_t* p_module, module_data_t *p_module_data)
{
	IRequestMessage *msg = NewInstance_IRequestMessage(CMD_HEART_BEAT);
	msg->Add_IntValue(KEY_RUNSTATUS, p_module->node_status);
	msg->Add_IntValue(KEY_TASKID, p_module->cur_task_id);
	msg->Add_IntValue(KEY_TASKRESULT, p_module->task_result);
	// 是否要加 token
	if (p_module->run_mode == 1)
	{
		RequestMsg* p = (RequestMsg*)msg;
		p->set_token(p_module->node_id);
	}

	//msg->Dump();
	std::string encode_data = msg->Encode();

	// send heart_beat request to server

	center_log(p_module, "heart_beat url:%s msg:%s\n ",p_module->host.c_str(), msg->to_String().c_str());
	delete msg;
	//doReq(p_module->host.c_str(), p_module->node_id.c_str(), encode_data.c_str(), encode_data.length(), 1002, req_proc, p_module);
    char* ret_buf = NULL;
    uint32_t ret_size = 0;
    int res = do_request(p_module->host.c_str(), encode_data.c_str(), encode_data.length(),&ret_buf, &ret_size);
    if (res == CURLE_OK){
        res = on_heart_beat(p_module,ret_buf, ret_size);
        release_buf(ret_buf);
        ret_buf = NULL;
    }
    else 
    {
        printf("heart_beat err!!!\n");
    }

	return res;
}

int on_report(module_t* p_module, char* p_buf, int n_length)
{
    static uint64_t report_success = 0;
    static uint64_t report_failed = 0;
    
    if (p_buf != NULL)
    {
        int size = 0;
        IResponseMessage* msg = NewInstance_IResponseMessage((const unsigned char*)p_buf, n_length, &size);
        if (msg != NULL)
        {
            // msg->Dump();
			center_log(p_module, "report response msg:%s", msg->to_String().c_str());
			int err_code = msg->Get_Error();
            if (err_code == 0) {
            
                __sync_fetch_and_add(&report_success, 1);
				module_data_t* p_data = create_module_data();
				std::string message_id = "report";
				set_module_data_property(p_data, g_p_message_id, message_id.c_str(), message_id.length());
				
				set_module_data_property(p_data, "REQ_COUNT", (const char*)&req_count, sizeof(req_count));
                set_module_data_property(p_data, "SUCCESS_COUNT", (const char*)&report_success, sizeof(report_success));

				module_message_t module_msg;
				module_msg.category = static_cast<module_category_t>(p_module->category);
				module_msg.p_data = p_data;

				mdh_sync_params_t sync_param;
				sync_param.is_sync = false;
				p_module->p_notifier(&module_msg, p_module->p_params, &sync_param);

			//	printf("#####send report to edr#####\n");

				destroy_module_data(p_data);


            }    

            delete msg;
            return err_code;
            
        }
    }
    else
    {
        printf("report !!!2");
    }
    return -1;
}

int on_login(module_t* p_module, char* p_buf, int n_length)
{
	if (p_buf != NULL)
	{
		int size = 0;
		IResponseMessage* msg = NewInstance_IResponseMessage((const unsigned char*)p_buf, n_length, &size);
		if (msg != NULL)
		{
			if (p_module->login_count == 1)
			{
				p_module->first_login_time = clock() - p_module->start_first_login_time;
			}
			else
			{
				p_module->second_login_time = clock() - p_module->start_second_login_time;
			}
//			msg->Dump();
			int err_code = msg->Get_Error();
			int elapse = msg->Get_Info();
			if (err_code == ERR_OK)
			{
                center_log(p_module, "login response %s\n", msg->to_String().c_str());
				center_log(p_module, "login success %d\n", err_code);
				Set_Token(p_module->node_id.c_str());
				p_module->login_status = LOGINED;
				p_module->elapsed_time = elapse;

				// send token

				module_data_t* p_data = create_module_data();
				std::string message_id = "receive_token";
				set_module_data_property(p_data, g_p_message_id, message_id.c_str(),message_id.length());
				std::string token = p_module->node_id;
				set_module_data_property(p_data, "TOKEN", (const char*)token.c_str(), token.length());

				module_message_t module_msg;
				module_msg.category = static_cast<module_category_t>(p_module->category);
				module_msg.p_data = p_data;

				mdh_sync_params_t sync_param;
				sync_param.is_sync = false;
//				printf("send token %s to edr", token.c_str());
				center_log(p_module, "send token %s to edr ", token.c_str());
				p_module->p_notifier(&module_msg, p_module->p_params, &sync_param);

//				printf("#####send token to edr#####\n");

				destroy_module_data(p_data);
			}
			else
			{
				center_log(p_module, "login err ret is %d\n", err_code);
			}
			delete msg;
			return err_code;
		}
		else
		{
			center_log(p_module, "login ret msg is NULL\n");
		}
		return -1;
	}
	return -1;
}



int on_reg(module_t* p_module, char* p_buf, int n_length)
{
	if (p_buf != NULL)
	{
		int size;
		IResponseMessage* msg = NewInstance_IResponseMessage((const unsigned char*)p_buf, n_length, &size);

		if (msg != NULL)
		{
			p_module->reg_time = clock() - p_module->start_reg_time;
//			msg->Dump();
			int err_code = msg->Get_Error();
	        center_log(p_module, "register ret code %d\n", err_code);
			delete msg;
			return err_code;
		}
		else
		{
			center_log(p_module, "register ret msg is NULL\n");
            return -1;
		}
	}
	else
	{
		return -1; //可能为http code
	}
}

static const module_data_t **sync_alloc(uint32_t input_count)
{
    const module_data_t **pp_restult = NULL;

    pp_restult = new const module_data_t* [input_count];

    if (NULL == pp_restult)
        return NULL;

    return pp_restult;
}

int on_heart_beat(module_t* p_module, char* p_buf, int n_length)
{
	if (p_buf != NULL)
	{
		int size;
		IResponseMessage* msg = NewInstance_IResponseMessage((const unsigned char*)p_buf, n_length, &size);

		if (msg != NULL)
		{
//			msg->Dump();
			center_log(p_module, "heart beat response msg:%s", msg->to_String().c_str());
			int err_code = msg->Get_Error();
			int info = msg->Get_Info();
	//		info = 1;
			if (info == 1)
			{
				p_module->cur_task_id = msg->Get_IntValue(KEY_TASKID);
				std::string cmd = msg->Get_StringValue(KEY_TASK_CMD);
				std::string params = msg->Get_StringValue(KEY_TASK_PARAMS);
				//test
	/*			static int i = 0;
				if (i == 0)
				{
				   i++;
				   cmd.clear();
				   params.clear();

				// test CheckByConnection

				   cmd = "CheckByConnection";
				   params = generate_conn_json();

				// test action rule
				//   cmd = "SetActionRule";
				//   params = generate_rule_json();


				// test UDiskLimit
				// cmd = "UDiskLimit";
				// params = "{\"auto_upload\":1,\"max_size\":10485760,\"mode\":[1,2,3]}";

				// test check by md5
				// cmd = "CheckByMD5";
				// params = "{\"md5\":\"fdfdfdfdsfsdfsdfsd\",\"finish_time\":0,\"mode\":0";

				}
	*/
                center_log(p_module, "heart_beat receive new task cmd: %s params: %s", cmd.c_str(), params.c_str());

                //根据不同命令， 发送给不同的模块

				module_data_t* p_data = create_module_data();
				std::string message_id = cmd;
				set_module_data_property(p_data, g_p_message_id, message_id.c_str(), message_id.length());
				set_module_data_property(p_data, "TASK_PARAMS", (const char*)params.c_str(), params.length());

				module_message_t module_msg;
				module_msg.category = static_cast<module_category_t>(p_module->category);
				module_msg.p_data = p_data;

                mdh_sync_params_t sync_param;
                sync_param.is_sync = true;
                sync_param.ptrs_alloc = sync_alloc;
                sync_param.result.count = 0;
                sync_param.result.pp_ptrs = NULL;
                // p_params 不能为 NULL
                center_log(p_module, "send task cmd:%s params: %s to other module\n", cmd.c_str(), params.c_str());
                p_module->p_notifier(&module_msg, p_module->p_params, &sync_param);
//                printf("### on_heart_beat call back ###\n");
                destroy_module_data(p_data);
  //              printf("### on_heart_beat destroy data ###\n");

                // 从 sync_params 中拿到 task_result  赋值 p_module 中的task_result
                // get task result
                const char* task_result = NULL;
                uint32_t task_res_len = 0;

    //            printf("on_heart_beat get_module_data prev  result count is %d\n", sync_param.result.count);
                if (sync_param.result.count == 0 || sync_param.result.count == (uint32_t)-1)
                {
                    center_log(p_module, "on_heart_beat result count is %d\n", sync_param.result.count);
                    p_module->m_mutex.enter();

                    p_module->task_result = VALUE_NOTASK;
                    p_module->cur_task_id = VALUE_NOTASK;
                    center_log(p_module, "set task_result %d   cmd: %s\n", p_module->task_result, cmd.c_str());

                    p_module->m_mutex.leave();
					delete msg;
                    return 0;
                }

                //copy data and destroy
                for(uint32_t i = 0; i < sync_param.result.count; i++)
                {
                    int ret = get_module_data_property(sync_param.result.pp_ptrs[0], "CENTER_MESSAGE_TASK_RESULT", &task_result, &task_res_len);
                    if (ret != 0)
					{
						center_log(p_module, "heart_beat get module_data task_result err %d\n", ret);
						task_result = 0; //get err set task_result 0
					}
                }

                //release 同步调用返回的 module_data
                for (int i = 0; i < (int)sync_param.result.count; i++)
                {
                    destroy_module_data((module_data_t*)sync_param.result.pp_ptrs[i]);
                }
                delete [] sync_param.result.pp_ptrs;
                // release end

                //lock; 赋值 task result
                //printf("on_heart_beat lock tast_result is %d\n", (int)*task_result);
                p_module->m_mutex.enter();
                if (task_result)
                    p_module->task_result = (int)*task_result;
                if (p_module->task_result == VALUE_NOTASK)
                {
                    p_module->cur_task_id = VALUE_NOTASK;
                }

                // for sdk
                p_module->task_result = VALUE_NOTASK;
                p_module->cur_task_id = VALUE_NOTASK;
                center_log(p_module, "set task_result %d   cmd: %s\n", p_module->task_result, cmd.c_str());
                p_module->m_mutex.leave();
                //unlock;
				delete msg;
                return err_code;
            }
			delete msg;
            return err_code;

        }
        else
        {
            center_log(p_module, "on heart beat response msg is NULL\n");
            return -1;
        }
    }
    else
    {
        center_log(p_module, "on_heart_beat err buf is NULL\n");
        return -1;
    }
}


int center_logout(module_t* p_module)
{
    IRequestMessage* msg = NewInstance_IRequestMessage(CMD_LOGIN, SUBCMD_LOGOUT);

    // msg->Dump();

    std::string encode_data = msg->Encode();
    delete msg;

    //doReq(p_module->host.c_str(), p_module->node_id.c_str(), encode_data.c_str(),encode_data.length(), 1003, req_proc, p_module);
    char* ret_buf = NULL;
    uint32_t ret_size = 0;
    int res = do_request(p_module->host.c_str(), encode_data.c_str(), encode_data.length(),&ret_buf, &ret_size);
    if (res == CURLE_OK){
        res = on_logout(p_module,ret_buf, ret_size);
        release_buf(ret_buf);
        ret_buf = NULL;
    }

    return 0;
}

int on_logout(module_t* p_module, char* p_buf, int n_length)
{
    if (p_buf != NULL)
    {
        int size;
        IResponseMessage* msg = NewInstance_IResponseMessage((const unsigned char*)p_buf, n_length, &size);

        if (msg != NULL)
        {
            // msg->Dump();
            int errcode = msg->Get_Error();
            if (errcode == ERR_OK)
                p_module->stop_flag = false;
            delete msg;
            return errcode;
        }
    }
    return -1;
}
