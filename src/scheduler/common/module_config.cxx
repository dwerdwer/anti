
#include "module_config.h"
#include "expat/expat.h"

#include <cstdlib>
#include <cstring>

using namespace std;

#define MAX_STR_LEN 256
#define MAX_MODULES_NUM 128
#define MAX_PARAM_COUNT 30

#define JUDGE_TAG	"yes"
#define CATEGORY	"category"

typedef struct
{
	char name[MAX_STR_LEN];
	module_category_t category;
	int param_count;
}modules_t;

typedef struct
{
	char *file_buf;
	size_t buf_size;

	int depth;
	int modules_count;

	bool module_flag; // 判断是否解析 modules
	vector<module_setting> *modules_set_vec;

	bool msg_flag;
	unordered_map<string, message_rules> *p_msg_rules;

	char cur_module_name[MAX_STR_LEN];
	char cur_param_name[MAX_STR_LEN];
	int cur_module_index; // record cur module' index in modules_set_vec

}configure_info_t;


// 单个元素
typedef struct
{
	bool exist_flag;
	char ret_buf[MAX_STR_LEN];
	char nature[64];

}single_user_data_t;

static void single_start(void *user_data, const XML_Char *name, const XML_Char **atts)
{
	single_user_data_t* user = (single_user_data_t*)user_data;

	if (strncmp(name, user->nature, strlen(name)) == 0)
		user->exist_flag = true;
}

static void single_element(void *user_data, const XML_Char *ele_str, int len)
{
	single_user_data_t* user = (single_user_data_t*)user_data;

	if (len != 0 && user->exist_flag && len < MAX_STR_LEN)
	{
		strncpy(user->ret_buf, ele_str, len);

		user->ret_buf[len] = '\0';
		user->exist_flag = false;
	}
}

static int get_single(configure_info_t *configure_info, single_user_data_t *user)
{
	XML_Parser p_parser = NULL;
	int result = 0;

	if (!(p_parser = XML_ParserCreate(NULL)))
		return -1;

	XML_SetElementHandler(p_parser, single_start, NULL);

	XML_SetCharacterDataHandler(p_parser, single_element);

	user->exist_flag = false;
	XML_SetUserData(p_parser, user);

	if (XML_STATUS_ERROR == XML_Parse(p_parser, configure_info->file_buf, configure_info->buf_size, 0))
		result = -1;
	else if (strcmp(user->ret_buf, "\n") == 0)
		result = -1;

	if (NULL != p_parser)
		XML_ParserFree(p_parser);

	return result;
}

static configure_info_t* create_configure_paser(const char *read_path)
{
	if (NULL == read_path)
		return NULL;

	FILE *rfp = NULL;
	size_t file_size = 0;
	configure_info_t *p_con = (configure_info_t*)malloc(sizeof(configure_info_t));

	if (NULL == p_con)
		return NULL;

	memset(p_con, 0, sizeof(configure_info_t));

	p_con->module_flag = false;

	if (NULL == (rfp = fopen(read_path, "rb")))
		goto ErrEnd;

	fseek(rfp, 0, SEEK_END);
	file_size = ftell(rfp);
	fseek(rfp, 0, SEEK_SET);

	p_con->file_buf = (char*)malloc(file_size);

	if (!p_con->file_buf)
		goto ErrEnd;

	memset(p_con->file_buf, 0, file_size);

	if (fread(p_con->file_buf, 1, file_size, rfp) == 0)
		goto ErrEnd;

	fclose(rfp);
	p_con->buf_size = file_size;

	return p_con;

ErrEnd:
	if (NULL != rfp)
		fclose(rfp);

	if (NULL != p_con)
	{
		if (NULL != p_con->file_buf)
		{
			free(p_con->file_buf);
			p_con->file_buf = NULL;
		}
		free(p_con);
		p_con = NULL;
	}
	return NULL;
}

static uint32_t get_category_num(configure_info_t *configure_info, const char *msg_str)
{
	uint32_t ret_value = 0;

	char *str_tag = NULL;

	char tmp_buf[MAX_STR_LEN] = { 0 };
	strncpy(tmp_buf, msg_str, MAX_STR_LEN);

	single_user_data_t single_user_data;
	memset(&single_user_data, 0, sizeof(single_user_data));

	if ((str_tag = strstr(tmp_buf, "|")) == NULL) // 不含按位或
	{
		memcpy(single_user_data.nature, msg_str, strlen(msg_str));

		get_single(configure_info, &single_user_data);

		sscanf(single_user_data.ret_buf + 2, "%x", &ret_value);
	}
	else
	{
		uint32_t tmp_value = 0;

		str_tag[0] = '\0';
		memcpy(single_user_data.nature, tmp_buf, strlen(tmp_buf));
		get_single(configure_info, &single_user_data);

		sscanf(single_user_data.ret_buf + 2, "%x", &ret_value);

		memcpy(single_user_data.nature, str_tag + 1, strlen(str_tag + 1));
		get_single(configure_info, &single_user_data);

		sscanf(single_user_data.ret_buf + 2, "%x", &tmp_value);

		ret_value = ret_value | tmp_value;
	}
	return ret_value;
}

