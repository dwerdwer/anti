#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

#include "kv_engine.h"

#define CHANGE_LOAD_ST  "../../tools/run_modify_load"
#define RESTART_SCRIPT  "../../tools/restart_daemon.sh"

#define FILE_MONITOR    "file_monitor"
#define NET_MONITOR     "net_monitor"
#define PROC_MONITOR    "proc_monitor"

#define MISC_NAME       "misc"

using namespace std;

static int iCase = 0;

int init_1(const char *daemon_path)
{
    kv_engine_t *p_engine = initialize_engine();

    if (NULL == p_engine) {
        printf("%s error\n", __func__);
        return -1;
    }
    finalize_engine(p_engine);
    printf("%s success\n", __func__);

    return 0;
}

int init_2(const char *daemon_path)
{
    int pid = fork();

    if (0 == pid)
    {
        finalize_engine(NULL);
        exit(0);
    }
    else if (0 < pid)
    {
        usleep(200000);

        int status = 0;
        int ret = waitpid(pid, &status, WNOHANG);

        if (ret < 0)
        {
            perror("waitpid error");
            printf("%s error\n", __func__);
            return -1;
        }
        if (WIFEXITED(status)) {
            printf("child exited normal exit status: %d\n", WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status)) {
            printf("child exited abnormal signal number: %d\n", WTERMSIG(status));
            printf("%s success\n", __func__);
            return 0;
        }
    }
    printf("%s error\n", __func__);
    return -1;
}

static const char *get_monitor_name(monitor_type_t type)
{
    const char *monitor = NULL;

    if (MONITOR_TYPE_FILE == type)
        monitor = FILE_MONITOR;

    if (MONITOR_TYPE_NETWORK == type)
        monitor = NET_MONITOR;

    if (MONITOR_TYPE_PROCESS == type)
        monitor = PROC_MONITOR;

    return monitor;
}

static void set_single(const char *p_config, bool load_flag, monitor_type_t type)
{
    char cmd_buf[1024] = { 0 };

    const char *load = "n";

    if (true == load_flag)
        load = "y";

    const char *monitor = get_monitor_name(type);

    snprintf(cmd_buf, sizeof(cmd_buf),
             "%s -f %s -m %s -l %s", CHANGE_LOAD_ST, p_config, monitor, load);
    system(cmd_buf);
}

static void restart_daemon(const char *daemon_path)
{
    char cmd_buf[1024] = { 0 };
    snprintf(cmd_buf, sizeof(cmd_buf), "%s %s", RESTART_SCRIPT, daemon_path);
    system(cmd_buf);

    sleep(1);
}

static int enable_and_disable(kv_engine_t *p_engine, const char *daemon_path,
                              const char *p_config, monitor_type_t type)
{
    set_single(p_config, true, type);

    restart_daemon(daemon_path);

    if (0 == enable_monitor(p_engine, type)
        && 0 == disable_monitor(p_engine, type) )
    {
        return 0;
    }
    return -1;
}

static int enable_and_enable(kv_engine_t *p_engine, const char *daemon_path,
                             const char *p_config, monitor_type_t type)
{
    set_single(p_config, true, type);

    restart_daemon(daemon_path);

    if (0 == enable_monitor(p_engine, type)
        && 0 == enable_monitor(p_engine, type) )
    {
        return 0;
    }
    return -1;
}

static int disable_and_disable(kv_engine_t *p_engine, const char *daemon_path,
                               const char *p_config, monitor_type_t type)
{
    // set_single(p_config, true, type);

    // restart_daemon(daemon_path);

    if (0 == disable_monitor(p_engine, type)
        && 0 == disable_monitor(p_engine, type) )
    {
        return 0;
    }
    return -1;
}

static int unload_enable(kv_engine_t *p_engine, const char *daemon_path,
                         const char *p_config, monitor_type_t type)
{
    set_single(p_config, false, type);

    restart_daemon(daemon_path);

    if (type == enable_monitor(p_engine, type))
    {
        return 0;
    }
    return -1;
}

