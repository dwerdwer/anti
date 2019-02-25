#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/times.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "debug_print.h"
#include "util-string.h"
#include "stap_monitor.h"

// #define PUBLIC_API __attribute__((visibility("default")))
#define PUBLIC_API

#define LINE_MAX_SIZE 1024

struct stap_monitor {
    FILE           *rfp;
    int             rfd;
    fd_set          rfd_set;
    int             error_no;

    stap_action_cb  p_callback;
    void           *p_user_data;
};

enum tokes_proc_index {
    INDEX_PROC_PROC_ACTION         = 0,
    INDEX_PROC_TIMESTAMP           = 1,
    INDEX_PROC_PARENT_PID          = 2,
    INDEX_PROC_PID                 = 3,
    INDEX_PROC_MAX
};

#define PROC_ACTION_EXIT     0
#define PROC_ACTION_CREATE   1

#define MAX_TOKS 20

static int set_block_flag(int fd, int flag)
{
    int flags = fcntl(fd, F_GETFL);
    if(flags == -1) {
        return -1;
    }
    flags |= flag;
    return fcntl(fd, F_SETFL, flags);
}


static FILE *run_stap(const char *exe, const char *ko)
{
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s %s", exe, ko);

    return popen(cmd, "re");
}

static int proc_action_transform(int flag)
{
    switch (flag) {
        case PROC_ACTION_CREATE:
            return STAP_PROC_ACTION_CREATE;
        case PROC_ACTION_EXIT:
            return STAP_PROC_ACTION_EXIT;
        default:
            break;
    }
    return -1;
}

static void parse_proc(char **toks, int num_toks, stap_monitor_t *p_stap_monitor)
{
    if(num_toks != INDEX_PROC_MAX)
        return ;
    stap_proc_action_t act;
    memset(&act, 0, sizeof(act));

    // INDEX_PROC_PROC_ACTION
    act.action = proc_action_transform(atoi(toks[INDEX_PROC_PROC_ACTION]));
    if(act.action < 0)
        goto parse_proc_end;
    // INDEX_PROC_TIMESTAMP
    act.time = atol(toks[INDEX_PROC_TIMESTAMP]);
    // INDEX_PROC_PARENT_PID
    act.ppid = atoi(toks[INDEX_PROC_PARENT_PID]);
    // INDEX_PROC_PID
    act.pid = atoi(toks[INDEX_PROC_PID]);
    if(!act.pid)
        goto parse_proc_end;

    if(p_stap_monitor->p_callback) {
        p_stap_monitor->p_callback(&act, p_stap_monitor->p_user_data);
    }

parse_proc_end:
    return ;
}

static void parse_line(const char *line, stap_monitor_t *p_stap_monitor)
{
    int num_toks = 0;
    char **toks = str_split(line, " \t\r\n", MAX_TOKS, &num_toks, 0);
    if(!toks || !num_toks) {
        debug_print("%s: str_split parse[%s] failed\n",
                __func__, line);
        return ;
    }

    parse_proc(toks, num_toks, p_stap_monitor);
    str_split_free(&toks, num_toks);

    return ;
}

PUBLIC_API void wait_stap_monitor(stap_monitor_t *p_stap_monitor, struct timeval *timeout)
{
    char buffer[LINE_MAX_SIZE];

    if(!p_stap_monitor) {
        return;
    }
    if(p_stap_monitor->error_no) {
        if(timeout) {
            useconds_t time_us = timeout->tv_sec*1000000UL + timeout->tv_usec;
            usleep(time_us);
        }
        else {
            usleep(100000);
        }
        return;
    }

    fd_set read_set = p_stap_monitor->rfd_set;
    select(p_stap_monitor->rfd + 1, &read_set, NULL, NULL, timeout);
    if(FD_ISSET(p_stap_monitor->rfd, &read_set)) {
        while(fgets(buffer, LINE_MAX_SIZE, p_stap_monitor->rfp)) {
            parse_line(buffer, p_stap_monitor);
        }
        if(errno == EAGAIN) {
            // ignore
        }
        else {
            // ENOENT, EINVAL
            DEBUG_PRINT("%d, %s\n", errno, strerror(errno));
            p_stap_monitor->error_no = errno;
        }
    }
}

PUBLIC_API stap_monitor_t *init_stap_monitor(const char *exe, const char *ko, stap_action_cb p_callback, void *p_user_data)
{
    if(!exe || !ko || !p_callback)
        return NULL;

    FILE *rfp = run_stap(exe, ko);
    if(!rfp) {
        return NULL;
    }

    stap_monitor_t *p_stap_monitor = (stap_monitor_t *)calloc(1, sizeof(stap_monitor_t));
    assert(p_stap_monitor);

    p_stap_monitor->error_no = 0;
    p_stap_monitor->rfp = rfp;
    p_stap_monitor->rfd = fileno(rfp);
    p_stap_monitor->p_callback = p_callback;
    p_stap_monitor->p_user_data = p_user_data;

    FD_ZERO(&p_stap_monitor->rfd_set);
    FD_SET(p_stap_monitor->rfd, &p_stap_monitor->rfd_set);
    set_block_flag(p_stap_monitor->rfd, O_NONBLOCK);
    DEBUG_PRINT("set_block_flag(O_NONBLOCK) -> %d\n", set_block_flag(p_stap_monitor->rfd, O_NONBLOCK));
    return p_stap_monitor;
}

PUBLIC_API void fin_stap_monitor(stap_monitor_t *p_stap_monitor)
{
    if(p_stap_monitor) {
        if(p_stap_monitor->rfp) {
            pclose(p_stap_monitor->rfp);
            p_stap_monitor->rfp = NULL;
            DEBUG_PRINT("%s exit monitor ok\n", __func__);
        }
        free(p_stap_monitor);
        DEBUG_PRINT("%s exit ok\n", __func__);
    }
}
