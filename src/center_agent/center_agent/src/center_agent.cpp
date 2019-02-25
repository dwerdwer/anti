// center_agent.cpp
#include <iostream>
#include <sys/syscall.h>
#include <vector>
#include <string>
#include <string.h>
#include <unistd.h>
#include <algorithm>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include "utils/utils_library.h"
#include "uuid.h"
#include "module_interfaces.h"
#include "module_data.h"
//#include "module_message_defines.h"
#include "Mutex.h"
#include "module_def.h"
#include "functions.h"
#include "IKVMessage.h"
#include "center_log.h"
using namespace std;

//#define CENTER_AGENT_API __attribute ((visibility("default")))
#define UUID_FILE "/etc/uuid"

//typedef struct {
//	std::vector<module_data_t*> 	m_data_vct;
//	notify_scheduler_t		 		p_notifier;
//	Mutex 							m_mutex;
//}module_t;

//pid_t get_thread_id()
//{
//	return syscall(SYS_gettid);
//}


inline string& ltrim(string &str) {
	string::iterator p = find_if(str.begin(), str.end(), not1(ptr_fun<int, int>(isspace)));
		str.erase(str.begin(), p);
			return str;
}

inline string& rtrim(string &str) {
	string::reverse_iterator p = find_if(str.rbegin(), str.rend(), not1(ptr_fun<int, int>(isspace)));
		str.erase(p.base(), str.end());
			return str;
}

std::string& trim(string &str) {
	ltrim(rtrim(str));
	return str;
}

std::string get_ifaces(void)
{
#ifdef WIN32
	WSADATA    wsadata;
	WSAStartup(MAKEWORD(1, 0), &wsadata);
#endif

#ifdef WIN32
#define ifr_addr        iiAddress.AddressIn
#else
		struct ifconf   ifc;
#define INTERFACE_INFO  struct ifreq
#endif
		struct sockaddr_in  *sin;
		INTERFACE_INFO  *ifr;
		int         sd,	i, j, num = 0;
		in_addr_t   *ifaces, lo;
	unsigned char          buff[sizeof(INTERFACE_INFO) * 16];
	sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sd < 0)
		perror("");
#ifdef WIN32
	if (WSAIoctl(sd, SIO_GET_INTERFACE_LIST, 0, 0, buff, sizeof(buff), (DWORD *)&num, 0, 0) < 0)
		std_err();
	ifr = (INTERFACE_INFO *)buff;
#else
	ifc.ifc_len = sizeof(buff);
	ifc.ifc_buf = (caddr_t)buff;
	if (ioctl(sd, SIOCGIFCONF, (char *)&ifc)< 0)
		perror("");//std_err();
	num = ifc.ifc_len;
	ifr = ifc.ifc_req;
#endif
	num /= sizeof(INTERFACE_INFO);
	close(sd);
	ifaces = (in_addr_t*)malloc(sizeof(in_addr_t) * (num + 2)); // num + lo + NULL
	if (!ifaces) perror("");//std_err();

	lo = inet_addr("127.0.0.1");
	for (j = i = 0; i < num; i++)
	{
		sin = (struct sockaddr_in *)&ifr[i].ifr_addr;
		if (sin->sin_family != AF_INET)
			continue;
		if (sin->sin_addr.s_addr == INADDR_NONE)
			continue;
		if (sin->sin_addr.s_addr == lo)
		{
			continue;
		}
		ifaces[j++] = sin->sin_addr.s_addr;
		/*	ifaces[j++] = sin->sin_addr.s_addr;
			if (sin->sin_addr.s_addr == lo)
			lo = INADDR_NONE;*/																}
																							//	ifaces[j++] = lo;
	ifaces[j] = INADDR_NONE;
																							//return(ifaces);
	std::string ipAddrs = "";
	for (int i = 0; ifaces[i] != INADDR_NONE; i++)
	{
		char str[18] = { 0 };
		sprintf(str, "%x", ifaces[i]);
		ipAddrs.append(str);
		ipAddrs.append(";");
		/*std::string tmp = inet_ntoa(*(struct  in_addr *)&ifaces[i]);
		ipAddrs.append(tmp);
	  	ipAddrs.append(" ");*/
	}
	ipAddrs.erase(ipAddrs.end() - 1);
	trim(ipAddrs);

	return ipAddrs;
}