static int unload_disable(kv_engine_t *p_engine, const char *daemon_path,
                          const char *p_config, monitor_type_t type)
{
    set_single(p_config, false, type);

    restart_daemon(daemon_path);

    if (type == disable_monitor(p_engine, type))
    {
        return 0;
    }
    return -1;
}

//./ruin_modify_load -f test_config.xml -m file_monitor -l y
int emdm_1(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    if (0 == enable_and_disable(p_engine, daemon_path, p_config, MONITOR_TYPE_FILE))
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}

int emdm_2(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    if (0 == enable_and_enable(p_engine, daemon_path, p_config, MONITOR_TYPE_FILE))
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}

int emdm_3(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    if (0 == disable_and_disable(p_engine, daemon_path, p_config, MONITOR_TYPE_FILE))
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}

int emdm_4(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    if (0 == unload_enable(p_engine, daemon_path, p_config, MONITOR_TYPE_FILE))
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}

int emdm_5(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    if (0 == unload_disable(p_engine, daemon_path, p_config, MONITOR_TYPE_FILE))
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}

int emdm_6(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    if (0 == enable_and_disable(p_engine, daemon_path, p_config, MONITOR_TYPE_NETWORK))
    {
        printf("%s success\n", __func__);
        return 0;
    }

    printf("%s error\n", __func__);
    return -1;
}

int emdm_7(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    if (0 == enable_and_enable(p_engine, daemon_path, p_config, MONITOR_TYPE_NETWORK))
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}

int emdm_8(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    if (0 == disable_and_disable(p_engine, daemon_path, p_config, MONITOR_TYPE_NETWORK))
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}

int emdm_9(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    if (0 == unload_enable(p_engine, daemon_path, p_config, MONITOR_TYPE_NETWORK))
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}

int emdm_10(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    if (0 == unload_disable(p_engine, daemon_path, p_config, MONITOR_TYPE_NETWORK))
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}

int emdm_11(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    if (0 == enable_and_disable(p_engine, daemon_path, p_config, MONITOR_TYPE_PROCESS))
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}

int emdm_12(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    if (0 == enable_and_enable(p_engine, daemon_path, p_config, MONITOR_TYPE_PROCESS))
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}

int emdm_13(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    if (0 == disable_and_disable(p_engine, daemon_path, p_config, MONITOR_TYPE_PROCESS))
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}

int emdm_14(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    if (0 == unload_enable(p_engine, daemon_path, p_config, MONITOR_TYPE_PROCESS))
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}

int emdm_15(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    if (0 == unload_enable(p_engine, daemon_path, p_config, MONITOR_TYPE_PROCESS))
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}

static void set_all(const char *p_config, bool load_flag)
{
    char cmd_buf[1024] = { 0 };

    const char *load = "n";

    if (true == load_flag)
        load = "y";

    snprintf(cmd_buf, sizeof(cmd_buf),
             "%s -f %s -m %s -l %s", CHANGE_LOAD_ST, p_config, FILE_MONITOR, load);
    system(cmd_buf);

    snprintf(cmd_buf, sizeof(cmd_buf),
             "%s -f %s -m %s -l %s", CHANGE_LOAD_ST, p_config, NET_MONITOR, load);
    system(cmd_buf);

    snprintf(cmd_buf, sizeof(cmd_buf),
             "%s -f %s -m %s -l %s", CHANGE_LOAD_ST, p_config, PROC_MONITOR, load);
    system(cmd_buf);
}

int emdm_16(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    set_all(p_config, true);

    restart_daemon(daemon_path);

    monitor_type_t type_all = (monitor_type_t)
        (MONITOR_TYPE_FILE | MONITOR_TYPE_NETWORK | MONITOR_TYPE_PROCESS);

    if (0 == enable_monitor(p_engine, type_all)
        && 0 == disable_monitor(p_engine, type_all) )
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}

