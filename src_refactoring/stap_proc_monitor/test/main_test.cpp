#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <time.h>
#include <sys/time.h>
#include <unistd.h>     // sleep

#include "proc_monitor.h"

const size_t snapshot_delta = 3;
const char *g_options[][2] = { {"snapshot_delta", (const char *)&snapshot_delta } };


typedef struct
{
    int test_no;
    bool result;

    time_t before;
    time_t after;
    int shot_count;

}test_data_t;

static void compare_time(time_t before, time_t after, int test_no)
{
    if(0 != after) {
        if(3 == after - before) {
            printf("test %d seuccess!!!\n", test_no);
            exit(1);
        }
    }
}

void proc_call_back(proc_event_type_t event_type,
                    size_t nmemb, Proc **procs, void *p_user_data){

    if(p_user_data)
    {
        struct timeval tv;
        gettimeofday(&tv,NULL);

        test_data_t *p_test = (test_data_t*)p_user_data;

        if(PROC_SNAPSHOT == event_type)
        {
            p_test->shot_count++;
            if(0 == p_test->before)
                p_test->before = tv.tv_sec;
            else
                p_test->after = tv.tv_sec;
        }
        switch (p_test->test_no)
        {
        case 1:
            compare_time(p_test->before, p_test->after, p_test->test_no);
            break;
        case 2:
            compare_time(p_test->before, p_test->after, p_test->test_no);
            break;
        case 8:
            if(5 == p_test->after - p_test->before) {
                printf("test %d seuccess!!!\n", p_test->test_no);
                exit(1);
            }
            break;
        default:
            break;
        }
    }
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
    for(size_t i=0; i< nmemb; i++) {
        printf( "\tpid:%" PRIu64 "\n"
                "\tppid:%d\n"
                "\tpath:%s\n"
                "\tstarttime:%lu\n"
                "\tendtime:%lu\n"
                ,
                procs[i]->pid,
                procs[i]->ppid,
                procs[i]->abs_name.data(),
                procs[i]->starttime,
                procs[i]->stoptime
        );
        printf("\n");
    }

    printf("\n\n");
}

void wait_for_exit()
{
    printf("\n Press \"Enter\" to continue\n");
    getchar();
}

static void show_result(test_data_t *p_test)
{
    if(p_test->result)
        printf("+ + + + test_%d success\n", p_test->test_no);
    else
        printf("- - - - test_%d error\n", p_test->test_no);
}

void test(int test_no)
{
    test_data_t test = {test_no, false, 0, 0, 0};

    proc_monitor_t *handle = init_proc_monitor(proc_call_back, &test, 1, g_options);
    wait_for_exit();
    fin_proc_monitor(handle);
}
void test_1(void)
{
    test(1);
}

void test_2(void)
{
    test(2);
}

void test_3(void)
{
    test(3);
}

void test_4(void)
{
    test(4);
}

void test_5(void)
{
    test_data_t test = {5, false, 0, 0, 0};

    proc_monitor_t *handle = init_proc_monitor(NULL, &test, 3, g_options);

    if(NULL == handle)
        test.result = true;

    show_result(&test);
    wait_for_exit();
    fin_proc_monitor(handle);
}

void test_6(void)
{
    test_data_t test = {6, false, 0, 0, 0};

    proc_monitor_t *handle = init_proc_monitor(NULL, &test, 1, g_options);

    if(NULL == handle)
        test.result = true;

    show_result(&test);
    wait_for_exit();
    fin_proc_monitor(handle);
}

void test_7(void)
{
    test(7);
}

void test_8(void)
{
    test_data_t test = {8, false, 0, 0, 0};

    proc_monitor_t *handle = init_proc_monitor(proc_call_back, &test, 1, g_options);

    size_t temp_delta = 5;
    test.result = proc_monitor_set_option(handle, "snapshot_delta", (const char *)&temp_delta );

    show_result(&test);
    wait_for_exit();
    fin_proc_monitor(handle);

}

void test_9(void)
{
     test_data_t test = {9, false, 0, 0, 0};

    proc_monitor_t *handle = init_proc_monitor(NULL, &test, 1, g_options);

    size_t temp_delta = 5;
    bool result = proc_monitor_set_option(handle, "__snapshot_delta", (const char *)&temp_delta );

    if(false == result)
        test.result = true;

    show_result(&test);
    wait_for_exit();
    fin_proc_monitor(handle);
}

