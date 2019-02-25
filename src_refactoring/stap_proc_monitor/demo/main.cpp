#include <stdio.h>
#include <inttypes.h>
#include "proc_monitor.h"

void proc_call_back(proc_event_type_t event_type,
                    size_t nmemb,
                    Proc **procs,
                    void *p_user_data){
    const char *event = NULL;
    if(event_type == PROC_SNAPSHOT) {
        event = "PROC_SNAPSHOT";
    }
    else if(event_type == PROC_ACTION_CREATE) {
        event = "PROC_ACTION_CREATE";
    }
    else if(event_type == PROC_ACTION_DESTROY) {
        event = "PROC_ACTION_DESTROY";
    }
    else {
        event = "UNKNOWN";
    }


    printf("event:%s\n", event);
    for(size_t i=0; i<nmemb; i++) {
        printf( "\tpid:%d\n"
                "\tppid:%d\n"
                "\tpath:%s\n"
                "\tcmd:%s\n"
                "\tstarttime:%lu\n"
                "\tendtime:%lu\n"
                ,
                procs[i]->pid,
                procs[i]->ppid,
                procs[i]->abs_name.size() ? procs[i]->abs_name.data() : procs[i]->cmd,
                procs[i]->cmdline.data(),
                procs[i]->starttime,
                procs[i]->stoptime
        );
        // must free every proc
        proc_free(procs[i]);

        putchar(10);
    }
    putchar(10);
    putchar(10);
    putchar(10);
}

int main(void)
{
    size_t snapshot_delta = 3;
    const char *options[1][2] = { {"snapshot_delta", (const char *)&snapshot_delta } };
    proc_monitor_t *handle = init_proc_monitor(proc_call_back, NULL, sizeof(options)/sizeof(options[0]), options);

    snapshot_delta = 5;
    proc_monitor_set_option(handle, "snapshot_delta", (const char *)&snapshot_delta);

    run_proc_monitor(handle);

    int res = proc_monitor_get_option(handle, "snapshot_delta", (char *)&snapshot_delta);
    if(res == 0) {
        printf("proc_monitor_get_option(snapshot_delta) -> %d \n", snapshot_delta);
    }
    else {
        printf("proc_monitor_get_option(snapshot_delta) -> failed \n");
    }

    fin_proc_monitor(handle);
    return 0;
}