int emdm_17(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    set_all(p_config, true);

    restart_daemon(daemon_path);

    monitor_type_t type_all = (monitor_type_t)
        (MONITOR_TYPE_FILE | MONITOR_TYPE_NETWORK | MONITOR_TYPE_PROCESS);

    if (0 == disable_monitor(p_engine, type_all)
        && 0 == disable_monitor(p_engine, type_all) )
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}

int emdm_18(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    set_all(p_config, true);

    restart_daemon(daemon_path);

    monitor_type_t type_all = (monitor_type_t)
        (MONITOR_TYPE_FILE | MONITOR_TYPE_NETWORK | MONITOR_TYPE_PROCESS);

    if (0 == enable_monitor(p_engine, type_all)
        && 0 == enable_monitor(p_engine, type_all) )
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}

int emdm_19(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    set_all(p_config, false);

    char cmd_buf[1024] = { 0 };

    snprintf(cmd_buf, sizeof(cmd_buf),
             "%s -f %s -m %s -l y", CHANGE_LOAD_ST, p_config, FILE_MONITOR);
    system(cmd_buf);

    restart_daemon(daemon_path);

    monitor_type_t type_all = (monitor_type_t)
        (MONITOR_TYPE_FILE | MONITOR_TYPE_NETWORK | MONITOR_TYPE_PROCESS);

    monitor_type_t type_part = (monitor_type_t)
        (MONITOR_TYPE_NETWORK | MONITOR_TYPE_PROCESS);

    if (type_part == enable_monitor(p_engine, type_all))
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}

int emdm_20(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    set_all(p_config, false);

    char cmd_buf[1024] = { 0 };

    snprintf(cmd_buf, sizeof(cmd_buf),
             "%s -f %s -m %s -l y", CHANGE_LOAD_ST, p_config, FILE_MONITOR);
    system(cmd_buf);

    restart_daemon(daemon_path);

    monitor_type_t type_all = (monitor_type_t)
        (MONITOR_TYPE_FILE | MONITOR_TYPE_NETWORK | MONITOR_TYPE_PROCESS);

    monitor_type_t type_part = (monitor_type_t)
        (MONITOR_TYPE_NETWORK | MONITOR_TYPE_PROCESS);

    if (type_part == disable_monitor(p_engine, type_all))
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}

int emdm_21(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    set_all(p_config, true);

    restart_daemon(daemon_path);

    int undieine_type = 233;

    if (undieine_type == enable_monitor(p_engine, (monitor_type_t)undieine_type))
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}

int emdm_22(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    set_all(p_config, true);

    restart_daemon(daemon_path);

    int undieine_type = 233;

    if (undieine_type == disable_monitor(p_engine, (monitor_type_t)undieine_type))
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}

int emdm_23(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    int pid = fork();

    if (0 == pid)
    {
        enable_monitor(NULL, MONITOR_TYPE_FILE);
        exit(0);
    }
    else if (0 < pid)
    {
        usleep(200000);

        int status = 0;
        int ret = waitpid(pid, &status, WNOHANG);

        if (ret < 0)
        {
            perror("waitpid error");
            printf("%s error\n", __func__);
            return -1;
        }
        if (WIFEXITED(status)) {
            printf("child exited normal exit status: %d\n", WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status)) {
            printf("child exited abnormal signal number: %d\n", WTERMSIG(status));
            return 0;
        }
    }
    printf("%s error\n", __func__);
    return -1;
}

int emdm_24(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    int pid = fork();

    if (0 == pid)
    {
        disable_monitor(NULL, MONITOR_TYPE_FILE);
        exit(0);
    }
    else if (0 < pid)
    {
        usleep(200000);

        int status = 0;
        int ret = waitpid(pid, &status, WNOHANG);

        if (ret < 0)
        {
            perror("waitpid error");
            printf("%s error\n", __func__);
            return -1;
        }
        if (WIFEXITED(status)) {
            printf("child exited normal exit status: %d\n", WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status)) {
            printf("child exited abnormal signal number: %d\n", WTERMSIG(status));
            return 0;
        }
    }
    printf("%s error\n", __func__);
    return -1;
}