void test_10(void)
{
    test_data_t test = {10, false, 0, 0, 0};

    proc_monitor_t *handle = init_proc_monitor(NULL, &test, 1, g_options);

    int temp_delta = -5;
    bool result = proc_monitor_set_option(handle, "snapshot_delta", (const char *)&temp_delta );

    if(false == result)
        test.result = true;

    show_result(&test);
    wait_for_exit();
    fin_proc_monitor(handle);
}

void test_11(void)
{
    test_data_t test = {11, false, 0, 0, 0};

    proc_monitor_t *handle = init_proc_monitor(NULL, &test, 1, g_options);

    bool result = proc_monitor_set_option(handle, NULL, NULL);

    if(false == result)
        test.result = true;

    show_result(&test);
    wait_for_exit();
    fin_proc_monitor(handle);
}

void test_12(void)
{
    test_data_t test = {12, false, 0, 0, 0};

    proc_monitor_t *handle = init_proc_monitor(NULL, &test, 1, g_options);

    const char *res = proc_monitor_get_option(handle, "snapshot_delta");

    if(res && snapshot_delta == *(size_t*)res)
        test.result = true;

    show_result(&test);
    wait_for_exit();
    fin_proc_monitor(handle);
}

void test_13(void)
{
    test_data_t test = {13, false, 0, 0, 0};

    proc_monitor_t *handle = init_proc_monitor(NULL, &test, 1, g_options);

    size_t temp_delta = 5;
    proc_monitor_set_option(handle, "snapshot_delta", (const char *)&temp_delta );

    const char *res = proc_monitor_get_option(handle, "snapshot_delta");

    if(res && temp_delta == *(const size_t*)res)
        test.result = true;

    show_result(&test);
    wait_for_exit();
    fin_proc_monitor(handle);
}

void test_14(void)
{
    test_data_t test = {14, false, 0, 0, 0};

    proc_monitor_t *handle = init_proc_monitor(NULL, &test, 1, g_options);

    int temp_delta = 5;
    bool result = proc_monitor_set_option(handle, "___snapshot_delta", (const char *)&temp_delta );

    const char *res = proc_monitor_get_option(handle, "snapshot_delta");

    if(false == result && res && snapshot_delta == *(size_t*)res)
        test.result = true;

    show_result(&test);
    wait_for_exit();
    fin_proc_monitor(handle);
}

void test_15(void)
{
    test_data_t test = {15, false, 0, 0, 0};

    proc_monitor_t *handle = init_proc_monitor(NULL, &test, 1, g_options);

    int temp_delta = 5;
    bool result = proc_monitor_set_option(handle, "___snapshot_delta", (const char *)&temp_delta );

    const char *res = proc_monitor_get_option(handle, "___snapshot_delta");

    if(false == result && res == NULL)
        test.result = true;
    show_result(&test);
    wait_for_exit();
    fin_proc_monitor(handle);
}

void test_16(void)
{
    test_data_t test = {16, false, 0, 0, 0};

    proc_monitor_t *handle = init_proc_monitor(NULL, &test, 1, g_options);

    fin_proc_monitor(handle);

    test.result = true;

    show_result(&test);

    wait_for_exit();
}

int32_t main(int32_t argc, const char *args[])
{
    if(argc < 2) {
        printf("Missing parameter try \"-h\" or \"--help\"\n");
        return -1;
    }

    if( strcmp(args[1], "-h") == 0 || strcmp(args[1], "--help") == 0 )
    {
        printf("\nread standard input\n"
               "1~16,   run the selected test\n");
        return 0;
    }
    switch(atoi(args[1]))
    {
    case 1: test_1(); return 0;
    case 2: test_2(); return 0;
    case 3: test_3(); return 0;
    case 4: test_4(); return 0;
    case 5: test_5(); return 0;
    case 6: test_6(); return 0;
    case 7: test_7(); return 0;
    case 8: test_8(); return 0;
    case 9: test_9(); return 0;
    case 10: test_10(); return 0;
    case 11: test_11(); return 0;
    case 12: test_12(); return 0;
    case 13: test_13(); return 0;
    case 14: test_14(); return 0;
    case 15: test_15(); return 0;
    case 16: test_16(); return 0;
    default:
        break;
    }

    printf("Parameter error: %s\n", args[1]);

    return 0;
}
