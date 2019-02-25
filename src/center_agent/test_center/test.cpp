#include <stdio.h>
#include <iostream>
#include <sys/syscall.h>
#include <string>
#include <string.h>
#include <thread>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <vector>
#include "center_agent.h"
#include "module_data.h"
#include "module_interfaces.h"
#include "Mutex.h"
#include "module_def.h"
#include "make_log.h"
#include "IKVMessage.h"
#include "KVMessage.h"
#include "uploader_factory.h"
#include <json/json.h>
#define UUIDS_FILE "./uuids"
//const char *g_p_message_id = "MESSAGE_ID";

using namespace std;

static void* p_log_ret = NULL;
uint32_t g_logok_count = 0;
uint64_t g_report_count = 0;
uint64_t g_report_success_count = 0;
std::string g_s_token = "";

module_t *g_p_module = NULL;

uploader *g_p_upload = NULL;

std::string generate_report_json()
{
    Json::Value encode;

    encode["virus_name"] = "virus_name";
    encode["vlib_version"] = "vlib_version";
    encode["virus_findby"] = 1;
    encode["virus_op"] = 1;
    encode["date"] = time((time_t*)NULL);
    encode["find_time"] = time((time_t*)NULL);
    encode["filepath"] = "ceshipath";
    encode["virus_type"] = 2;

    Json::Value root;
    root["node_file_virus"] = encode;
    root["samps"] = "null";
    root["virus_type"] = "null";
    root["virus_features"] = "null";

    Json::FastWriter writer;
    std::string out2 = writer.write(root);
    return out2;
}

void ck_notify(const module_message_t *p_module_message, void *p_params, mdh_sync_params_t *p_sync)
{

//	printf("call back receive data\n");
	module_data_t *data = copy_module_data(p_module_message->p_data);
	const char* message_id;
	uint32_t msg_id_len;
	get_module_data_property(data,  g_p_message_id, &message_id, &msg_id_len);

    
	if (strcmp(message_id, "MSG_TYPE_LOG") == 0)
	{
       
//		printf("### callback heart_beat MSG_TYPE_LOG ###\n");
		const char* log_str;
		uint32_t log_str_length;
		get_module_data_property(data, "LOG_MESSAGE", &log_str, &log_str_length);

		log_to_file(p_log_ret, "%s", log_str);
       
	}
    else if (strcmp(message_id, "report") == 0)
    {
        const char* buf;
        uint32_t len;
        get_module_data_property(data, "SUCCESS_COUNT", &buf, &len);
        g_report_success_count = *(uint64_t*)(buf);

        get_module_data_property(data, "REQ_COUNT", &buf, &len);
        g_report_count = *(uint64_t*)(buf);

        log_to_file(p_log_ret, "report count is %ld, report success count is %ld\n", g_report_count, g_report_success_count);
//        printf("##########%d   $$$$$$$$%d\n", g_report_success_count, g_report_count);
    }
    /*
	else if (strcmp(message_id, "MSG_TYPE_MONITOR") == 0)
	{
	
		printf("### callback heart_beat MSG_TYPE_MONITOR ###\n");
		p_sync->result.count = 1;
		module_data_t ** ptr = new module_data_t*[1];
		ptr[0] = create_module_data();
		int status = 1;
		set_module_data_property(ptr[0], "CENTER_MESSAGE_TASK_RESULT", (const char*)&status, sizeof(status));
		p_sync->result.pp_ptrs = (const module_data_t**)ptr;
	
	}
	else if (strcmp(message_id, "MSG_TYPE_REPORTER") == 0)
	{
	
		const char* category;
		uint32_t cate_len;
		const char* token;
		uint32_t token_len;
		get_module_data_property(data, "TOKEN", &token, &token_len);
		printf("#### MSG_TYPE_REPORTER receive token is %d ###\n",*((int*)token));
		const char* report_data;
		uint32_t data_length;
		get_module_data_property(data, "REPORTER_MESSAGE_DATA", &report_data, &data_length);

		module_data_t ** ptr = new module_data_t*[1];
		ptr[0] = create_module_data();
		int status = 2;
		set_module_data_property(ptr[0], "CENTER_MESSAGE_TASK_RESULT", (const char*)&status, sizeof(status));
		p_sync->result.pp_ptrs = (const module_data_t**)ptr;
		printf("### MSG_TYPE_REPORTER receive data  is %s data_length is %d  ###\n", report_data, data_length);
	
	}
	else if (strcmp(message_id, "MSG_TYPE_VIRUSSCAN") == 0)
	{
	
		printf("### call back heart_beat MSG_TYPE_VIRUSSCAN ####\n");
		const char* category;
		uint32_t cate_len;
		const char* token;
		uint32_t token_len;
		get_module_data_property(data, "CENTER_MESSAGE_TOKEN", &token, &token_len);
		printf("#### MSG_TYPE_VIRUSSCAN receive token is %d ###\n",(int)(*token));
		const char* params;
		uint32_t params_len;
		get_module_data_property(data, "CENTER_MESSAGE_TASK_PARAMS", &params, &params_len);

		module_data_t ** ptr = new module_data_t*[1];
		ptr[0] = create_module_data();
		int status = 2;
		set_module_data_property(ptr[0], "CENTER_MESSAGE_TASK_RESULT", (const char*)&status, sizeof(status));
		p_sync->result.pp_ptrs = (const module_data_t**)ptr;
		printf("### MSG_TYPE_VIRUSSCAN receive params is %s ###\n", params);
	
	}
    */
	if (strcmp(message_id , "receive_token") == 0)
	{
        __sync_fetch_and_add(&g_logok_count, 1);

        const char* token;
		uint32_t token_len;
		get_module_data_property(data, "TOKEN", &token, &token_len);
		printf("#####receive token is %s\n", token);
        
        std::string ss(token);
        g_s_token = token;

		module_data_t * ptr = NULL;
		ptr = create_module_data();
		int status = 1;
        int bSet = 0;
        std::string msg_id = "CENTER_MESSAGE_NODE_STATUS";
        set_module_data_property(ptr, g_p_message_id, msg_id.c_str(), msg_id.length());
		set_module_data_property(ptr, "CENTER_MESSAGE_NODE_STATUS", (const char*)&status, sizeof(status));
        set_module_data_property(ptr, "CENTER_MESSAGE_NODE_STATUS_SET", (const char*)&bSet, sizeof(bSet));
        assign(g_p_module, ptr, false);

        // g_p_upload->stop_upload();
        // delte g_p_upload;
	/*
		const char* token;
		uint32_t token_len;
		get_module_data_property(data, "TOKEN", &token, &token_len);
		printf("#####receive token is %s\n", token);
	*/
	}
    else if (strcmp(message_id, "relogin") == 0)
    {
        __sync_fetch_and_add(&g_logok_count, -1);

    }
	else if (strcmp(message_id, "SetSnapTime") == 0)
	{
		printf("############## SetSnapTime ############\n");
	}
	else if (strcmp(message_id, "CheckByMD5") == 0)
	{
		printf("############## CheckByMD5 ############\n");

	}
	else if (strcmp(message_id, "CheckByIOC") == 0)
	{
		printf("############## CheckByIOC ############\n");

	}
	destroy_module_data(data);
}