int emdm_25(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    int ret = upgrade_daemon(p_engine, UPDATE_TYPE_ALL, 0, NULL);
    if(ret)
    {
        printf("%s error\n", __func__);
    }
    else
    {
        printf("%s success\n", __func__);
    }
    return ret;
}

static void set_misc(const char *p_config, bool load_flag)
{
    char cmd_buf[1024] = { 0 };

    const char *load = "n";

    if (true == load_flag)
        load = "y";

    snprintf(cmd_buf, sizeof(cmd_buf),
             "%s -f %s -m %s -l %s", CHANGE_LOAD_ST, p_config, MISC_NAME, load);
    system(cmd_buf);
}

int assets_1(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    int pid = fork();

    if (0 == pid)
    {
        uint32_t count = 0;
        const asset_t *p_assets = NULL;
        get_assets_info(NULL, &count, &p_assets);
        exit(0);
    }
    else if (0 < pid)
    {
        usleep(200000);

        int status = 0;
        int ret = waitpid(pid, &status, WNOHANG);

        if (ret < 0)
        {
            perror("waitpid error");
            printf("%s error\n", __func__);
            return -1;
        }
        if (WIFEXITED(status)) {
            printf("child exited normal exit status: %d\n", WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status)) {
            printf("child exited abnormal signal number: %d\n", WTERMSIG(status));
            return 0;
        }
    }
    printf("%s error\n", __func__);
    return -1;
}

int assets_2(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    set_misc(p_config, true);

    restart_daemon(daemon_path);

    const asset_t *p_assets = NULL;
    if (0 != get_assets_info(p_engine, NULL, &p_assets))
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}

int assets_3(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    set_misc(p_config, true);

    restart_daemon(daemon_path);

    uint32_t count = 0;
    if (0 != get_assets_info(p_engine, &count, NULL))
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}

int assets_4(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    set_misc(p_config, true);

    restart_daemon(daemon_path);

    uint32_t count = 0;
    if (0 != get_assets_info(p_engine, &count, NULL))
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}

int assets_5(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    set_misc(p_config, false);

    restart_daemon(daemon_path);

    uint32_t count = 0;
    const asset_t *p_assets = NULL;

    if (0 != get_assets_info(p_engine, &count, &p_assets))
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}

int assets_6(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    // set_misc(p_config, true);

    // restart_daemon(daemon_path);

    uint32_t count = 0;
    const asset_t *p_assets = NULL;

    printf("%s begin\n", __func__);
    if (0 == get_assets_info(p_engine, &count, &p_assets))
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}

void notifier_cb(const char *data, size_t datalen, void *p_param)
{

}

int set_notifier_1(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
   int pid = fork();

    if (0 == pid)
    {
        set_notifier(NULL, notifier_cb, NULL);
        exit(0);
    }
    else if (0 < pid)
    {
        usleep(200000);

        int status = 0;
        int ret = waitpid(pid, &status, WNOHANG);

        if (ret < 0)
        {
            perror("waitpid error");
            printf("%s error\n", __func__);
            return -1;
        }
        if (WIFEXITED(status)) {
            printf("child exited normal exit status: %d\n", WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status)) {
            printf("child exited abnormal signal number: %d\n", WTERMSIG(status));
            return 0;
        }
    }
    printf("%s error\n", __func__);
    return -1;
}

int set_notifier_2(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    if (0 != set_notifier(p_engine, NULL, NULL))
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}

int set_notifier_3(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    if (0 == set_notifier(p_engine, notifier_cb, NULL))
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}

int set_notifier_4(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    void *p_param = NULL;
    if (0 == set_notifier(p_engine, notifier_cb, p_param))
    {
        printf("%s success\n", __func__);
        return 0;
    }
    printf("%s error\n", __func__);
    return -1;
}


static void split(void)
{
    printf("- - - - - - - - - - - - - - - - - - - - - - -\n");
}

#define CONTINUE_RUN