std::string get_machinename()
{
	char name[65];
	gethostname(name, sizeof(name));
	return std::string(name);
}

std::string getsystemid()
{
	pid_t pid;
	int ret = 0;
	int fd[2] = {0};
	 
	//创建管道
	ret = pipe(fd);
	if(ret == -1)
	{
		perror("pipe");
		_exit(1);
	}
	 
	//创建子进程，目的  1exec 2复制管道文件描述符
	pid = vfork();
	if(pid < 0)
	{
		perror("vfork");
	}
	else if(pid == 0)
	{
		dup2(fd[1], 1);//标准输出重定向到管道的写端
	 
		//char str[50]="dmidecode -s system-serial-number";
		char str[50]="dmidecode -s system-uuid";
		execlp("/bin/sh","sh","-c",str,NULL);
	}
	else
	{
        char result[100] = {0};
        read(fd[0], result, sizeof(result));// 从管道读端读取数据
        std::string uuid = result;
        if (uuid.length() > 1){
            uuid.erase(uuid.length() - 1);
            if (uuid[uuid.length() - 1] >= 'C')
                uuid[uuid.length() - 1] = 'B';
        }
        return uuid;
    }
    return "";
}

char *gen_uuid(char *buf)
{
	uuid_t uuid;
	uuid_generate_time_safe(uuid);
	uuid_unparse(uuid, buf);
	return buf;
}

string get_uuid()
{
	int fd = open(UUID_FILE, O_RDWR);
	if (fd < 0 )
	{
		printf("file note found\n");
		fd = open(UUID_FILE, O_CREAT|O_RDWR, 0600);
		if (fd < 0)
		{
			printf("create file error %d\n",errno);
			return "";
		}
		printf("open ok\n");
        std::string system_uuid = getsystemid();
        std::string to_write = "";
        if (system_uuid.compare(0, 6, "FFFFFF") != 0){
            // 获取system uuid success
            to_write = system_uuid;
        }
        else {
            // random uuid
		    char buf_uuid[1024];
    		system_uuid = gen_uuid(buf_uuid);
		    printf("random uuid %s\n", system_uuid.c_str());        
            to_write = system_uuid;
            to_write.append(";0"); //标识为random uuid
        }
		write(fd, to_write.c_str(), to_write.length());
		close(fd);
		if (system_uuid.length() == 0)
			unlink(UUID_FILE);
		return system_uuid;
	}
	else
	{
		printf("open ok\n");
		char buf[100] = {0};
		read(fd,buf, sizeof(buf) - 1);
        std::string uuid = buf;
        if (uuid.find(";0") != std::string::npos) {
            // random 标识
            uuid = uuid.substr(0, uuid.find(";0"));
        }
        else {
            std::string system_uuid = getsystemid();
            if (system_uuid.compare(0, 35, uuid.substr(0,35)) == 0){
                //相同
            }
            else {
                // 不同，则更新uuid文件
                lseek(fd,0,SEEK_SET);
                ftruncate(fd, 0);
        		write(fd, system_uuid.c_str(),system_uuid.length());
                uuid = system_uuid;
            }
        }

		close(fd);
		return uuid;
	}
}

