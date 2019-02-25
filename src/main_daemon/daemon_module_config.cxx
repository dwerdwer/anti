
#include "expat.h"
#include "daemon_module_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STR_LEN 256

typedef struct
{
    char *file_buf;
    size_t buf_size;

}configure_info_t;

typedef struct
{
    int depth;
    uint32_t param_index;
    bool name_tag;
    bool modu_tag;  

    int modules_count;
    dm_setting *p_setting;
    configure_info_t *configure_data;

}modules_info_t;

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


/* name 标签 atts[奇] 字段 atts[偶] 字段 */
static void base_info_start(void *user_data, const XML_Char *name, const XML_Char **atts)
{
    modules_info_t *modules_info = (modules_info_t*)user_data;

    modules_info->depth++;

    if (modules_info->modu_tag)
        modules_info->p_setting->argument_count++;

    if (modules_info->name_tag)
    {
        if (!strcmp(name, "module"))
            modules_info->modu_tag = true;
    }
    for (int i = 0; modules_info->name_tag && atts[i]; i += 2)
    {
        if (!strcmp(atts[i], "name"))
        {
            modules_info->p_setting->module_name = atts[i + 1];

            modules_info->p_setting->path_name = atts[i + 3];
        }
    }
    if (!strcmp(name, "modules"))
        modules_info->name_tag = true;
}

static void base_info_end(void *user_data, const XML_Char *name)
{
    modules_info_t *modules_info = (modules_info_t*)user_data;

    if (modules_info->name_tag)
    {
        if (!strcmp(name, "module"))
        {
            modules_info->modu_tag = false;

            modules_info->modules_count++;
        }
    }
    modules_info->depth--;
}

static void get_module_base_info(modules_info_t *modules_info)
{
    XML_Parser p_parser = NULL;

    if (!(p_parser = XML_ParserCreate(NULL)))
        return;

    XML_SetElementHandler(p_parser, base_info_start, base_info_end);

    XML_SetUserData(p_parser, modules_info);

    if (XML_STATUS_ERROR == XML_Parse(p_parser, modules_info->configure_data->file_buf, 
                                      modules_info->configure_data->buf_size, 0))
        return;

    if (NULL != p_parser)
        XML_ParserFree(p_parser);
}


static void start_cb(void *user_data, const XML_Char *name, const XML_Char **atts)
{
    modules_info_t *modules_info = (modules_info_t*)user_data;

    modules_info->depth++;
}

static void element_cb(void *user_data, const XML_Char *ele_str, int len)
{
    modules_info_t *modules_info = (modules_info_t*)user_data;

    if (modules_info->depth == 3 && len != 0 && modules_info->param_index < modules_info->p_setting->argument_count)
    {
        strncpy((char*)modules_info->p_setting->p_arguments[modules_info->param_index], ele_str, len);

        modules_info->param_index++;
    }
}

static void end_cb(void *user_data, const XML_Char *name)
{
    modules_info_t *modules_info = (modules_info_t*)user_data;

    modules_info->depth--;
}

static int get_params(modules_info_t *modules_info)
{
    int result = 0;
    XML_Parser p_parser = NULL;

    if (!(p_parser = XML_ParserCreate(NULL)))
        return -1;

    XML_SetElementHandler(p_parser, start_cb, end_cb);

    XML_SetCharacterDataHandler(p_parser, element_cb);

    XML_SetUserData(p_parser, modules_info);

    if (XML_STATUS_ERROR == XML_Parse(p_parser, modules_info->configure_data->file_buf, 
                                      modules_info->configure_data->buf_size, 0))
        result = -1;

    if (NULL != p_parser)
        XML_ParserFree(p_parser);

    return result;
}

bool load_dm_setting(const std::string &config_file, dm_setting &setting)
{
    bool result = true;

    configure_info_t *p_configure = create_configure_paser(config_file.c_str());

    if (NULL == p_configure)
    {
        result = false;
        return result;
    }
    modules_info_t modules_info;

    setting.argument_count = 0;
    modules_info.depth = 0;
    modules_info.param_index = 0;
    modules_info.configure_data = p_configure;
    modules_info.p_setting = &setting;

    modules_info.name_tag = false;
    modules_info.modu_tag = false;

    get_module_base_info(&modules_info);

    setting.p_arguments = new const char*[setting.argument_count];

    for (uint32_t i = 0; i < setting.argument_count; i++)
    {
        setting.p_arguments[i] = new char[MAX_STR_LEN];
        if (NULL == setting.p_arguments[i])
            result = false;

        memset((void*)setting.p_arguments[i], 0, MAX_STR_LEN * sizeof(char));
    }
    if (get_params(&modules_info) != 0)
        result = false;

    if (NULL != p_configure)
    {
        if (NULL != p_configure->file_buf)
            free(p_configure->file_buf);

        free(p_configure);
        p_configure = NULL;
    }

    return result;
}

void destroy_dm_setting(dm_setting &setting)
{
    if (NULL != setting.p_arguments)
    {
        for (uint32_t i = 0; i < setting.argument_count; i++)
        {
            delete [] setting.p_arguments[i];
            setting.p_arguments[i] = NULL;
        }
        delete [] setting.p_arguments;
        setting.p_arguments = NULL;
    }
}