uint32_t g_thread_count = 0;
void* thread_run_agent(void* p_module)
{
    __sync_fetch_and_add(&g_thread_count, 1);
	run((module_t*)p_module);
    __sync_fetch_and_sub(&g_thread_count, 1);
}

void *thread_assign_data(void*)
{
    sleep(2);
    module_data_t * ptr = NULL;
	ptr = create_module_data();
    std::string virus_lib_date = "1546314376";
    std::string msg_id = "CENTER_MESSAGE_VIRUS_LIB_DATE";
    set_module_data_property(ptr, g_p_message_id, msg_id.c_str(), msg_id.length());
    set_module_data_property(ptr, "CENTER_MESSAGE_VIRUS_LIB_DATE", virus_lib_date.c_str(), virus_lib_date.length());
    assign(g_p_module, ptr, false);

}

typedef struct thread_arg
{
	char** 		args;
	uint32_t 	arg_count;
}thread_arg_t;

pid_t get_thread_id()
{
	return syscall(SYS_gettid);
}

void* thread_test(void* param)
{
	printf("thread id is %d\n", get_thread_id());
	thread_arg_t *arg = (thread_arg_t*)param;
	const char** args = (const char**)arg->args;
	uint32_t arg_count = arg->arg_count;
	module_t* p_agent =	create(CATEGORY_CENTER_AGENT, ck_notify, NULL, arg_count, (const char**)args);
	if (!p_agent)
		run(p_agent);
}

// upload thread
void* thread_func(void *thread_parm)
{
    if (NULL != thread_parm) {

        uploader *p_upload = (uploader*)thread_parm;
        p_upload->run_upload();
    }
    return NULL;
}

void state_cb(bool succ, upload_info_t *p_info, void *p_param)
{
    if (succ) {
        printf("upload success %s\n", (char*)p_param);
    } else {
        printf("upload failed %s\n", (char*)p_param);
    }
}

void report_virus()
{
        upload_info_t upload_info;
        
        upload_info.url = "http://192.168.10.48/msgmgr/message/";
        upload_info.timeout = 3;
        std::string virus_info =  generate_report_json();
        IRequestMessage* msg = NewInstance_IRequestMessage(CMD_COMMON_REPORT, SUBCMD_COMMON_REPORT);
        msg->Add_StringValue(KEY_REPORT_TYPE, "VirusLog");
        msg->Add_StringValue(KEY_REPORT_ACTION, "findvirus");
        msg->Add_StringValue(KEY_REPORT_INFO, virus_info.c_str());
		
        RequestMsg *req_msg = (RequestMsg*)msg;
        req_msg->set_token(g_s_token);
        msg->Dump();

        std::string encode_data = msg->Encode();
        delete msg;
        upload_info.buf = encode_data.c_str();
        upload_info.buf_size = encode_data.length();

        printf("upload length is %d\n", upload_info.buf_size);
        upload_info.type = UPLOAD_TYPE_BUF;
        char param1[] = "1 buf upload-";
        g_p_upload->common_upload(&upload_info, param1);
}

