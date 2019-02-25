#include <string>

#include "util-path.h"
#include "sys_task.h"
#include "tunnel.h"

#include <stdio.h>
#include <stdlib.h>     /* calloc */
#include <string.h>     /* strdup */
#include <sys/types.h>
#include <sys/stat.h>   /* mkdir */
#include <dirent.h>     /* opendir() , readdir() */

#define TUNNEL_DIRECTORY "./tunnel"

#ifdef  TUNNEL_DEBUG
#define TUNNEL_DEBUG_PRINT(fmt, arg...) printf("tunnel " fmt, ##arg)
#else   /* TUNNEL_DEBUG */
#define TUNNEL_DEBUG_PRINT(fmt, arg...)
#endif  /* TUNNEL_DEBUG */

void tunnel_input(sys_task_t *task)
{
    if(!task || !task->cmd || !task->params || !task->cmd_size || !task->params_size)
        return ;
    std::string fname;
    fname = fname + TUNNEL_DIRECTORY + '/' + task->cmd;

    if(path_exists(TUNNEL_DIRECTORY))
    {
        if(!path_isdir(TUNNEL_DIRECTORY))
        {
            remove(TUNNEL_DIRECTORY);
            TUNNEL_DEBUG_PRINT("[%s] exists, but not dir, remove it\n", TUNNEL_DIRECTORY);
        }
    }
    if(!path_exists(TUNNEL_DIRECTORY))
    {
        mkdir(TUNNEL_DIRECTORY, 0777);
        TUNNEL_DEBUG_PRINT("[%s] un-exists, make it\n", TUNNEL_DIRECTORY);
    }

    FILE *p_file = fopen(fname.c_str(), "w");
    if(p_file)
    {
        fwrite(task->params, sizeof(char), task->params_size, p_file);
        fclose(p_file);
        TUNNEL_DEBUG_PRINT("write cmd:[%s], params:[%s]\n", task->cmd, task->params);
    }
}

static char *file_2_str(const char *abs_file, off_t *size)
{
    off_t file_size = path_getsize(abs_file);
    if(file_size <= 0)
        return NULL;

    char *content = (char *)malloc(file_size+1);
    if(content)
    {
        content[file_size] = '\0';
        FILE *p_file = fopen(abs_file, "r");
        if(p_file)
        {
            size_t readn = fread(content, 1, file_size, p_file);
            if(readn != (size_t)file_size)
            {
                TUNNEL_DEBUG_PRINT("read file[%s] error occurs, readn[%lu], size[%lu]\n", abs_file, readn, file_size);
                free(content);
                content = NULL;
            }
            else
            {
                *size = file_size;
            }
            fclose(p_file);
        }
    }
    return content;
}

static sys_task_t *file_2_task(const char *dir, const char *name)
{
    std::string file;
    file = file + dir + '/' + name;

    off_t cont_size = 0;
    char *content = file_2_str(file.c_str(), &cont_size);
    if(!content)
        return NULL;

    sys_task_t *task = (sys_task_t *)calloc(1, sizeof(sys_task_t));
    if(!task)   return NULL;

    task->cmd = strdup(name);
    task->cmd_size = strlen(task->cmd);
    task->params = content;
    task->params_size = cont_size;

    remove(file.c_str());
    TUNNEL_DEBUG_PRINT("file[%s] get sys_task ok, delete it\n", file.c_str());
    return task;
}

sys_task_t *tunnel_output()
{
    DIR *d = opendir(TUNNEL_DIRECTORY);
    if(!d)  return NULL;

    sys_task_t *task = NULL;
    struct dirent *p_file;
    while(NULL != (p_file = readdir(d)))
    {
        if('.' == p_file->d_name[0])
            continue;
        task = file_2_task(TUNNEL_DIRECTORY, p_file->d_name);
        if(NULL == task)
            continue;
        break;
    }

    closedir(d);
   
    return task;
}

#ifdef TUNNEL_DEBUG

struct fake_task {
    const char *cmd;
    const char *params;
};
static fake_task fake_tasks[] = {
    {"CheckByConnection",   "{\"destip\":\"\",\"domain\":\"www.baidu.com\",\"pid\":\"0\",\"port\":0,\"time\":88}"},
    {"SetActionRule",       "{\"interval_time\":10,\"report_host\":5,\"rules\":[{\"action\":\"1\",\"finish_time\":0,\"object_type\":\"2\",\"objects\":[\"www.baidu.com\",\"www.163.com\"],\"rule_id\":\"1232\"},{\"action\":\"1\",\"finish_time\":0,\"object_type\":\"4\",\"objects\":[\"885acc6870b8ba98983e88e578179a2c\",\"acf8da07c687dbfb0579af4a6dd31871\"],\"rule_id\":\"1232\"}]}"},
    {"CheckByMD5",          "{\"md5\":\"1ca402ca438ed74b69acce1a710537f3\",\"finish_time\":0,\"mode\":0}"},
    {"SetSnapTime",         "{\"report_proc\":\"22\",\"report_net\":22,\"report_host\":22}"},
    {"SetSnapTime",         "{\"report_proc\":\"21\",\"report_net\":21,\"report_host\":21}"},
    {"SetSnapTime",         "{\"report_proc\":\"7\",\"report_net\":7,\"report_host\":1}"},
    {"SetSnapTime",         "{\"report_proc\":\"6\",\"report_net\":6,\"report_host\":1}"}
};

static void sys_task_free(void *argu)
{
    sys_task_t *task = (sys_task_t *)argu;
    if(task)
    {
        if(task->cmd)
            free(task->cmd);
        if(task->params)
            free(task->params);
        free(task);
    }
}

#include <unistd.h>
void run_once()
{
    TUNNEL_DEBUG_PRINT("in-put---------------------------------------------------------\n");
    for(size_t idx=0; idx < sizeof(fake_tasks)/sizeof(fake_tasks[0]); idx++)
    {
        sys_task_t *task = (sys_task_t *)calloc(1, sizeof(sys_task_t));
        if(!task)
            continue;
        task->cmd = strdup(fake_tasks[idx].cmd);
        task->cmd_size = strlen(task->cmd);
        task->params = strdup(fake_tasks[idx].params);
        task->params_size = strlen(task->params);

        tunnel_input(task);
        TUNNEL_DEBUG_PRINT("in-put: cmd:[%" PRId32 "][%s], params:[%" PRId32 "][%s]\n", task->cmd_size, task->cmd, task->params_size, task->params);

        sys_task_free(task);
        usleep(200*1000);
    }

    TUNNEL_DEBUG_PRINT("output---------------------------------------------------------\n");
    sys_task_t *output = NULL;
    while(NULL != (output=tunnel_output()))
    {
        TUNNEL_DEBUG_PRINT("output: cmd:[%s], params:[%s]\n", output->cmd, output->params);
        sys_task_free(output);
    }

    TUNNEL_DEBUG_PRINT("\n\n\n\n\n\n");
}

int main(void)
{
    for(int i=0; i<1000; i++)
    {
        run_once();
        sleep(4);
    }
}
#endif