static void priv_exit(void)
{
#ifndef CONTINUE_RUN
    exit(1);
#endif
}

int run_all_test(kv_engine_t *p_engine, const char *daemon_path, const char *p_config)
{
    printf("%s run case %d\n", __func__, iCase);
    switch(iCase)
    {
    // case 1:
    //     if (0 != init_1(daemon_path)) {
    //         exit(1);
    //     }
    //     split();
    // break;
    // case 2:
    //     if (0 != init_2(daemon_path)) {
    //         printf("init_2 not crash\n");
    //     }
    //     split();
    // break;
    case 1:
        if (0 != emdm_1(p_engine, daemon_path, p_config)) {
            exit(1);
        }
        split();
    break;
    case 2:
        if (0 != emdm_2(p_engine, daemon_path, p_config)) {
            exit(1);
        }
        split();
    break;
    case 3:
        if (0 != emdm_3(p_engine, daemon_path, p_config)) {
            exit(1);
        }
        split();
    break;
    case 4:
        if (0 != emdm_4(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 5:
        if (0 != emdm_5(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 6:
        if (0 != emdm_6(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 7:
        if (0 != emdm_7(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 8:
        if (0 != emdm_8(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 9:
        if (0 != emdm_9(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 10:
        if (0 != emdm_10(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 11:
        if (0 != emdm_11(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 12:
        if (0 != emdm_12(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 13:
        if (0 != emdm_13(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 14:
        if (0 != emdm_14(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 15:
        if (0 != emdm_15(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 16:
        if (0 != emdm_16(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
        sleep(2);
    break;
    case 17:
        if (0 != emdm_17(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 18:
        if (0 != emdm_18(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 19:
        if (0 != emdm_19(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 20:
        if (0 != emdm_20(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 21:
        if (0 != emdm_21(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 22:
        if (0 != emdm_22(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 23:
        if (0 != emdm_23(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 24:
        if (0 != emdm_24(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 25:
        if (0 != assets_1(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 26:
        if (0 != assets_2(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 27:
        if (0 != assets_3(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 28:
        if (0 != assets_4(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 29:
        if (0 != assets_5(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 30:
        if (0 != assets_6(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 31:
        if (0 != set_notifier_1(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 32:
        if (0 != set_notifier_2(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 33:
        if (0 != set_notifier_3(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
        split();
    break;
    case 34:
        if (0 != set_notifier_4(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
    break;
    case 35:
        if (0 != emdm_25(p_engine, daemon_path, p_config)) {
            priv_exit();
        }
    break;

    default:
        fprintf(stderr, "case %d error!\n", iCase);
        break;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if (access(CHANGE_LOAD_ST, F_OK) == -1)
    {
        printf("\"%s\" %s\n", CHANGE_LOAD_ST, strerror(errno));
        return -1;
    }
    if (access(RESTART_SCRIPT , F_OK) == -1)
    {
        printf("\"%s\" %s\n", CHANGE_LOAD_ST, strerror(errno));
        return -1;
    }
    kv_engine_t *p_engine = NULL;
    int opt = 0;
    const char *daemon_path = NULL;
    const char *p_config = NULL;

    while ((opt = getopt(argc, argv, "d:f:n:")) != -1)
    {
        switch (opt) {
        case 'd':
            daemon_path = optarg;
            break;
        case 'f':
            p_config = optarg;
            break;
        case 'n':
            iCase = atoi(optarg);
            break;
        default: /* '?' */
            fprintf(stderr, "Usage: %s -d daemon_path -f config_file\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    if (NULL == daemon_path || NULL == p_config || iCase == 0) {
        goto args_error;
    }
    p_engine = initialize_engine();

    if (NULL == p_engine) {
        printf("%s error\n", __func__);
        return -1;
    }
    run_all_test(p_engine, daemon_path, p_config);

    finalize_engine(p_engine);

    return 0;
args_error:
    printf("Usage: %s -d main_daemon_path -f config_file -n [1-35]\n", argv[0]);
    exit(EXIT_FAILURE);
}

