#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "util-string.h"
#include "stap_file_monitor.h"

static FILE *run_stap(const char *exe, const char *stp)
{
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s %s", exe, stp);

    return popen(cmd, "r");
}

stap_monitor *init_stap_monitor(const char *exe, const char *stp, const stap_interface_t *p_itf)
{
    assert(exe && stp);

    FILE *rfp = run_stap(exe, stp);
    if(!rfp)
        return NULL;

    stap_monitor *pst = (stap_monitor *)calloc(1, sizeof(stap_monitor));
    assert(pst);

    if(p_itf) {
        memcpy(&pst->itf, p_itf, sizeof(stap_interface_t));
    }
    pst->rfp = rfp;
    pst->running = 1;

    return pst;
}

void stop_stap_monitor(stap_monitor *p_stap_monitor)
{
    if(p_stap_monitor) {
        p_stap_monitor->running = 0;
    }
}

void fin_stap_monitor(stap_monitor *p_stap_monitor)
{
    stap_monitor *pst = (stap_monitor *)p_stap_monitor;
    if(pst) {
        pst->running = 0;

        if(pst->rfp) {
            pclose(pst->rfp);
            pst->rfp = NULL;
            printf("%s exit monitor ok\n", __func__);
        }
        free(pst);
        printf("%s exit ok\n", __func__);
    }
}