static void get_argument(configure_info_t *configure_info,
	const XML_Char **atts)
{
	// add by tfei
	module_setting p_module;

	for (int i = 0; atts[i]; i += 2) {

		if (0 == strcmp(atts[i], "name")) {
			p_module.module_name = atts[i + 1];
			strcpy(configure_info->cur_module_name, atts[i + 1]);
		}
		else if (0 == strcmp(atts[i], "path")) {
			p_module.path_name = atts[i + 1];
		}
		else if (0 == strcmp(atts[i], "load")) {
			if (0 == strcmp(atts[i + 1], JUDGE_TAG)) {
				p_module.need_load = true;
			}
			else {
				p_module.need_load = false;
			}
		}
		else if (0 == strcmp(atts[i], "category")) {

			p_module.category = (module_category_t)get_category_num(configure_info, atts[i + 1]);

		}
		else if (0 == strcmp(atts[i], "isolation")) {
			if (0 == strcmp(atts[i + 1], JUDGE_TAG)) {
				p_module.need_isolation = true;
			}
			else {
				p_module.need_isolation = false;
			}
		}
	}

	p_module.p_arguments = (const char**)malloc(MAX_PARAM_COUNT * sizeof(char*));

	p_module.argument_count = 0;

	for (uint32_t i = 0; i < MAX_PARAM_COUNT; i++)
	{
		p_module.p_arguments[i] = (const char*)malloc(MAX_STR_LEN * sizeof(char));

		if (NULL == p_module.p_arguments[i])
			return;

		memset((void*)p_module.p_arguments[i], 0, MAX_STR_LEN * sizeof(char));
	}

	configure_info->modules_set_vec->push_back(p_module);
	configure_info->cur_module_index = configure_info->modules_set_vec->size() - 1;

}




/* get hex string by categories and commands */
static string get_hex_string(configure_info_t *configure_info, const char *msg_str)
{
	string result;

	char *str_tag = NULL;
	char msg_str_buf[MAX_STR_LEN] = { 0 }; // operation this one
	char single_num_str[MAX_STR_LEN / 2] = { 0 };
	strncpy(msg_str_buf, msg_str, MAX_STR_LEN);

	uint32_t tmp_hex = 0;
	single_user_data_t single_user_data;
	memset(&single_user_data, 0, sizeof(single_user_data));

	if ((str_tag = strstr(msg_str_buf, "|")) == NULL) // Contains only one element
	{
		memcpy(single_user_data.nature, msg_str, strlen(msg_str));

		get_single(configure_info, &single_user_data);

		sscanf(single_user_data.ret_buf + 2, "%x", &tmp_hex);
		snprintf(single_num_str, MAX_STR_LEN / 2, "%d", tmp_hex);

		result = single_num_str;
	}
	else
	{
		str_tag[0] = '\0';
		memcpy(single_user_data.nature, msg_str_buf, strlen(msg_str_buf));

		get_single(configure_info, &single_user_data);

		sscanf(single_user_data.ret_buf + 2, "%x", &tmp_hex);
		snprintf(single_num_str, MAX_STR_LEN / 2, "%d", tmp_hex);

		memcpy(single_user_data.nature, str_tag + 1, strlen(str_tag + 1));

		get_single(configure_info, &single_user_data);

		result = single_num_str;
		result.append("|");
		result.append(single_user_data.ret_buf);
	}
	return result;
}



