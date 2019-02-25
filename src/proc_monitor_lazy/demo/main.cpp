#include <time.h>
#include <stdio.h>
#include <inttypes.h>
#include "proc_monitor.h"

void proc_call_back(proc_event_type_t event_type, 
                    size_t nmemb, 
                    Proc **procs,
                    void *p_user_data){
    const char *event;
    if(event_type == PROC_SNAPSHOT)
        event = "PROC_SNAPSHOT";
    else if(event_type == PROC_ACTION_CREATE)
        event = "PROC_ACTION_CREATE";
    else if(event_type == PROC_ACTION_DESTROY)
        event = "PROC_ACTION_DESTROY";
    else
        event = "UNKNOWN";

    printf("event:%s\n", event);
    for(size_t i=0; i<nmemb; i++) {
        printf( "\ttime:%lu\n"
                "\tpid:%" PRIu64 "\n"
                "\tppid:%d\n"
                "\tpath:%s\n"
                "\tstarttime:%lu\n"
                "\tendtime:%lu\n"
                ,
                time(NULL),
                procs[i]->pid,
                procs[i]->ppid,
                procs[i]->abs_name.size() ? procs[i]->abs_name.data() : procs[i]->cmd,
                procs[i]->starttime,
                procs[i]->stoptime
        );
        putchar(10);
    }
    putchar(10);
    putchar(10);
    putchar(10);
}

int32_t main(int32_t argc,char *args[])
{
    size_t snapshot_delta = 3;
    const char *options[1][2] = { {"snapshot_delta", (const char *)&snapshot_delta } };
    proc_monitor_t *handle = init_proc_monitor(proc_call_back, NULL, sizeof(options)/sizeof(options[0]), options);

    snapshot_delta = 5;
    proc_monitor_set_option(handle, "snapshot_delta", (const char *)&snapshot_delta);

    const char *res = proc_monitor_get_option(handle, "snapshot_delta");
    if(res)
        printf("proc_monitor_get_option(snapshot_delta) -> %lu \n", *(size_t *)res);
    else
        printf("proc_monitor_get_option(snapshot_delta) -> failed \n");

    printf("exit with %c\n", getchar());

    fin_proc_monitor(handle);
    return 0;
}