extern "C" LIB_PUBLIC module_t *create(uint32_t category, notify_scheduler_t notifier,
		void *p_params, uint32_t arg_count, const char** p_args)
{
	// initialize agnet

	module_t *p_center_agent =  new module_t;

	p_center_agent->p_notifier = notifier;
	p_center_agent->p_params = p_params;
	p_center_agent->login_status = LOGIN_NULL;
	p_center_agent->last_heart_time = -1;
	p_center_agent->stop_flag = false;
	p_center_agent->category = category;

	p_center_agent->host = p_args[0];
	std::string run_mode = p_args[1];
	p_center_agent->run_mode = atoi(run_mode.c_str());

	if (p_center_agent->run_mode == 0)
	{
		string uuid = get_uuid();
		p_center_agent->node_id = uuid;
		if (p_center_agent->node_id.length() == 0)
		{
			center_log(p_center_agent, "get uuid error %d\n", errno);
			return NULL;
		}
	}
	else {
		p_center_agent->node_id = p_args[arg_count -1];
	}

	std::string id = p_args[2];
	std::string str_os = p_args[3];
	std::string str_runmode = p_args[4];

	p_center_agent->product_id = atoi(id.c_str());
	p_center_agent->product_os = atoi(str_os.c_str());
	p_center_agent->product_runmode = atoi(str_runmode.c_str());
	p_center_agent->product_type = p_args[5];
	p_center_agent->product_version = p_args[6];
	if (p_args[7][0] == ' ')
		p_center_agent->product_version2 = "";
	else
		p_center_agent->product_version2 = p_args[7];
	p_center_agent->sysversion = p_args[8];
	p_center_agent->machine_name = get_machinename();

	p_center_agent->node_status = 0;
	p_center_agent->cur_task_id = VALUE_NOTASK;
	p_center_agent->task_result = VALUE_NOTASK;

	center_log(p_center_agent, "centerurl: %s uuid:%s", p_center_agent->host.c_str(), p_center_agent->node_id.c_str());

	//for test

	p_center_agent->login_count = 0;
	p_center_agent->start_first_login_time = 0;
	p_center_agent->first_login_time = 0;
	p_center_agent->start_second_login_time = 0;
	p_center_agent->second_login_time = 0;
	p_center_agent->start_reg_time = 0;
	p_center_agent->reg_time = 0;

	return p_center_agent;
}
extern "C" LIB_PUBLIC module_state_t run(module_t *p_module)
{
	if (p_module == NULL)
		return MODULE_FATAL;

//    printf("######\n");

    sleep(5);
    int ret = -1;
    do{
        ret = login(p_module);
        if (ret == 0)
            ;
        else
            sleep(5);

  //      printf("!!!!!!\n");
    }while(p_module->login_status != LOGINED);

    center_heart_beat(p_module, NULL);
    unsigned int r = time(NULL);

    srand(r);
    int rand_size = rand() % 5;
    p_module->rand_size = rand_size + 1;
    //int hear_beat_count = rand_size + 1;
    int hear_beat_count = 1;

    while(true)
	{
		if (p_module->stop_flag)
		{
			center_logout(p_module);
			break;
		}
		if (p_module->m_data_vct.size() == 0)
			;
			//	std::cout << "data buffer is empty!" << std::endl;
		else
		{//测试
			/*
			module_data_t *data = p_module->m_data_vct[0];
			const char* key_data = NULL;
			uint32_t data_length;
			get_module_data_property(data, g_p_property_names[CENTER_MESSAGE_TEST1], &key_data, &data_length);

			printf("data:%s length:%d\n", key_data,data_length);
			*/
		}

		// 已登录的情况 发心跳
		if (p_module->login_status == LOGINED)
		{
			//
			if (p_module->last_heart_time != -1)
			{
				time_t cur = time((time_t*)NULL);
//				printf("cur time is %d last_heart_time is %d\n", cur, p_module->last_heart_time);
				if (cur - p_module->last_heart_time >= p_module->elapsed_time)
				{
					p_module->last_heart_time = cur;
					// send heart_beat
					//printf("***** heart_beat *****\n");
					int ret = center_heart_beat(p_module, NULL);
                    if (ret == ERR_NEED_LOGIN)
                    {
                        center_log(p_module, "heart_beat err retry login\n");
                        module_data_t* p_data = create_module_data();
                        std::string message_id = "relogin";
                        set_module_data_property(p_data, g_p_message_id, message_id.c_str(),message_id.length());
                        
                        module_message_t module_msg;
                        module_msg.category = static_cast<module_category_t>(p_module->category);
                        module_msg.p_data = p_data;
                        mdh_sync_params_t sync_param;
                        sync_param.is_sync = false;
                        
                        p_module->p_notifier(&module_msg, p_module->p_params, &sync_param);
                        destroy_module_data(p_data);

                        login(p_module);
                    }
                    /* fot test*/
                    /*
                    printf("heart_beat_count is %d\n", hear_beat_count);
                    if(--hear_beat_count == 0)
                    {
                        printf("#########");
                        center_report(p_module);
                        hear_beat_count = p_module->rand_size;
                    }
                    */
                    /**/



				}
			}
			else
			{
				p_module->last_heart_time = time((time_t*)NULL);
			}

			// TODO: send heart_beat request
		}

		sleep(3);
	}
	return MODULE_OK;
}

extern "C" LIB_PUBLIC module_state_t stop(module_t *p_module)
{
	//logout
	printf("stop center_agent\n");
	p_module->stop_flag = true;
	while(true == p_module->stop_flag) ;

	// TODO: 恢复初始值
	return MODULE_OK;
}

