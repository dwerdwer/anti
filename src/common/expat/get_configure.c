#include <stdbool.h>
#include "get_configure.h"
#include "expat.h"

static bool exist_flag = false; // 存在标记

typedef struct configure_info_t_
{
	char *file_buf;
	size_t buf_size;

}configure_info_t;


void* create_configure_paser(const char *read_path)
{
	if (NULL == read_path)
		return NULL;
	size_t file_size = 0;
	FILE *rfp = NULL;
	configure_info_t *p_con = (configure_info_t*)malloc(sizeof(configure_info_t));

	if (NULL == p_con)
		return NULL;
	memset(p_con, 0, sizeof(configure_info_t));

	if (NULL == (rfp = fopen(read_path, "rb")))
		goto FileEnd;
	
    fseek(rfp, 0, SEEK_END);
    file_size = ftell(rfp);
	fseek(rfp, 0, SEEK_SET);

	p_con->file_buf = (char*)malloc(file_size);

	if (!p_con->file_buf)
		goto FileEnd;
	memset(p_con->file_buf, 0, file_size);

	if (fread(p_con->file_buf, 1, file_size, rfp) == 0)
		goto FileEnd;
	fclose(rfp);
	p_con->buf_size = file_size;

	return p_con;

FileEnd:
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

/*
   user_data 自定义传入传出参数
   name 标签名
   atts[奇数] 字段名
   atts[偶数] 字段值
 */
static void start_cb(void *user_data, const XML_Char *name, const XML_Char **atts)
{
	char* ret_buf = (char*)user_data;

	if (strcmp(name, ret_buf) == 0)
		exist_flag = true;

	return;
}

/* 元素内容 */
static void character_cb(void *user_data, const XML_Char *ele_str, int len)
{
	char* ret_buf = (char*)user_data;
	if (len != 0 && exist_flag)
	{
		if (len > RET_BUF_LEN)
			return;

		strncpy(ret_buf, ele_str, len);
		ret_buf[len] = '\0';

		exist_flag = false;
	}
}

/* 获取配置信息 */
int get_configure_info(void *p_con, char *ret_buf, const char *key)
{
	if (NULL == p_con || NULL == key)
		return -1;

	configure_info_t *configure_info = (configure_info_t*)p_con;

	if (strlen(key) > RET_BUF_LEN)
		return -1;

	XML_Parser p_parser = NULL;

	// 构造
	if (!(p_parser = XML_ParserCreate(NULL)))
		goto ErrEnd;

	// 注册回调
	XML_SetElementHandler(p_parser, start_cb, NULL);

	XML_SetCharacterDataHandler(p_parser, character_cb);

	// 自定传入参数
	strcpy(ret_buf, key);

	XML_SetUserData(p_parser, ret_buf);	

	//  解析 xml 数据流  
    if (XML_STATUS_ERROR == XML_Parse(p_parser, configure_info->file_buf, (int)configure_info->buf_size, 0))
        goto ErrEnd;

    if (strcmp(ret_buf, key) == 0 || strcmp(ret_buf, "\n") == 0) // 没找到
        goto ErrEnd;

    if (NULL != p_parser)
        XML_ParserFree(p_parser);

    return 0;

ErrEnd:
    if (NULL != p_parser)
        XML_ParserFree(p_parser);

    return -1;
}

void destroy_configure_paser(void *p_con)
{
    configure_info_t *configure_info = (configure_info_t*)p_con;

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
