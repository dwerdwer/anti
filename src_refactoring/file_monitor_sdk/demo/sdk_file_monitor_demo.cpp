#include "module_file_monitor_interface.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

static bool event_callback_impl(int32_t event_count, file_event_t *p_events, void *p_user_data)
{
    printf("%s\n", (char*)p_user_data);
    for(int32_t i=0; i<event_count; i++) {
        printf("final result - type:%d, pid:%10d, time:%10lu, filename:%s\n",
                p_events[i].type, p_events[i].pid, p_events[i].time, p_events[i].p_dest_path);
    }
    free_file_event(p_events);
    p_events = NULL;

    return true;
}

void *thread_func(void *thread_parm)
{
    if (NULL != thread_parm) {
        file_monitor_t *p_file_monitor = (file_monitor_t*)thread_parm;
        run_file_monitor(p_file_monitor);
    }
    return NULL;
}

int main(void)
{
    void *p_user_data = (void *)"hello";
    char cwd[1024] = {0};
    getcwd(cwd, sizeof(cwd));
    if(cwd[0] == 0) {
        strncpy(cwd, "/home", sizeof(cwd));
    }
    const char *ops[][2] = {{"monitor_directory", cwd}};
    file_monitor_t *p_file_monitor =
        init_file_monitor(event_callback_impl, p_user_data, sizeof(ops)/sizeof(ops[0]), ops);

    if(p_file_monitor == NULL) {
        printf("init_file_monitor failed!\n");
        return 0;
    }
    pthread_t thread_id = 0;

    if (0 != pthread_create(&thread_id, NULL, thread_func, (void*)p_file_monitor))
    {
        perror("pthread_create error");
        destroy_file_monitor(p_file_monitor);
        return -1;
    }

    while(true) {
        printf("\ninput:\n"
                "\tadd_watcher\n"
                "\trm_watcher\n"
                "\tadd_whitelist\n"
                "\trm_whitelist\n"
                "\tquit\n"
        );
        char action[128], in_line[128];
        memset(action, 0, sizeof(action));
        memset(in_line, 0, sizeof(in_line));
        scanf("%s", action);
        printf("%s\n", action);
        if(strstr(action, "quit")) {
            break;
        }
        printf("input path: \n");
        scanf("%s", in_line);
        if(strstr(action, "add_watcher")) {
            add_dir_to_watcher(p_file_monitor, in_line);
        }
        else if(strstr(action, "rm_watcher")) {
            remove_dir_from_watcher(p_file_monitor, in_line);
        }
        else if(strstr(action, "add_whitelist")) {
            add_to_white_list(p_file_monitor, in_line);
        }
        else if(strstr(action, "add_whitelist")) {
            add_to_white_list(p_file_monitor, in_line);
        }
        else {
            printf("invalid command !\n");
        }
    }
    destroy_file_monitor(p_file_monitor);

    pthread_join(thread_id, NULL);

    return 0;
}