int main(int argc, char *argv[])
{
	uint32_t thread_count = 1;
	int opt;
	while((opt = getopt(argc, argv,"c:")) != -1)
	{
		switch(opt){
			case 'c':
				thread_count = atoi(optarg);
				printf("option: %c thread_count: %d\n", opt, thread_count);
				break;
			case '?':
				printf("Unknown option: %c\n", (char)(optopt));
				return -1;
				break;
			case ':':
				printf("option needs a value \n");
				return -1;
		}
	}
	p_log_ret = create_log_file("test_log", 1024*1024*100, 4);
//	thread_arg_t *p_arg = (thread_arg_t*)malloc(sizeof(thread_arg_t));

//	p_arg->arg_count = 10;
//	p_arg->args = (char**)malloc(9*sizeof(char*));
	uint32_t arg_count = 10;
	char **args = (char**)malloc(10*sizeof(char*));

	int i = 0;
	for (i = 0; i < 10; i++)
	{
		args[i] = (char*)malloc(sizeof(char)*50);
	}
	//strcpy(args[0], "http://139.199.63.171:8000/msgmgr/message/");
//	strcpy(args[0], "http://139.199.63.171:8000/msgmgr/message/");
//
	
    FILE *params_fp = fopen("params", "r");
    for(int i = 0; i < 9;i++)
    {
        char str[100] = {0};
        fgets(str, 100, params_fp);
        strcpy(args[i], str);
        args[i][strlen(args[i]) - 1] = '\0';
    }
    fclose(params_fp);

	std::vector<module_t*> vct_modules;
	FILE *fp = NULL;
	fp = fopen(UUIDS_FILE, "r");
	if (fp == NULL)
	{
		printf("open uuids error %d\n", errno);
		return -1;
	}
	for (uint32_t i = 0; i < thread_count; i++)
	{
        usleep(10000);
		char line[1024];
		if(fgets(line, 1023, fp)) {
		    //printf("%s\n",line);
			std::string uuid = line;
			uuid = uuid.substr(0, uuid.length() - 1);
			strcpy(args[9], uuid.c_str());
			pthread_t thread_id;
			module_t* p_agent =	create(CATEGORY_CENTER_AGENT, ck_notify, NULL, arg_count, (const char**)args);
            g_p_module = p_agent;
			if (p_agent)
				vct_modules.push_back(p_agent);

			int ret = pthread_create(&thread_id, NULL, thread_run_agent,(void*)p_agent);
            if (ret != 0)
            {
                perror("create thread");
            }

		}
	}
	fclose(fp);
    pthread_t thread_id;
    int ret = pthread_create(&thread_id, NULL, thread_assign_data,NULL);

    // create one upload thread
    uploader_factory *p_factory = new uploader_factory;
    g_p_upload = p_factory->create_uploader(HTTP_UPLOADER);

    delete p_factory;
    p_factory = NULL;

    g_p_upload->set_upload_cb(state_cb, NULL, NULL);
   // pthread_t thread_id = 0;
//    pthread_create(&thread_id, NULL, thread_func, (void*)g_p_upload);


	while(true)
	{
		sleep(5);
        //report_virus();
		long first_time = 0;
		long reg_time = 0;
		long second_time = 0;
		long max_first_time = 0;
		long max_second_time = 0;
		long max_reg_time = 0;
		std::vector<module_t*>::iterator it = vct_modules.begin();
		for (; it < vct_modules.end(); it++)
		{
			first_time += (*it)->first_login_time;
			reg_time += (*it)->reg_time;
			second_time += (*it)->second_login_time;
			max_first_time = max_first_time > (*it)->first_login_time ? max_first_time : (*it)->first_login_time;
			max_second_time = max_second_time > (*it)->second_login_time ? max_second_time : (*it)->second_login_time;
			max_reg_time = max_reg_time > (*it)->reg_time ? max_reg_time : (*it)->reg_time;
		}

		printf("thread_count is %d\n  logok_count is %d \n average first login time:%lf(s) reg_time: %lf(s) second_login_time: %lf(s) max_first_login_time: %lf(s) max_second_login_time: %lf(s) max_reg_time: %lf(s) report_count:%ld report_success:%ld\n",
                g_thread_count,g_logok_count, (double)first_time / CLOCKS_PER_SEC / vct_modules.size(), (double)reg_time / CLOCKS_PER_SEC / vct_modules.size(), (double)second_time / CLOCKS_PER_SEC / vct_modules.size(), (double)max_first_time / CLOCKS_PER_SEC, (double)max_second_time / CLOCKS_PER_SEC, (double)max_reg_time / CLOCKS_PER_SEC, g_report_count, g_report_success_count);
	}

	printf("*********************** the end!!! **************************\n");

    for (int i = 0; i < 10; i++)
    {
        free(args[i]);
    }
    free(args);
	while(true);
	return 0;
}