/* name 标签 atts[奇] 字段 atts[偶] 字段 */
static void start_cb(void *user_data, const XML_Char *name, const XML_Char **atts)
{
	configure_info_t *configure_info = (configure_info_t*)user_data;

	configure_info->depth++;

	if (strcmp(name, "modules") == 0)
		configure_info->module_flag = true;

	if (strcmp(name, "messages") == 0)
	{
		configure_info->module_flag = false;
		configure_info->msg_flag = true;
	}

	if (configure_info->p_msg_rules && configure_info->msg_flag) {

		message_rule msg_rule;
		string from_type;
		for (int i = 0; atts[i]; i += 2) {

			if (strcmp(atts[i], "from") == 0) {

				from_type = get_hex_string(configure_info, atts[i + 1]);
			}
			else if (strcmp(atts[i], "to") == 0) {

				msg_rule.to_category = (module_category_t)get_category_num(configure_info, atts[i + 1]);
			}
			else if (strcmp(atts[i], "broadcast") == 0)	{

				// judge broadcast atts value
				if (strcmp(atts[i + 1], JUDGE_TAG) == 0)
					msg_rule.need_broadcast = true;
				else
					msg_rule.need_broadcast = false;
			}
		}

		bool repeat_flag = false;

		for (auto it = configure_info->p_msg_rules->begin(); it != configure_info->p_msg_rules->end(); it++)
		{
			/* When from_type repeat "to categories" push to vector */
			if (!strcmp(from_type.c_str(), (*it).first.c_str()))
			{
				repeat_flag = true;
				(*it).second.push_back(msg_rule);
			}
		}
		/* When repeat_flag == false create map second (mostly) */
		if (false == repeat_flag)
		{
			vector<message_rule>to_rules;
			to_rules.push_back(msg_rule); // Current from_type only have one "to categories"
			configure_info->p_msg_rules->insert(make_pair(from_type, to_rules));
		}
	}

	if (strcmp(name, "module") == 0) {

		if (configure_info->modules_set_vec)
			get_argument(configure_info, atts);

	}

	// record node name
	strcpy(configure_info->cur_param_name, name);
}

static void element_cb(void *user_data, const XML_Char *ele_str, int len)
{
	configure_info_t *user = (configure_info_t*)user_data;

	char tmp_buf[MAX_STR_LEN] = { 0 };

	memcpy(tmp_buf, ele_str, len);

	if (user->module_flag && user->depth == 4 && len != 0)
	{
		// add by tpfei
		// 根据cur_module_name 和 cur_param_name 能唯一确定当前param 值为第几个参数
		// 写入对应的数组位置即可
		if (0 == strcmp(user->cur_module_name, "logger")) {
			if (0 == strcmp(user->cur_param_name, "log_path")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[0],tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "file_size_unit_byte")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[1], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "file_sum")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[2], tmp_buf, len);
			}
            else {

			    int param_count = (*(user->modules_set_vec))[user->cur_module_index].argument_count;
    			memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[param_count], tmp_buf, len);
            }
		}
		else if (0 == strcmp(user->cur_module_name, "rpcsrv")) {
			if (0 == strcmp(user->cur_param_name, "service_port")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[0], tmp_buf, len);
			}
            else {

			    int param_count = (*(user->modules_set_vec))[user->cur_module_index].argument_count;
    			memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[param_count], tmp_buf, len);
            }
		}
		else if (0 == strcmp(user->cur_module_name, "avx")) {
			if (0 == strcmp(user->cur_param_name, "center_info")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[0], tmp_buf, len);
			}
            else {

			    int param_count = (*(user->modules_set_vec))[user->cur_module_index].argument_count;
    			memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[param_count], tmp_buf, len);
            }
		}
		else if (0 == strcmp(user->cur_module_name, "center_agent")) {
			if (0 == strcmp(user->cur_param_name, "center_info")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[0], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "debug_mod")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[1], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "product_id")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[2], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "product_os")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[3], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "product_runmode")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[4], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "product_type")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[5], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "product_version")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[6], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "product_version2")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[7], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "system_version")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[8], tmp_buf, len);
			}
            else {

			    int param_count = (*(user->modules_set_vec))[user->cur_module_index].argument_count;
    			memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[param_count], tmp_buf, len);
            }
		}
		else if (0 == strcmp(user->cur_module_name, "file_monitor")) {
			if (0 == strcmp(user->cur_param_name, "monitor_path")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[0], tmp_buf, len);
			}
            else {

			    int param_count = (*(user->modules_set_vec))[user->cur_module_index].argument_count;
    			memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[param_count], tmp_buf, len);
            }
		}
		else if (0 == strcmp(user->cur_module_name, "proc_monitor")) {

		}
		else if (0 == strcmp(user->cur_module_name, "net_monitor")) {

		}
		else if (0 == strcmp(user->cur_module_name, "sysinfo")) {
			if (0 == strcmp(user->cur_param_name, "center_addr")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[0], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "center_port")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[1], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "sysinfo_delta")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[2], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "proc_action_delta")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[3], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "network_delta")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[4], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "file_white")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[5], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "process_white")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[6], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "module_white")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[7], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "host_white")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[8], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "enable_white_list_log")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[9], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "enable_network_log")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[10], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "edition")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[11], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "wlist_path")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[12], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "rule_path")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[13], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "zip_path")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[14], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "machine_code")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[15], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "file_size")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[16], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "folder_size")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[17], tmp_buf, len);
			}
            else {

			    int param_count = (*(user->modules_set_vec))[user->cur_module_index].argument_count;
    			memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[param_count], tmp_buf, len);
            }

		}
		else if (0 == strcmp(user->cur_module_name, "usb_monitor")) {

		}
		else if (0 == strcmp(user->cur_module_name, "upgrade")) {

			if (0 == strcmp(user->cur_param_name, "logfile")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[0], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "upgrade_daemon_script")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[1], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "upgrade_viruslib_script")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[2], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "upgrade_daemon_viruslib_script")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[3], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "daemon_package")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[4], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "viruslib_package")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[5], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "version_file_local")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[6], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "version_file_url")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[7], tmp_buf, len);
			}
			else if (0 == strcmp(user->cur_param_name, "download_version_file_script")) {
				memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[8], tmp_buf, len);
			}
            else {

			    int param_count = (*(user->modules_set_vec))[user->cur_module_index].argument_count;
    			memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[param_count], tmp_buf, len);
            }
		}
		else {
			// 2019.01.28 之后增加的模块， 不判断param名，写配置文件时，value值， 按照<node>key:value</node>格式写
			// 由各模块自己解析收到的参数内容，不依赖参数顺序

			int param_count = (*(user->modules_set_vec))[user->cur_module_index].argument_count;
			memcpy((void*)((*(user->modules_set_vec))[user->cur_module_index]).p_arguments[param_count], tmp_buf, len);
		}
		(*(user->modules_set_vec))[user->cur_module_index].argument_count++;
	}
}

