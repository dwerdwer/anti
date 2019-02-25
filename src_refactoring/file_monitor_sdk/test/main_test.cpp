#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <time.h>
#include <sys/time.h>
#include <unistd.h>     // sleep getcwd

#include "module_file_monitor_interface.h"

const char * event_table[] = {
    "EVENT_NEW_DIR",
	"EVENT_REMOVE_DIR",
	"EVENT_NEW_FILE",
	"EVENT_REMOVE_FILE",
	"EVENT_MODIFY_FILE",
};


#define BASE_PATH "/home"
#define TEST_FILE_PATH "/home/xxx.xx"

const char *g_opt_array[][2] = { {"monitor_directory", BASE_PATH} };

typedef struct
{
    int test_no;
    bool result;

}test_data_t;

void wait_for_exit()
{
    printf("\n Press \"Enter\" to continue\n");
    getchar();
}

bool data_proc(int32_t event_count, file_event_t *p_events, void *p_usr_data)
{
    if(p_usr_data)
    {
        test_data_t *p_test = (test_data_t*)p_usr_data;
        p_test->result = true;
        switch (p_test->test_no)
        {
        case 5:
            p_test->result = false;
            break;
        case 33:
            p_test->result = false;
            break;
        default:
            break;
        }
    }
    for (int index=0;index<event_count;index++){
    	printf("    %s:%s\n\n", event_table[p_events->type], p_events->p_dest_path);
    }
    return true;
}

static void file_opt()
{
    system("mkdir dir");

    system("rmdir dir");

    system("touch xxx.xx");

    system("echo \"123\" > xxx.xx");

    system("rm xxx.xx");
}

static void file_opt_b()
{
    system("touch /bin/aaa.aa");

    system("rm /bin/aaa.aa");
}

static void show_result(test_data_t *p_test)
{
    if(p_test->result)
        printf("+ + + + test_%d success\n", p_test->test_no);
    else
        printf("- - - - test_%d error\n", p_test->test_no);
}

void test_func(int test_no)
{
    test_data_t test = {test_no, false};

    file_monitor_t * p_monitor = init_file_monitor(data_proc, NULL, 1, g_opt_array);

    char file_path[1024] = { 0 };
    switch (test_no)
    {
    case 12:
        test.result = add_dir_to_watcher(p_monitor, "/bin");
        file_opt_b();
        goto FuncEnd;
    case 13:
        test.result = add_dir_to_watcher(p_monitor, BASE_PATH);
        file_opt();
        goto FuncEnd;
    case 14:
        if(false == add_dir_to_watcher(p_monitor, "../"));
            test.result = true;
        goto FuncEnd;
    case 15:
        if(false == add_dir_to_watcher(p_monitor, "/xxxx"));
            test.result = true;
        goto FuncEnd;
    case 16:
        add_dir_to_watcher(p_monitor, "/bin");
        test.result = remove_dir_from_watcher(p_monitor, "/bin");
        file_opt();
        goto FuncEnd;
        return;
     case 17:
        if(false == remove_dir_from_watcher(p_monitor, "/bin"))
            test.result = true;
        goto FuncEnd;
     case 18:
        if(false == remove_dir_from_watcher(p_monitor, ".."))
            test.result = true;
        goto FuncEnd;
     case 19:
        getcwd(file_path, sizeof(file_path));
        test.result = add_to_white_list(p_monitor, file_path);
        file_opt();
        goto FuncEnd;
     case 20:
        getcwd(file_path, sizeof(file_path));
        test.result = add_to_white_list(p_monitor, file_path);
        file_opt();
        goto FuncEnd;
     case 21:
        if(false == add_to_white_list(p_monitor, "/bin"))
            test.result = true;
        goto FuncEnd;
     case 22:
        if(false == add_to_white_list(p_monitor, ".."))
            test.result = true;
        goto FuncEnd;
     case 23:
        system("touch /home/xxx.xx");
        test.result = add_to_white_list(p_monitor, TEST_FILE_PATH);

        system("echo \"123\" > /home/xxx.xx");

        system("rm /home/xxx.xx");
        goto FuncEnd;
     case 24:
        system("touch /home/xxx.xx");
        test.result = add_to_white_list(p_monitor, TEST_FILE_PATH);

        system("echo \"123\" > /home/xxx.xx");

        system("rm /home/xxx.xx");
        goto FuncEnd;
     case 25:
        if(false == add_to_white_list(p_monitor, "/bin/ls"))
            test.result = true;
        goto FuncEnd;
     case 26:
        if(false == add_to_white_list(p_monitor, "./run_test"))
            test.result = true;
        goto FuncEnd;
     case 27:
        getcwd(file_path, sizeof(file_path));
        add_to_white_list(p_monitor, file_path);
        test.result = remove_from_white_list(p_monitor, file_path);
        file_opt();
        goto FuncEnd;
     case 28:
        getcwd(file_path, sizeof(file_path));
        if (false == remove_from_white_list(p_monitor, file_path))
            test.result = true;
        goto FuncEnd;
     case 29:
        if (false == remove_from_white_list(p_monitor, "./"))
            test.result = true;
        goto FuncEnd;
     case 30:
        system("touch /home/xxx.xx");
        add_to_white_list(p_monitor, TEST_FILE_PATH);
        test.result = remove_from_white_list(p_monitor, TEST_FILE_PATH);
        system("echo \"123\" > /home/xxx.xx");

        system("rm /home/xxx.xx");
        goto FuncEnd;
     case 31:
        if (false == remove_from_white_list(p_monitor, TEST_FILE_PATH))
            test.result = true;
        goto FuncEnd;
     case 32:
        if (false == remove_from_white_list(p_monitor, "./run_test"))
            test.result = true;
        goto FuncEnd;
     default:
        break;
    }
    file_opt();
    show_result(&test);
    wait_for_exit();
    destroy_file_monitor(p_monitor);
    return;
FuncEnd:
    show_result(&test);
    destroy_file_monitor(p_monitor);
}