extern "C" LIB_PUBLIC void destroy(module_t *p_module)
{
	stop(p_module);
	printf("destroy center_agent\n");
	if(p_module){

		delete p_module;
		printf("destroy center_agent end\n");
	}
}


extern "C" LIB_PUBLIC module_data_t *assign(module_t *p_module, module_data_t *p_data, bool is_sync)
{
	// 暂不处理 is_sync
	if (p_data == NULL || p_module == NULL)
		return NULL;
	module_data_t *data = copy_module_data(p_data);
	if (data == NULL)
		return NULL;
	const char* message_id;
	uint32_t msg_id_len;
	get_module_data_property(data,  g_p_message_id, &message_id, &msg_id_len);

	if (strcmp(message_id, "CENTER_MESSAGE_TASK_RESULT") == 0)
	{
		const char* task_result;
		uint32_t task_res_len;

		int ret = get_module_data_property(data, "CENTER_MESSAGE_TASK_RESULT", &task_result, &task_res_len);
		if (ret != 0)
		{
	        center_log(p_module, "assign get module_data task_result err %d\n", ret);
	        task_result = 0; //get err set task_result 0
		}
		const char* task_id;
		uint32_t task_id_len;
		ret = get_module_data_property(data, "CENTER_MESSAGE_TASK_ID", &task_id, &task_id_len);
		p_module->m_mutex.enter();
		p_module->task_result = (int)*task_result;
		if (p_module->task_result == VALUE_NOTASK)
		{
		    p_module->cur_task_id = VALUE_NOTASK;
		}
		else
		{
			p_module->cur_task_id = (int)*task_id;
		}
		p_module->m_mutex.leave();

	}
	else if (strcmp(message_id, "CENTER_MESSAGE_NODE_STATUS") == 0)
	{
		const char* node_status;
		uint32_t ns_len;
		int ret = get_module_data_property(data, "CENTER_MESSAGE_NODE_STATUS", &node_status, &ns_len);
		if (ret != 0)
		{
			center_log(p_module, "assign get module_data node_status err %d\n", ret);
			return NULL;
		}

        const char* bset;
        uint32_t b_len;
        get_module_data_property(data, "CENTER_MESSAGE_NODE_STATUS_SET", &bset, &b_len);
		if (ret != 0)
		{
			center_log(p_module, "assign get module_data node_status bset err %d\n", ret);
			return NULL;
		}

		p_module->m_mutex.enter();
        if ((int)*bset){

    		p_module->node_status = p_module->node_status | (int)*node_status;
            center_log(p_module, "set node_status %d", p_module->node_status);
        }
        else {

            center_log(p_module, "unset node_status begin node_status %d, %d", p_module->node_status, (int)*node_status);
            p_module->node_status = p_module->node_status & ~((int)*node_status);
            center_log(p_module, "unset node_status %d", p_module->node_status);
        }
		p_module->m_mutex.leave();
	}
	else if (strcmp(message_id, "CENTER_MESSAGE_VIRUS_LIB_DATE") == 0)
    {
		const char* virus_lib_date;
		uint32_t ns_len;
		int ret = get_module_data_property(data, "CENTER_MESSAGE_VIRUS_LIB_DATE", &virus_lib_date, &ns_len);
		if (ret != 0)
		{
			center_log(p_module, "assign get module_data virus_lib_date err %d\n", ret);
			return NULL;
		}

		p_module->m_mutex.enter();
   		p_module->product_version2 = virus_lib_date;
        center_log(p_module, "set product_version2 virus_lib_date %s", p_module->product_version2.c_str());
		p_module->m_mutex.leave();
    }
    else{
        ;
	}

	return NULL;
}

static const char *cmd[] = {"CENTER_MESSAGE_NODE_STATUS", "CENTER_MESSAGE_TASK_RESULT", "CENTER_MESSAGE_VIRUS_LIB_DATE"};


extern "C" LIB_PUBLIC void get_inputted_message_type(module_t *p_module, const char *** const ppp_inputted_message_types,
		        uint32_t *p_message_type_count)
{
	int count = sizeof(cmd) / sizeof(cmd[0]);
//	printf("sizeof cmd is %d\n sizeof cmd[0] is %d\n", sizeof(cmd), sizeof(cmd[0]));
	*p_message_type_count = count;
	*ppp_inputted_message_types  = cmd;
}