static void end_cb(void *user_data, const XML_Char *name)
{
	configure_info_t *user = (configure_info_t*)user_data;

	user->depth--;
}

static int get_configure_info(configure_info_t *configure_info)
{
	XML_Parser p_parser = NULL;
	int result = 0;

	if (!(p_parser = XML_ParserCreate(NULL)))
		return -1;

	XML_SetElementHandler(p_parser, start_cb, end_cb);

	if (configure_info->module_flag)
		XML_SetCharacterDataHandler(p_parser, element_cb);

	XML_SetUserData(p_parser, configure_info);

	if (XML_STATUS_ERROR == XML_Parse(p_parser, configure_info->file_buf, configure_info->buf_size, 0))
		result = -1;

	if (NULL != p_parser)
		XML_ParserFree(p_parser);

	return result;
}

static void destroy_configure_paser(configure_info_t *configure_info)
{
	if (NULL != configure_info)
	{
		if (NULL != configure_info->file_buf)
		{
			free(configure_info->file_buf);
			configure_info->file_buf = NULL;
		}
		free(configure_info);
		configure_info = NULL;
	}
}

/* 读配置信息 获取模块参数 */
vector<module_setting> load_module_settings(const string &config_file)
{
	configure_info_t *p_configure = create_configure_paser(config_file.c_str());

	vector<module_setting> modules_set_vec;

	if (NULL == p_configure)
		return modules_set_vec;

	vector<modules_t> modules_array;

	p_configure->module_flag = true;

	p_configure->modules_set_vec = &modules_set_vec;

	if (get_configure_info(p_configure) != 0)
		return modules_set_vec;

	int index = 0;

	destroy_configure_paser(p_configure);

	return modules_set_vec;
}

void destroy_module_settings(vector<module_setting> &module_settings)
{
	for (auto it = module_settings.begin(); it != module_settings.end(); it++)
	{
		if (NULL != (*it).p_arguments)
		{
			for (uint32_t i = 0; i < (*it).argument_count; i++)
			{
				free((void*)(*it).p_arguments[i]);
				(*it).p_arguments[i] = NULL;
			}
			free((*it).p_arguments);
			(*it).p_arguments = NULL;
		}
	}
}

/* 加载数据流向规则 */
message_rules_hash_table load_message_rules(const string &config_file)
{
	configure_info_t *p_configure = create_configure_paser(config_file.c_str());

	unordered_map<string, message_rules>overall_msg_rules;

	if (NULL == p_configure)
		return overall_msg_rules;

	p_configure->p_msg_rules = &overall_msg_rules;

	if (get_configure_info(p_configure) != 0)
	{
		overall_msg_rules.clear();
		return overall_msg_rules;
	}

	destroy_configure_paser(p_configure);

	return overall_msg_rules;
}