void test_1(void)
{
    test_data_t test = {1, false};

    file_monitor_t * p_monitor = init_file_monitor(data_proc, NULL, 1, g_opt_array);

    if(NULL != p_monitor)
        test.result = true;

    show_result(&test);
    wait_for_exit();
    destroy_file_monitor(p_monitor);
}

void test_2(void)
{
    test_data_t test = {2, false};

    const char *opt_array[][2] = { {"monitor_directory", "/home/"},
                                   {"monitor_directory", "/lib"} };
    file_monitor_t * p_monitor = init_file_monitor(data_proc, NULL, 2, opt_array);

    if(NULL != p_monitor)
        test.result = true;

    show_result(&test);
    wait_for_exit();
    destroy_file_monitor(p_monitor);
}

void test_3(void)
{
    test_data_t test = {3, false};

    const char *opt_array[][2] = { {"monitor_directory", "/home/"},
                                   {"monitor_director", NULL} };
    file_monitor_t * p_monitor = init_file_monitor(data_proc, NULL, 2, opt_array);

    if(NULL == p_monitor)
        test.result = true;

    show_result(&test);
    wait_for_exit();
    destroy_file_monitor(p_monitor);
}

void test_4(void)
{
    test_data_t test = {4, false};

    const char *opt_array[][2] = { {"monitor_directory", "/home/"},
        {"monitor_director", "xxxx"} };
    file_monitor_t * p_monitor = init_file_monitor(data_proc, NULL, 2, opt_array);

    if(NULL == p_monitor)
        test.result = true;

    show_result(&test);
    wait_for_exit();
    destroy_file_monitor(p_monitor);
}

void test_5(void)
{
    test_data_t test = {5, true};

    const char *opt_array[][2] = { {"monitor_directory", "/home/"} };
    file_monitor_t * p_monitor = init_file_monitor(data_proc, &test, 1, opt_array);
    file_opt();
    wait_for_exit();
    show_result(&test);
    destroy_file_monitor(p_monitor);
}

void test_7(void)
{
    const char *opt_array[][2] = { {"monitor_directory", "/home/"},
        {"monitor_director", "/lib/"} };
    file_monitor_t * p_monitor = init_file_monitor(data_proc, NULL, 2, opt_array);

    file_opt();

    wait_for_exit();
    destroy_file_monitor(p_monitor);
}

void test_33(void)
{
    test_data_t test = {33, true};

    char file_path[2014] = { 0 };
    getcwd(file_path, sizeof(file_path));
    const char *opt_array[][2] = { {"monitor_directory", file_path} };
    file_monitor_t * p_monitor = init_file_monitor(data_proc, &test, 1, opt_array);

    sleep(1);
    destroy_file_monitor(p_monitor);
    file_opt();
    show_result(&test);
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
               "1~34,   run the selected test\n");
        return 0;
    }
    switch(atoi(args[1]))
    {
    case 1: test_1(); return 0;
    case 2: test_2(); return 0;
    case 3: test_3(); return 0;
    case 4: test_4(); return 0;
    case 5: test_5(); return 0;
    case 6: test_func(6); return 0;
    case 7: test_7(); return 0;
    case 8: test_func(8); return 0;
    case 9: test_func(9); return 0;
    case 10: test_func(10); return 0;
    case 11: test_func(11); return 0;
    case 12: test_func(12); return 0;
    case 13: test_func(13); return 0;
    case 14: test_func(14); return 0;
    case 15: test_func(15); return 0;
    case 16: test_func(16); return 0;
    case 17: test_func(17); return 0;
    case 18: test_func(18); return 0;
    case 19: test_func(19); return 0;
    case 20: test_func(20); return 0;
    case 21: test_func(21); return 0;
    case 22: test_func(22); return 0;
    case 23: test_func(23); return 0;
    case 24: test_func(24); return 0;
    case 25: test_func(25); return 0;
    case 26: test_func(26); return 0;
    case 27: test_func(27); return 0;
    case 28: test_func(28); return 0;
    case 29: test_func(29); return 0;
    case 30: test_func(30); return 0;
    case 31: test_func(31); return 0;
    case 32: test_func(32); return 0;
    case 33: test_33(); return 0;
    default:
        break;
    }

    printf("Parameter error: %s\n", args[1]);

    return 0;
}
