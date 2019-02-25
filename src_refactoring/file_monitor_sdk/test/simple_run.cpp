#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <time.h>
#include <sys/time.h>
#include <unistd.h>     // sleep getcwd

#include "module_file_monitor_interface.h"

const char *g_event_table[] = {
    "EVENT_NEW_DIR",
	"EVENT_REMOVE_DIR",
	"EVENT_NEW_FILE",
	"EVENT_REMOVE_FILE",
	"EVENT_MODIFY_FILE",
};

bool event_cb(int32_t event_count, file_event_t *p_events, void *p_data)
{
    char time_buf[100] = { 0 };

    for (int32_t i = 0; i < event_count; i++)
    {
        printf("\npath:       %s\n", p_events[i].p_dest_path);
        printf("type:       %s\n", g_event_table[p_events[i].type]);
        printf("pid:        %d\n", p_events[i].pid);

        strftime(time_buf, sizeof(time_buf),
                 "%Y-%m-%d %H:%M:%S", localtime(&p_events[i].time));

        printf("time:       %s\t%d\n", time_buf, (int)p_events[i].time);
    }
    free_file_event((void*)p_events);
    return true;
}

int main(void)
{
    const char *opt_array[][2] = { {"monitor_directory", "/home/snail"},
                                  {"monitor_directory", "/home/snail/2017"} };

    file_monitor_t *p_monitor = init_file_monitor(event_cb, NULL, 2, opt_array);

    if (NULL == p_monitor) {
        printf("init_file_monitor error\n");
        return -1;
    }
    run_file_monitor(p_monitor);

    destroy_file_monitor(p_monitor);
    p_monitor = NULL;
    return 0;
}
