#include <iostream>
#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <dirent.h>
// #include "../material/use_sdk/include/antivirus_interface.h"
#include "sdk_includes/antivirus_interface.h"

#define FUNCTIONSTARTINFO printf("[i] start test %s \n", __FUNCTION__)
#define DEBUGINFO printf("file : %s --- line : %d --- function : %s\n", __FILE__, __LINE__, __FUNCTION__)
// 定义文件总个数，用于判断扫描是否完成，需要手动修改
// #define FILECOUNT 5
// 定义等待扫描完成时间，用于判断中途停止后是否还在扫描，需要确保在此时间内扫描可以完成，需要手动修改
// #define WAITMINUTE 3

using namespace std;

//一些全局变量
string global_log_filename = "sdk_test.log";
string global_daemon_path_with_name = "/home/jiangmin_antivirus/linux_kv_sdk_test/scripts/run_daemon.sh";
string global_daemon_config_xml_path_with_name = "/home/jiangmin_antivirus/linux_kv_sdk_test/etc/customize_edr.xml";
string global_run_modify_load_path_with_name = "/home/jiangmin_antivirus/linux_kv_sdk_test/sdk_executable/run_modify_load";
string global_kv_plugin_name = "sysinfo";
string global_restart_main_daemon_script_path_with_name = "/home/jiangmin_antivirus/linux_kv_sdk_test/scripts/restart_daemon.sh";
int global_log_method = 1; //记录日志方式：0-仅显示错误（标准输出），1-全部显示（标准输出），2-仅显示错误（文件），3-全部显示（文件）

int global_retrycount = 3; //设置全局重试次数为3，超过重试次数即为失败
const void *global_ptr = NULL;
uint32_t golbal_scancount = 0; //记录扫描文件数量
int global_waitminute = 0;     //扫描用时（分钟），确保在此时间内可以扫描完
uint32_t global_filecount = 0; //总文件数量
uint32_t global_scanoption = Unzip;
string global_scanpath = "/home/jiangmin_antivirus/linux_kv_sdk_test/sdk_scan_path";            //扫描路径,路径下最少需要两个文件
string global_specialpath = "/home/jiangmin_antivirus/linux_kv_sdk_test/sdk_special_scan_path"; //只含有一个病毒文件的路径
// char* global_knownvirusfilename = "";
string global_virusnewfilename = "";
uint32_t global_quarantinefilecount = 0;              //实时记录的隔离区总文件数量
uint32_t global_quarantineallfilecount = 0;           //扫描后隔离区会存在的总文件数量
string global_testinfo = "chapter 01 - test 01"; //测试编号

void prinf_variblity(){
    cout<<"***********************"<<endl;
    cout<<"global_log_filename"<<" ---> "<<global_log_filename<<endl;
    cout<<"global_daemon_path_with_name"<<" ---> "<<global_daemon_path_with_name<<endl;
    cout<<"global_daemon_config_xml_path_with_name"<<" ---> "<<global_daemon_config_xml_path_with_name<<endl;
    cout<<"global_run_modify_load_path_with_name"<<" ---> "<<global_run_modify_load_path_with_name<<endl;
    cout<<"global_kv_plugin_name"<<" ---> "<<global_kv_plugin_name<<endl;
    cout<<"global_restart_main_daemon_script_path_with_name"<<" ---> "<<global_restart_main_daemon_script_path_with_name<<endl;
    cout<<"global_scanoption"<<" ---> "<<global_scanoption<<endl;
    cout<<"global_log_method"<<" ---> "<<global_log_method<<endl;
    cout<<"global_waitminute"<<" ---> "<<global_waitminute<<endl;
    cout<<"global_filecount"<<" ---> "<<global_filecount<<endl;
    cout<<"global_scanpath"<<" ---> "<<global_scanpath<<endl;
    cout<<"global_specialpath"<<" ---> "<<global_specialpath<<endl;
    cout<<"***********************"<<endl;
}

//判断文件夹是否存在
bool file_exist(string path_with_name){
    if (access(path_with_name.c_str(), F_OK) == 0)
    {
        return true;
    }
    return false;
}
//判断文件夹是否存在
bool folder_exist(string path_with_name){
    if (opendir(path_with_name.c_str()) == NULL){
        return false;
    }
    return true;
}

string getcurrenttime(){
    struct tm *tm_ptr;
    time_t the_time;
    char buf[256];
    char *result;
    (void)time(&the_time);
    tm_ptr = localtime(&the_time);
    strftime(buf, 256, "%Y-%m-%d %H:%M:%S", tm_ptr);
    string str = buf;
    return str;
}

//运行shell命令,不同于system
string ShellCommand(string cmd, int timeout){
    cout << "try run shell command : "<<cmd<<endl;
    FILE *pp = popen(cmd.c_str(), "r");
    if (!pp)
        return "";
    char buffer[1024 * 2] = {0};
    string str_buffer = "";
    while (fgets(buffer, sizeof(buffer), pp) != NULL)
    {
        str_buffer.append(buffer);
    }
    pclose(pp);
    cout<<"shell comand return : "<<str_buffer<<endl;
    return str_buffer;
}

bool Log2File(string str)
{
    FILE *fd = NULL;
    fd = fopen(global_log_filename.c_str(), "a+");
    if (fd == NULL)
    {
        printf("[-] open log file failed,file: %s", global_log_filename.c_str());
        return false;
    }
    size_t writelength = fwrite(str.c_str(), 1, str.length(), fd);
    if (writelength != str.length())
    {
        printf("[-] write string to log file failed,file: %s", global_log_filename.c_str());
        fclose(fd);
        return false;
    }
    fflush(fd);
    fclose(fd);
    return true;
}

void Log2Console(string str)
{
    cout << str;
}

// int global_log_method = 0;//记录日志方式：0-仅显示错误（标准输出），1-全部显示（标准输出），2-仅显示错误（文件），3-全部显示（文件）
//loglevel : 1---[i]---info,2---[-]---error,3---[+]---success,4---[notify]---callback notify error,5---[notify]---callback notify success
void Log(string message, int loglevel = 1)
{
    string logtext = "";
    switch (loglevel)
    {
    case 1:
        logtext += "[i] ";
        break;
    case 2:
        logtext += "[-] ";
        break;
    case 3:
        logtext += "[+] ";
        break;
    case 4:
        logtext += "[notify] ";
        break;
    case 5:
        logtext += "[notify] ";
        break;
    default:
        break;
    }
    logtext += global_testinfo;
    logtext += " ---> ";
    logtext += message;
    logtext += "\n";
    if (global_log_method == 0){
        if (loglevel == 2 || loglevel == 4){
            Log2Console(logtext);
        }
    }
    else if(global_log_method == 1){
        Log2Console(logtext);
    }
    else if (global_log_method == 2){
        if (loglevel == 2 || loglevel == 4){
            Log2File(logtext);
        }
    }
    else if(global_log_method == 3){
        Log2File(logtext);
    }
    else{
        return;
    }
}
//[-] chapter 01 - test 01 ---> test for something failed

void print_test_failed_message(const char *error_message, const char *test_number = "")
{
    string str = "";
    str += test_number;
    str += " --- test for ";
    str += error_message;
    str += " failed";
    Log(str, 2);
    // printf("[-] %s --- test for %s failed\n",test_number,error_message);
}

uint32_t testdatanotify(const char *data, uint32_t length, int32_t flag, void *ptr_param)
{
    cout << "[testdatanotify] data : " << data << "length : " << length << "flag : " << flag << "ptr_param :" << ptr_param << endl;
    return flag;
}

//正常扫描回调函数
uint32_t testdatanotify_scan(const char *data, uint32_t length, int32_t flag, void *ptr_param)
{
    cout << "[testdatanotify_scan] data : " << data << "length : " << length << "flag : " << flag << "ptr_param :" << ptr_param << endl;
    golbal_scancount++;
    return flag;
}

uint32_t testdatanotify_scan_01(const char *data, uint32_t length, int32_t flag, void *ptr_param)
{
    if (ptr_param != global_ptr)
    {
        Log("触发回调函数，回调函数的ptr_param参数和av_scan_target传入的相等", 4);
    }
    else
    {
        Log("触发回调函数，回调函数的ptr_param参数和av_scan_target传入的相等", 5);
    }
    cout << "[testdatanotify_scan_01] data : " << data << "length : " << length << "flag : " << flag << "ptr_param :" << ptr_param << endl;
    return flag;
}
// chapter 02 - test 08,todo
uint32_t testdatanotify_scan_02(const char *data, uint32_t length, int32_t flag, void *ptr_param)
{
    cout << "[datanotify] data : " << data << "length : " << length << "flag : " << flag << "ptr_param :" << ptr_param << endl;
    if (data != nullptr)
    {
        Log("返回json非空", 3);
    }
    else
    {
        Log("返回json为空", 2);
    }
    return flag;
}
// chapter 02 - test 09,todo
uint32_t testdatanotify_scan_03(const char *data, uint32_t length, int32_t flag, void *ptr_param)
{
    cout << "[datanotify] data : " << data << "length : " << length << "flag : " << flag << "ptr_param :" << ptr_param << endl;
    if (data != nullptr)
    {
        Log("返回json非空", 3);
    }
    else
    {
        Log("返回json为空", 2);
    }
    return flag;
}
// chapter 03 - test 10,todo
uint32_t testdatanotify_scan_04(const char *data, uint32_t length, int32_t flag, void *ptr_param)
{
    cout << "[datanotify] data : " << data << "length : " << length << "flag : " << flag << "ptr_param :" << ptr_param << endl;
    golbal_scancount++;
    if (data != nullptr)
    {
        Log("返回json非空", 3);
    }
    else
    {
        Log("返回json为空", 2);
    }
    return flag;
}
// chapter 03 - test 11,todo
uint32_t testdatanotify_scan_05(const char *data, uint32_t length, int32_t flag, void *ptr_param)
{
    cout << "[datanotify] data : " << data << "length : " << length << "flag : " << flag << "ptr_param :" << ptr_param << endl;
    golbal_scancount++;
    if (data != nullptr)
    {
        Log("返回json非空", 3);
    }
    else
    {
        Log("返回json为空", 2);
    }
    return flag;
}

//正常隔离区枚举回调函数,todo
uint32_t testdatanotify_list(const char *data, uint32_t length, int32_t flag, void *ptr_param)
{
    cout << "[datanotify] data : " << data << "length : " << length << "flag : " << flag << "ptr_param :" << ptr_param << endl;
    global_quarantinefilecount++;
    global_virusnewfilename = "neeeeeee";
    return flag;
}
//chapter 04 - test 07,todo
uint32_t testdatanotify_list_01(const char *data, uint32_t length, int32_t flag, void *ptr_param)
{
    cout << "[datanotify] data : " << data << "length : " << length << "flag : " << flag << "ptr_param :" << ptr_param << endl;
    global_quarantinefilecount++;
    global_virusnewfilename = "neeeeeee";
    if (data != nullptr)
    {
        Log("返回json非空", 3);
    }
    else
    {
        Log("返回json为空", 2);
    }
    return flag;
}
// chapter 04 - test 08,todo
uint32_t testdatanotify_list_02(const char *data, uint32_t length, int32_t flag, void *ptr_param)
{
    cout << "[datanotify] data : " << data << "length : " << length << "flag : " << flag << "ptr_param :" << ptr_param << endl;
    global_quarantinefilecount++;
    global_virusnewfilename = "neeeeeee";
    if (data != nullptr)
    {
        Log("返回json非空", 3);
    }
    else
    {
        Log("返回json为空", 2);
    }
    return flag;
}
// chapter 04 - test 10,todo
uint32_t testdatanotify_list_03(const char *data, uint32_t length, int32_t flag, void *ptr_param)
{
    cout << "[datanotify] data : " << data << "length : " << length << "flag : " << flag << "ptr_param :" << ptr_param << endl;
    global_quarantinefilecount++;
    global_virusnewfilename = "neeeeeee";
    if (data != nullptr)
    {
        Log("返回json非空", 3);
    }
    else
    {
        Log("返回json为空", 2);
    }
    return flag;
}

void test_file_count()
{
    Log("----- test file count -----", 1);
    string cmd = "ls -lR ";
    cmd += global_scanpath;
    cmd += " |grep \"^-\"|wc -l";
    string ret = ShellCommand(cmd, 30);
    global_filecount = stoi(ret);
    Log("----- test file count finish, result : " + to_string(global_filecount) + " files in " + global_scanpath, 1);
}

void test_scan_time()
{
    Log("----- test scan time -----\n", 1);
    clock_t start, end;
    start = clock();
    golbal_scancount = 0;
    void *ptr = av_sdk_init();
    uint32_t ret = av_scan_target(ptr, global_scanoption, global_scanpath.c_str(), testdatanotify_scan, nullptr);
    while (golbal_scancount != global_filecount)
    {
        sleep(5);
    }
    av_sdk_uninit(ptr);
    end = clock();
    // e:计算结果  a：被除数  b：除数:
    // 1(四舍五入) ： e=(a+(b/2))/b
    // 2(进一法)    ： e=(a+(b-1))/b
    //seconds : (double)(end-start)/CLOCKS_PER_SEC
    //minutes : (double)(end-start)/CLOCKS_PER_SEC/60
    //minutes : (int)((double)(end-start)/CLOCKS_PER_SEC+59)/60 进1法
    global_waitminute = (int)((double)(end - start) / CLOCKS_PER_SEC + 59) / 60;
    string str = "----- test scan time finish, result : scan ";
    str += global_scanpath;
    str += "for " + to_string((double)(end - start) / CLOCKS_PER_SEC) + " seconds, set to " + to_string(global_waitminute) + " to make sure it can be finished in this time -----";
    Log(str, 1);
}

void restart_daemon()
{
    string cmd = "";
    cmd += global_restart_main_daemon_script_path_with_name;
    cmd += " ";
    cmd += global_daemon_path_with_name;
    ShellCommand(cmd, 30);
}

void modify_config_mount_kv_plugin()
{
    string cmd = "";
    cmd += global_run_modify_load_path_with_name;
    cmd += " -f ";
    cmd += global_daemon_config_xml_path_with_name;
    cmd += " -m ";
    cmd += global_kv_plugin_name;
    cmd += " -l y";
    ShellCommand(cmd, 30);
}
void modify_config_not_mount_kv_plugin()
{
    string cmd = "";
    cmd += global_run_modify_load_path_with_name;
    cmd += " -f ";
    cmd += global_daemon_config_xml_path_with_name;
    cmd += " -m ";
    cmd += global_kv_plugin_name;
    cmd += " -l n";
    ShellCommand(cmd, 30);
}

//检查运行环境，成功返回1，否则为0
int check_environment()
{
    // global_daemon_path_with_name = str1.c_str();
    // global_daemon_config_xml_path_with_name = str2.c_str();
    // global_run_modify_load_path_with_name = str3.c_str();
    // global_kv_plugin_name = str4.c_str();
    // global_restart_main_daemon_script_path_with_name = str5.c_str();
    // global_scanpath = str6.c_str();
    // global_specialpath = str7.c_str();
    // global_scanoption = EFlags2Int(str8);
    // global_log_method = atoi(str9.c_str());
    if (!file_exist(global_daemon_path_with_name) || !file_exist(global_daemon_config_xml_path_with_name) || !file_exist(global_run_modify_load_path_with_name) || !file_exist(global_restart_main_daemon_script_path_with_name) || !folder_exist(global_scanpath) || !folder_exist(global_specialpath)){
        Log("file or folder not exist", 2);
        return 0;
    }
    string cmd = "ps -ef | grep $(basename ";
    cmd += global_daemon_path_with_name;
    cmd += ") | wc -l";
    string ret = ShellCommand(cmd, 30);
    if (stoi(ret) > 1)
    {
        Log("main_daemon started, check passed", 3);
        return 1;
    }
    else
    {
        Log(("main_daemon not started, check failed, you should run " + global_daemon_path_with_name).c_str(), 2);
        return 0;
    }
}

// chapter 01
void TEST_av_sdk_init_and_av_sdk_uninit()
{
    FUNCTIONSTARTINFO;
    void *ptr = nullptr;
    // test 01 修改配置文件不挂载杀毒引擎插件；重启Daemon；调用av_sdk_init，返回NULL。
    global_testinfo = "chapter 01 - test 01";
    modify_config_not_mount_kv_plugin();
    restart_daemon();
    ptr = av_sdk_init();
    if (ptr != nullptr)
    {
        Log("修改配置文件不挂载杀毒引擎插件；重启Daemon；调用av_sdk_init，返回NULL", 2);
    }
    else
    {
        Log("修改配置文件不挂载杀毒引擎插件；重启Daemon；调用av_sdk_init，返回NULL", 3);
    }
    // test 02 修改配置文件挂载杀毒引擎插件；重启Daemon；调用av_sdk_init获得指针；以此指针调用av_sdk_uninit，返回1。
    global_testinfo = "chapter 01 - test 02";
    modify_config_mount_kv_plugin();
    restart_daemon();
    ptr = av_sdk_init();
    if (av_sdk_uninit(ptr) != 1)
    {
        Log("修改配置文件挂载杀毒引擎插件；重启Daemon；调用av_sdk_init获得指针；以此指针调用av_sdk_uninit，返回1", 2);
    }
    else
    {
        Log("修改配置文件挂载杀毒引擎插件；重启Daemon；调用av_sdk_init获得指针；以此指针调用av_sdk_uninit，返回1", 3);
    }
    //test 03 用空指针调用av_sdk_uninit，程序崩溃。
    global_testinfo = "chapter 01 - test 03";
    printf("[i] try call av_sdk_uninit with nullptr...\n");
    av_sdk_uninit(nullptr);
    Log("用空指针调用av_sdk_uninit，程序崩溃     没有提供接口，有什么现象？", 1);
    printf("[i] try call av_sdk_uninit with nullptr done,so what happened?\n");
}

void single_test_av_scan_target(void *ptr_sdk, uint32_t option, DataNotify datanotify, const char *path, void *ptr_param, uint32_t expect_pass_flag, const char *test_message, bool use_passed_ptr_sdk = false)
{
    void *ptr = av_sdk_init();
    uint32_t ret_target = 0;
    if (use_passed_ptr_sdk)
    {
        ret_target = av_scan_target(ptr_sdk, option, path, datanotify, ptr_param);
        // ret_target = av_scan_target(ptr_sdk,option,testscannotify,path,ptr_param);
    }
    else
    {
        ret_target = av_scan_target(ptr, option, path, datanotify, ptr_param);
        // ret_target = av_scan_target(ptr,option,testscannotify,path,ptr_param);
    }
    string str = "";
    if (ret_target != expect_pass_flag)
    {
        str += "expect " + to_string(expect_pass_flag) + " but return " + to_string(ret_target) + " while test for " + test_message;
        Log(str, 2);
        // printf("[-] test FAILED : expect %u but return %u while test for %s \n",expect_pass_flag,ret_target,test_message);
    }
    else
    {
        str += "expect " + to_string(expect_pass_flag) + " but return " + to_string(ret_target) + " while test for " + test_message;
        Log(str, 3);
    }
    if (!ptr)
    {
        av_sdk_uninit(ptr);
    }
}

// chapter 02
void TEST_av_scan_target()
{
    FUNCTIONSTARTINFO;
    // void *ptr = av_sdk_init();
    uint32_t option = global_scanoption;
    const char *path = global_scanpath.c_str();
    uint32_t ret_target = 0;
    void *test_ptr = NULL;
    global_ptr = test_ptr;
    //test 01 以ptr_sdk为空指针调用av_scan_target，程序崩溃
    global_testinfo = "chapter 02 - test 01";
    single_test_av_scan_target(nullptr, option, testdatanotify_scan, path, test_ptr, 0, "以ptr_sdk为空指针调用av_scan_target，程序崩溃", true);
    //test 02
    global_testinfo = "chapter 02 - test 02";
    single_test_av_scan_target(nullptr, 0, testdatanotify_scan, path, test_ptr, 0, "以uint32_option为0而其它参数正常调用av_scan_target，返回0", false);
    //test 03
    global_testinfo = "chapter 02 - test 03";
    single_test_av_scan_target(nullptr, option, testdatanotify_scan, nullptr, test_ptr, 0, "以ptr_path为NULL而其它参数正常调用av_scan_target，返回0", false);
    //test 04
    global_testinfo = "chapter 02 - test 04";
    single_test_av_scan_target(nullptr, option, nullptr, path, test_ptr, 1, "以ptr_notify为NULL而其它参数正常调用av_scan_target，返回1", false);
    //test 05
    global_testinfo = "chapter 02 - test 05";
    single_test_av_scan_target(nullptr, option, testdatanotify_scan, path, nullptr, 1, "以ptr_param为NULL而其它参数正常调用av_scan_target，返回1", false);
    //test 06
    global_testinfo = "chapter 02 - test 06";
    single_test_av_scan_target(nullptr, option, testdatanotify_scan, path, test_ptr, 1, "以所有参数正常时调用av_scan_target，返回1", false);
    //test 07 --- 以所有参数正常时调用av_scan_target，返回1；触发回调函数，回调函数的ptr_param参数和av_scan_target传入的相等。
    global_testinfo = "chapter 02 - test 07";
    single_test_av_scan_target(nullptr, option, testdatanotify_scan_01, path, test_ptr, 1, "以所有参数正常时调用av_scan_target，返回1", false);
    //test 08 --- 以所有参数正常时调用av_scan_target，返回1；触发回调函数，回调函数的file参数不为NULL。
    global_testinfo = "chapter 02 - test 08";
    single_test_av_scan_target(nullptr, option, testdatanotify_scan_02, path, test_ptr, 1, "以所有参数正常时调用av_scan_target，返回1", false);
    //test 09 --- 以所有参数正常时调用av_scan_target，返回1；触发回调函数，回调函数的flag参数不为0时，description参数不为NULL。
    global_testinfo = "chapter 02 - test 09";
    single_test_av_scan_target(nullptr, option, testdatanotify_scan_03, path, test_ptr, 1, "以所有参数正常时调用av_scan_target，返回1", false);
}

// chapter 03
void TEST_av_scan_stop_and_av_sdk_pause_and_av_scan_resume()
{
    FUNCTIONSTARTINFO;
    const char *test_number = "";
    golbal_scancount = 0;
    void *ptr = av_sdk_init();
    if (ptr == nullptr)
    {
        Log("TEST_av_scan_stop_and_av_sdk_pause_and_av_scan_resume", 1);
        return;
    }
    global_ptr = ptr;
    uint32_t option = global_scanoption;
    const char *path = global_scanpath.c_str();
    uint32_t ret_status = 0;
    ret_status = av_scan_target(ptr, option, path, testdatanotify_scan, nullptr);
    //test 01
    global_testinfo = "chapter 03 - test 01";
    ret_status = av_sdk_stop(nullptr);
    if (ret_status == 1)
    {
        Log("以空指针调用av_sdk_stop，程序崩溃。", 2);
    }
    else
    {
        Log("以空指针调用av_sdk_stop，程序崩溃。", 3);
    }
    //test 02
    global_testinfo = "chapter 03 - test 02";
    ret_status = av_sdk_pause(nullptr);
    if (ret_status == 1)
    {
        Log("以空指针调用av_sdk_pause，程序崩溃。", 2);
    }
    else
    {
        Log("以空指针调用av_sdk_pause，程序崩溃。", 3);
    }
    //tesst 03
    global_testinfo = "chapter 03 - test 03";
    ret_status = av_sdk_resume(nullptr);
    if (ret_status == 1)
    {
        Log("以空指针调用av_sdk_resume，程序崩溃。", 2);
    }
    else
    {
        Log("以空指针调用av_sdk_resume，程序崩溃。", 3);
    }
    //test 04 重启Daemon；在未调用av_scan_target时，以正常参数调用av_sdk_stop，返回0。
    global_testinfo = "chapter 03 - test 04";
    restart_daemon();
    if (av_sdk_stop(ptr) != 0)
    {
        Log("重启Daemon；在未调用av_scan_target时，以正常参数调用av_sdk_stop，返回0", 2);
    }
    else
    {
        Log("重启Daemon；在未调用av_scan_target时，以正常参数调用av_sdk_stop，返回0", 3);
    }
    //test 05 重启Daemon；在未调用av_scan_target时，以正常参数调用av_sdk_pause，返回0。
    global_testinfo = "chapter 03 - test 05";
    restart_daemon();
    if (av_sdk_pause(ptr) != 0)
    {
        Log("重启Daemon；在未调用av_scan_target时，以正常参数调用av_sdk_pause，返回0。", 2);
    }
    else
    {
        Log("重启Daemon；在未调用av_scan_target时，以正常参数调用av_sdk_pause，返回0。", 3);
    }
    //test 06 重启Daemon；在未调用av_scan_target时，以正常参数调用av_sdk_resume，返回0。
    global_testinfo = "chapter 03 - test 06";
    restart_daemon();
    if (av_sdk_resume(ptr) != 0)
    {
        Log("重启Daemon；在未调用av_scan_target时，以正常参数调用av_sdk_resume，返回0。", 2);
    }
    else
    {
        Log("重启Daemon；在未调用av_scan_target时，以正常参数调用av_sdk_resume，返回0。", 3);
    }
    //test 07 以正常参数调用av_scan_target；在确认扫描结束之后，以正常参数调用av_sdk_stop，返回0。
    global_testinfo = "chapter 03 - test 07";
    av_sdk_stop(ptr);
    av_scan_target(ptr, option, path, testdatanotify_scan, nullptr);
    sleep(global_waitminute * 60);
    ret_status = av_sdk_stop(ptr);
    if (ret_status != 0)
    {
        Log("以正常参数调用av_scan_target；在确认扫描结束之后，以正常参数调用av_sdk_stop，返回0", 2);
    }
    else
    {
        Log("以正常参数调用av_scan_target；在确认扫描结束之后，以正常参数调用av_sdk_stop，返回0", 3);
    }
    //test 08 以正常参数调用av_scan_target；在确认扫描结束之后，以正常参数调用av_sdk_pause，返回0。
    global_testinfo = "chapter 03 - test 08";
    av_sdk_stop(ptr);
    av_scan_target(ptr, option, path, testdatanotify_scan, nullptr);
    sleep(global_waitminute * 60);
    ret_status = av_sdk_pause(ptr);
    if (ret_status != 0)
    {
        Log("以正常参数调用av_scan_target；在确认扫描结束之后，以正常参数调用av_sdk_pause，返回0", 2);
    }
    else
    {
        Log("以正常参数调用av_scan_target；在确认扫描结束之后，以正常参数调用av_sdk_pause，返回0", 3);
    }
    //test 09 以正常参数调用av_scan_target；在确认扫描结束之后，以正常参数调用av_sdk_resume，返回0。
    global_testinfo = "chapter 03 - test 09";
    av_sdk_stop(ptr);
    av_scan_target(ptr, option, path, testdatanotify_scan, nullptr);
    sleep(global_waitminute * 60);
    ret_status = av_sdk_resume(ptr);
    if (ret_status != 0)
    {
        Log("以正常参数调用av_scan_target；在确认扫描结束之后，以正常参数调用av_sdk_resume，返回0", 2);
    }
    else
    {
        Log("以正常参数调用av_scan_target；在确认扫描结束之后，以正常参数调用av_sdk_resume，返回0", 3);
    }
    //test 10 以正常参数调用av_scan_target扫描含有两个以上文件的目录，在第一次触发回调函数时，在回调函数中以正常参数调用av_sdk_stop，返回1。且后续不再有回调函数触发。
    global_testinfo = "chapter 03 - test 10";
    av_sdk_stop(ptr);
    golbal_scancount = 0;
    ret_status = av_scan_target(ptr, option, path, testdatanotify_scan_04, nullptr);
    // ret_status = av_scan_target(ptr,option,testscannotify5,path,nullptr);
    while (golbal_scancount != 1)
    {
        av_sdk_stop(ptr);
        golbal_scancount = 0;
        ret_status = av_scan_target(ptr, option, path, testdatanotify_scan_04, nullptr);
        sleep(1);
    }
    golbal_scancount = 0;
    sleep(global_waitminute * 60);
    if (golbal_scancount != 0)
    {
        Log("以正常参数调用av_scan_target扫描含有两个以上文件的目录，在第一次触发回调函数时，在回调函数中以正常参数调用av_sdk_stop，返回1。且后续不再有回调函数触发", 2);
    }
    else
    {
        Log("以正常参数调用av_scan_target扫描含有两个以上文件的目录，在第一次触发回调函数时，在回调函数中以正常参数调用av_sdk_stop，返回1。且后续不再有回调函数触发", 3);
    }
    //test 11 以正常参数调用av_scan_target扫描含有两个以上文件的目录，在第一次触发回调函数时，在回调函数中以正常参数调用av_sdk_pause，返回1；第一次回调结束后，后续不再有回调函数触发；以正常参数调用av_sdk_resume，后续有回调函数触发，且后续回调函数触发的总个数等于文件个数减1。
    global_testinfo = "chapter 03 - test 11";
    av_sdk_stop(ptr);
    golbal_scancount = 0;
    ret_status = av_scan_target(ptr, option, path, testdatanotify_scan_05, nullptr);
    // ret_status = av_scan_target(ptr,option,testscannotify4,path,nullptr);
    while (golbal_scancount != 1)
    {
        av_sdk_stop(ptr);
        golbal_scancount = 0;
        // ret_status = av_scan_target(ptr,option,testscannotify4,path,nullptr);
        ret_status = av_scan_target(ptr, option, path, testdatanotify_scan_05, nullptr);
        sleep(1);
    }
    golbal_scancount = 0;
    sleep(global_waitminute * 60);
    if (golbal_scancount != 0)
    {
        Log("以正常参数调用av_scan_target扫描含有两个以上文件的目录，在第一次触发回调函数时，在回调函数中以正常参数调用av_sdk_pause，返回1；第一次回调结束后，后续不再有回调函数触发", 2);
    }
    else
    {
        Log("以正常参数调用av_scan_target扫描含有两个以上文件的目录，在第一次触发回调函数时，在回调函数中以正常参数调用av_sdk_pause，返回1；第一次回调结束后，后续不再有回调函数触发", 3);
    }
    golbal_scancount = 0;
    ret_status = av_sdk_resume(ptr);
    if (ret_status != 1)
    {
        Log("以正常参数调用av_scan_target扫描含有两个以上文件的目录，在第一次触发回调函数时，在回调函数中以正常参数调用av_sdk_pause，返回1；第一次回调结束后，后续不再有回调函数触发；以正常参数调用av_sdk_resume", 2);
    }
    else
    {
        Log("以正常参数调用av_scan_target扫描含有两个以上文件的目录，在第一次触发回调函数时，在回调函数中以正常参数调用av_sdk_pause，返回1；第一次回调结束后，后续不再有回调函数触发；以正常参数调用av_sdk_resume", 3);
    }
    sleep(global_waitminute * 60);
    if (golbal_scancount == 0)
    {
        Log("以正常参数调用av_scan_target扫描含有两个以上文件的目录，在第一次触发回调函数时，在回调函数中以正常参数调用av_sdk_pause，返回1；第一次回调结束后，后续不再有回调函数触发；以正常参数调用av_sdk_resume，后续有回调函数触发，后续有回调函数触发", 2);
    }
    else
    {
        Log("以正常参数调用av_scan_target扫描含有两个以上文件的目录，在第一次触发回调函数时，在回调函数中以正常参数调用av_sdk_pause，返回1；第一次回调结束后，后续不再有回调函数触发；以正常参数调用av_sdk_resume，后续有回调函数触发，后续有回调函数触发", 3);
    }
    if (golbal_scancount != global_filecount - 1)
    {
        Log("以正常参数调用av_scan_target扫描含有两个以上文件的目录，在第一次触发回调函数时，在回调函数中以正常参数调用av_sdk_pause，返回1；第一次回调结束后，后续不再有回调函数触发；以正常参数调用av_sdk_resume，后续有回调函数触发，且后续回调函数触发的总个数等于文件个数减1", 2);
    }
    else
    {
        Log("以正常参数调用av_scan_target扫描含有两个以上文件的目录，在第一次触发回调函数时，在回调函数中以正常参数调用av_sdk_pause，返回1；第一次回调结束后，后续不再有回调函数触发；以正常参数调用av_sdk_resume，后续有回调函数触发，且后续回调函数触发的总个数等于文件个数减1", 3);
    }
    //test 12 以正常参数调用av_scan_target；在确认扫描结束之前，在回调函数之外，以正常参数调用av_sdk_stop，返回1；后续不再有回调函数触发。
    global_testinfo = "chapter 03 - test 12";
    av_sdk_stop(ptr);
    ret_status = av_scan_target(ptr, option, path, testdatanotify_scan, nullptr);
    // ret_status = av_scan_target(ptr,option,testscannotify,path,nullptr);
    while (golbal_scancount != global_filecount)
    {
        printf("[i] scan not finished, wait for 10 seconds");
        ret_status = av_sdk_stop(ptr);
        if (ret_status != 1)
        {
            Log("以正常参数调用av_scan_target；在确认扫描结束之前，在回调函数之外，以正常参数调用av_sdk_stop，返回1", 2);
        }
        else
        {
            Log("以正常参数调用av_scan_target；在确认扫描结束之前，在回调函数之外，以正常参数调用av_sdk_stop，返回1", 3);
        }
        av_sdk_resume(ptr);
        sleep(10);
    }
    golbal_scancount = 0;
    sleep(global_waitminute * 60);
    if (golbal_scancount != 0)
    {
        Log("以正常参数调用av_scan_target；在确认扫描结束之前，在回调函数之外，以正常参数调用av_sdk_stop，返回1；后续不再有回调函数触发。", 2);
    }
    else
    {
        Log("以正常参数调用av_scan_target；在确认扫描结束之前，在回调函数之外，以正常参数调用av_sdk_stop，返回1；后续不再有回调函数触发。", 3);
    }
    //test 13 以正常参数调用av_scan_target；在确认扫描结束之后，在回调函数之外，以正常参数调用av_sdk_pause，返回1；后续不再有回调函数触发；以正常参数调用av_sdk_resume，后续有回调函数触发，且总回调函数个数等于文件个数。
    global_testinfo = "chapter 03 - test 13";
    av_sdk_stop(ptr);
    ret_status = av_scan_target(ptr, option, path, testdatanotify_scan, nullptr);
    // ret_status = av_scan_target(ptr,option,testscannotify,path,nullptr);
    while (golbal_scancount != global_filecount)
    {
        Log("scan not finished, wait for 10 seconds", 1);
        sleep(10);
    }
    golbal_scancount = 0;
    ret_status = av_sdk_pause(ptr);
    if (ret_status != 1)
    {
        Log("以正常参数调用av_scan_target；在确认扫描结束之后，在回调函数之外，以正常参数调用av_sdk_pause，返回1", 2);
    }
    else
    {
        Log("以正常参数调用av_scan_target；在确认扫描结束之后，在回调函数之外，以正常参数调用av_sdk_pause，返回1", 3);
    }
    sleep(global_waitminute * 60);
    if (golbal_scancount != 0)
    {
        Log("以正常参数调用av_scan_target；在确认扫描结束之后，在回调函数之外，以正常参数调用av_sdk_pause，返回1；后续不再有回调函数触发", 2);
    }
    else
    {
        Log("以正常参数调用av_scan_target；在确认扫描结束之后，在回调函数之外，以正常参数调用av_sdk_pause，返回1；后续不再有回调函数触发", 3);
    }
    golbal_scancount = 0;
    ret_status = av_sdk_resume(ptr);
    sleep(global_waitminute * 60);
    if (golbal_scancount == 0)
    {
        Log("以正常参数调用av_scan_target；在确认扫描结束之后，在回调函数之外，以正常参数调用av_sdk_pause，返回1；后续不再有回调函数触发；以正常参数调用av_sdk_resume，后续有回调函数触发", 2);
    }
    else
    {
        Log("以正常参数调用av_scan_target；在确认扫描结束之后，在回调函数之外，以正常参数调用av_sdk_pause，返回1；后续不再有回调函数触发；以正常参数调用av_sdk_resume，后续有回调函数触发", 3);
    }
    if (golbal_scancount != global_filecount)
    {
        Log("以正常参数调用av_scan_target；在确认扫描结束之后，在回调函数之外，以正常参数调用av_sdk_pause，返回1；后续不再有回调函数触发；以正常参数调用av_sdk_resume，后续有回调函数触发，且总回调函数个数等于文件个数。", 2);
    }
    else
    {
        Log("以正常参数调用av_scan_target；在确认扫描结束之后，在回调函数之外，以正常参数调用av_sdk_pause，返回1；后续不再有回调函数触发；以正常参数调用av_sdk_resume，后续有回调函数触发，且总回调函数个数等于文件个数。", 3);
    }
    //擦屁股
    if (!ptr)
    {
        av_sdk_uninit(ptr);
    }
}

// chapter 04
void TEST_av_list_file_in_guarantine_and_av_restore_isolation()
{
    FUNCTIONSTARTINFO;
    const char *test_number = "";
    uint32_t ret_status = 0;
    void *ptr_sdk = av_sdk_init();
    global_ptr = ptr_sdk;
    int retry_count = 0;
    // void * ptr = nullptr;
    global_quarantinefilecount = 0;
    //test 01 以ptr_sdk为空指针调用av_list_isolation，程序崩溃。
    global_testinfo = "chapter 04 - test 01";
    ret_status = av_list_isolation(nullptr, testdatanotify_list, nullptr);
    // ret_status = av_list_isolation(nullptr,testlistnotify);
    if (ret_status != 0)
    {
        Log("以ptr_sdk为空指针调用av_list_isolation，程序崩溃。", 2);
    }
    else
    {
        Log("以ptr_sdk为空指针调用av_list_isolation，程序崩溃。", 3);
    }
    //test 02 以ptr_sdk为空指针调用av_restore_isolation，程序崩溃。
    global_testinfo = "chapter 04 - test 02";
    ret_status = av_restore_isolation(nullptr, "");
    if (ret_status != 0)
    {
        Log("以ptr_sdk为空指针调用av_restore_isolation，程序崩溃", 2);
    }
    else
    {
        Log("以ptr_sdk为空指针调用av_restore_isolation，程序崩溃", 3);
    }
    //test 03 以ptr_notify为空指针其它参数正常调用av_list_isolation，返回0。
    global_testinfo = "chapter 04 - test 03";
    ret_status = av_list_isolation(ptr_sdk, nullptr, nullptr);
    if (ret_status != 0)
    {
        Log("以ptr_notify为空指针其它参数正常调用av_list_isolation，返回0。", 2);
    }
    else
    {
        Log("以ptr_notify为空指针其它参数正常调用av_list_isolation，返回0。", 3);
    }
    //test 04 以ptr_name为空指针其它参数正常调用av_restore_isolation，返回0。
    global_testinfo = "chapter 04 - test 04";
    ret_status = av_restore_isolation(ptr_sdk, nullptr);
    if (ret_status != 0)
    {
        Log("以ptr_name为空指针其它参数正常调用av_restore_isolation，返回0", 2);
    }
    else
    {
        Log("以ptr_name为空指针其它参数正常调用av_restore_isolation，返回0", 3);
    }
    //test 05 以所有参数正常时调用av_list_isolation，返回1。
    global_testinfo = "chapter 04 - test 05";
    global_quarantinefilecount = 0;
    global_virusnewfilename = nullptr;
    ret_status = av_list_isolation(ptr_sdk, testdatanotify_list, nullptr);
    if (ret_status != 1)
    {
        Log("以ptr_sdk为空指针调用av_list_isolation，程序崩溃", 2);
    }
    else
    {
        Log("以ptr_sdk为空指针调用av_list_isolation，程序崩溃", 3);
    }
    //test 06 以所有参数正常时调用av_restore_isolation，返回1。
    global_testinfo = "chapter 04 - test 06";
    while (global_virusnewfilename.c_str() == nullptr)
    {
        sleep(1);
    }
    ret_status = av_restore_isolation(ptr_sdk, global_virusnewfilename.c_str());
    if (ret_status != 1)
    {
        Log("以ptr_sdk为空指针调用av_restore_isolation，程序崩溃", 2);
    }
    else
    {
        Log("以ptr_sdk为空指针调用av_restore_isolation，程序崩溃", 3);
    }
    //test 07 调用av_scan_target扫描一个病毒文件，保证有文件被隔离；以所有参数正常时调用av_list_isolation，返回1；触发ListNotify回调函数时，函数参数file_old、file_new、sha不为空，size参数不为0。
    global_testinfo = "chapter 04 - test 07";
    golbal_scancount = 0;
    while (golbal_scancount != 1)
    {
        ret_status = av_scan_target(ptr_sdk, global_scanoption, global_specialpath.c_str(), testdatanotify_scan, nullptr);
        // ret_status = av_scan_target(ptr_sdk,global_scanoption,testscannotify,global_specialpath,nullptr);
        if (ret_status != 1)
        {
            golbal_scancount = 0;
            Log("调用av_scan_target扫描一个病毒文件", 2);
        }
        if (retry_count == global_retrycount)
        {
            retry_count = 0;
            Log("[retry] 调用av_scan_target扫描一个病毒文件", 2);
        }
    }
    ret_status = av_list_isolation(ptr_sdk, testdatanotify_list_01, nullptr);
    if (ret_status != 1)
    {
        Log("保证有文件被隔离；以所有参数正常时调用av_list_isolation，返回1", 2);
    }
    //test 08 调用av_scan_target扫描一个病毒文件，保证有文件被隔离；以所有参数正常时调用av_list_isolation，返回1；触发ListNotify回调函数时，如果file_old指向的文件不存在，以正常参数调用av_restore_isolation，返回1。确认file_old指向的文件存在。
    global_testinfo = "chapter 04 - test 08";
    golbal_scancount = 0;
    if (av_restore_isolation(ptr_sdk, global_virusnewfilename.c_str()) != 1)
    {
        printf("[-] restore last test environment failed\n");
    }
    while (golbal_scancount != 1)
    {
        ret_status = av_scan_target(ptr_sdk, global_scanoption, global_specialpath.c_str(), testdatanotify_scan, nullptr);
        // ret_status = av_scan_target(ptr_sdk,global_scanoption,testscannotify,global_specialpath,nullptr);
        if (ret_status != 1)
        {
            golbal_scancount = 0;
            Log("调用av_scan_target扫描一个病毒文件", 2);
        }
        if (retry_count == global_retrycount)
        {
            retry_count = 0;
            Log("[retry] 调用av_scan_target扫描一个病毒文件 failed\n", 2);
        }
    }
    ret_status = av_list_isolation(ptr_sdk, testdatanotify_list_02, nullptr);
    if (ret_status != 1)
    {
        Log("保证有文件被隔离；以所有参数正常时调用av_list_isolation，返回1", 2);
    }
    //test 09 调用av_scan_target扫描一个病毒文件，保证有文件被隔离；以所有参数正常时调用av_list_isolation，返回1；触发ListNotify回调函数时，如果file_old指向的文件不存在，获取file_new；以获取的file_new为参数，其它参数正常调用av_restore_isolation，返回1。确认file_old指向的文件存在。
    global_testinfo = "chapter 04 - test 09";
    printf("[i] test 09 is more likely test 08,not test\n");
    //test 10 调用av_scan_target扫描一个病毒文件，保证有文件被隔离；以所有参数正常时调用av_list_isolation，返回1；触发ListNotify回调函数时，如果file_old指向的文件存在，以file_new为参数，其它参数正常调用av_restore_isolation，返回1。确认file_old指向的文件已经被覆盖（修改日期）。
    global_testinfo = "chapter 04 - test 10";
    golbal_scancount = 0;
    if (av_restore_isolation(ptr_sdk, global_virusnewfilename.c_str()) != 1)
    {
        printf("[-] restore last test environment failed\n");
    }
    while (golbal_scancount != 1)
    {
        ret_status = av_scan_target(ptr_sdk, global_scanoption, global_specialpath.c_str(), testdatanotify_scan, nullptr);
        // ret_status = av_scan_target(ptr_sdk,global_scanoption,testscannotify,global_specialpath,nullptr);
        if (ret_status != 1)
        {
            golbal_scancount = 0;
            Log("调用av_scan_target扫描一个病毒文件", 2);
        }
        if (retry_count == global_retrycount)
        {
            retry_count = 0;
            Log("[retry] 调用av_scan_target扫描一个病毒文件 failed\n", 2);
        }
    }
    // ret_status = av_list_isolation(ptr_sdk,testlistnotify4);
    ret_status = av_list_isolation(ptr_sdk, testdatanotify_list_03, nullptr);
    if (ret_status != 1)
    {
        Log("保证有文件被隔离；以所有参数正常时调用av_list_isolation，返回1", 2);
    }
    //擦屁股
    if (!ptr_sdk)
    {
        av_sdk_uninit(ptr_sdk);
    }
}

void TEST_process()
{
    string str = "\n**************************";
    str += getcurrenttime();
    str += "**************************";
    Log2File(str);
    cout << "[+] start running test" << endl;
    TEST_av_sdk_init_and_av_sdk_uninit();
    TEST_av_scan_target();
    TEST_av_scan_stop_and_av_sdk_pause_and_av_scan_resume();
    TEST_av_list_file_in_guarantine_and_av_restore_isolation();
    cout << "[+] running test finished" << endl;
}


uint32_t EFlags2Int(string str){
    if (str.compare("Unzip") == 0){
        return 0x01;
    }
    else if(str.compare("Unpack") == 0){
        return 0x02;
    }
    else if(str.compare("StopOnOne") == 0){
        return 0x10;
    }
    else if(str.compare("ProgramOnly") == 0){
        return 0x100;
    }
    else if(str.compare("OriginalMd5") == 0){
        return 0x1000;
    }
    else if(str.compare("UseFinger") == 0){
        return 0x2000;
    }
    else if(str.compare("UseCloud") == 0){
        return 0x4000;
    }
    else if(str.compare("Backup") == 0){
        return 0x8000;
    }
    else if(str.compare("ForceUnzip") == 0){
        return 0x100000;
    }
    else{
        return 0;
    }
}



int get_command_option(int argc, char *argv[]){
    string helpinfo = "";
    helpinfo += "====================================================================================================================================================\n";
    helpinfo += "|                                               This application is designed for testing jiangmin kv sdk                                           |\n";
    helpinfo += "----------------------------------------------------------------------------------------------------------------------------------------------------\n";
    helpinfo += "|  Usege:                                                                                                                                          |\n";
    helpinfo += "|      --run_daemon_script=<path_with_name>                script(recommend beacause of environment variable) or program to run main_daemon        |\n";
    helpinfo += "|                                                          dafault: /home/jiangmin_antivirus/linux_kv_sdk_test/scripts/run_daemon.sh               |\n";
    helpinfo += "|      --config_xml=<path_with_name>                       main_daemon config .xml path,generally with some path relation to main_daemon           |\n";
    helpinfo += "|                                                          default: /home/jiangmin_antivirus/linux_kv_sdk_test/etc/customize_edr.xml               |\n";
    helpinfo += "|      --run_modify_load=<path_with_name>                  a small tool that modify .xml some arguments                                            |\n";
    helpinfo += "|                                                          default: /home/jiangmin_antivirus/linux_kv_sdk_test/sdk_executable/run_modify_load      |\n";
    helpinfo += "|      --kv_plugin_name=<name>                             main_daemon config .xml plugin name you want to test                                    |\n";
    helpinfo += "|                                                          default: sysinfo                                                                        |\n";
    helpinfo += "|      --restart_script=<path_with_name>                   script used to restart main_daemon                                                      |\n";
    helpinfo += "|                                                          default: /home/jiangmin_antivirus/linux_kv_sdk_test/scripts/restart_daemon.sh           |\n";
    helpinfo += "|      --scan_path=<path>                                  generally scan path,make sure at least include 2 normal files                           |\n";
    helpinfo += "|                                                          default: /home/jiangmin_antivirus/linux_kv_sdk_test/sdk_scan_path                       |\n";
    helpinfo += "|      --special_scan_path=<path>                          special scan path,make sure just include ONE virus files                                |\n";
    helpinfo += "|                                                          default: /home/jiangmin_antivirus/linux_kv_sdk_test/sdk_special_scan_path               |\n";
    helpinfo += "|      --scan_option=<option>                              scan option: Unzip(default), Unpack, StopOnOne, ProgramOnly, OriginalMd5,               |\n";
    helpinfo += "|                                                                  UseFigner, UseCloud, Backup, ForceUnzip                                         |\n";
    helpinfo += "|      --log_method=<method>                               custome log method: 0-show only errors(stdout,default), 1-show all(stdout),             |\n";
    helpinfo += "|                                                          2-show only errors(file), 3-show all(file), log file default name: sdk_test.log         |\n";
    helpinfo += "|      --help                                              print this infomation                                                                   |\n";
    helpinfo += "----------------------------------------------------------------------------------------------------------------------------------------------------\n";
    helpinfo += "|  Example:                                                                                                                                        |\n";
    helpinfo += "|  ./sdk_test.run --run_daemon_script=/path/with/name --config_xml=/path/with/name --run_modify_load=/path/with/name --kv_plugin_name=sysinfo \\    |\n";
    helpinfo += "|          --restart_script=/path/with/name --scan_path=/path --special_scan_path=/path --scan_option=Unzip --log_method=1                         |\n";
    helpinfo += "----------------------------------------------------------------------------------------------------------------------------------------------------\n";
    helpinfo += "|  author: yezhihui@jiangmin.com   codedate: 20181127                                                                                              |\n";
    helpinfo += "====================================================================================================================================================\n";
    if (argc == 1)
    {
        cout << "---- 使用默认配置（可使用--help参数查看） ---" << endl;
        return 0;
    }
    string str1 = "";
    string str2 = "";
    string str3 = "";
    string str4 = "";
    string str5 = "";
    string str6 = "";
    string str7 = "";
    string str8 = "";
    string str9 = "";
    for (int i = 1; i < argc; i++)
    {
        if (strncmp(argv[i], "--run_daemon_script=", 20) == 0)
        {
            const char *pBuffer = argv[i];
            char mbsBuffer[1024] = {0};
            char *pStart = (char *)strstr(pBuffer, "=");
            if (pStart)
            {
                pStart = pStart + strlen("=");
                int length = strlen(pStart);
                if (length < sizeof(mbsBuffer) && length > 0)
                {
                    strncpy(mbsBuffer, pStart, length);
                    if (file_exist(mbsBuffer)){
                        str1 = pStart;
                    }
                    else{
                        cout<<"run_daemon_script 指定文件"<<mbsBuffer<<"不存在"<<endl;
                        return -1;
                    }
                }
            }
        }
        else if (strncmp(argv[i], "--config_xml=", 13) == 0)
        {
            const char *pBuffer = argv[i];
            char mbsBuffer[1024] = {0};
            char *pStart = (char *)strstr(pBuffer, "=");
            if (pStart)
            {
                pStart = pStart + strlen("=");
                int length = strlen(pStart);
                if (length < sizeof(mbsBuffer) && length > 0)
                {
                    strncpy(mbsBuffer, pStart, length);
                    if (file_exist(mbsBuffer)){
                        str2 = pStart;
                    }
                    else{
                        cout<<"config_xml 指定文件"<<mbsBuffer<<"不存在"<<endl;
                        return -1;
                    }
                }
            }
        }
        else if (strncmp(argv[i], "--run_modify_load=", 18) == 0)
        {
            const char *pBuffer = argv[i];
            char mbsBuffer[1024] = {0};
            char *pStart = (char *)strstr(pBuffer, "=");
            if (pStart)
            {
                pStart = pStart + strlen("=");
                int length = strlen(pStart);
                if (length < sizeof(mbsBuffer) && length > 0)
                {
                    strncpy(mbsBuffer, pStart, length);
                    if (file_exist(mbsBuffer)){
                        str3 = pStart;
                    }
                    else{
                        cout<<"run_modify_load 指定文件"<<mbsBuffer<<"不存在"<<endl;
                        return -1;
                    }
                }
            }
        }
        else if (strncmp(argv[i], "--kv_plugin_name=", 17) == 0)
        {
            const char *pBuffer = argv[i];
            char mbsBuffer[1024] = {0};
            char *pStart = (char *)strstr(pBuffer, "=");
            if (pStart)
            {
                pStart = pStart + strlen("=");
                int length = strlen(pStart);
                if (length < sizeof(mbsBuffer) && length > 0)
                {
                    str4 = pStart;
                }
            }
        }
        else if (strncmp(argv[i], "--restart_script=", 17) == 0)
        {
            const char *pBuffer = argv[i];
            char mbsBuffer[1024] = {0};
            char *pStart = (char *)strstr(pBuffer, "=");
            if (pStart)
            {
                pStart = pStart + strlen("=");
                int length = strlen(pStart);
                if (length < sizeof(mbsBuffer) && length > 0)
                {
                    strncpy(mbsBuffer, pStart, length);
                    if (file_exist(mbsBuffer)){
                        str5 = pStart;
                    }
                    else{
                        cout<<"restart_script 指定文件"<<mbsBuffer<<"不存在"<<endl;
                        return -1;
                    }
                }
            }
        }
        else if (strncmp(argv[i], "--scan_path=", 12) == 0)
        {
            const char *pBuffer = argv[i];
            char mbsBuffer[1024] = {0};
            char *pStart = (char *)strstr(pBuffer, "=");
            if (pStart)
            {
                pStart = pStart + strlen("=");
                int length = strlen(pStart);
                if (length < sizeof(mbsBuffer) && length > 0)
                {
                    strncpy(mbsBuffer, pStart, length);
                    if (folder_exist(mbsBuffer)){
                        str6 = pStart;
                    }
                    else{
                        cout<<"scan_path 指定文件夹"<<mbsBuffer<<"不存在"<<endl;
                        return -1;
                    }
                }
            }
        }
        else if (strncmp(argv[i], "--special_scan_path=", 20) == 0)
        {
            const char *pBuffer = argv[i];
            char mbsBuffer[1024] = {0};
            char *pStart = (char *)strstr(pBuffer, "=");
            if (pStart)
            {
                pStart = pStart + strlen("=");
                int length = strlen(pStart);
                if (length < sizeof(mbsBuffer) && length > 0)
                {
                    strncpy(mbsBuffer, pStart, length);
                    if (folder_exist(mbsBuffer)){
                        str7 = pStart;
                    }
                    else{
                        cout<<"special_scan_path 指定文件夹"<<mbsBuffer<<"不存在"<<endl;
                        return -1;
                    }
                }
            }
        }
        else if (strncmp(argv[i], "--scan_option=", 14) == 0)
        {
            const char *pBuffer = argv[i];
            char mbsBuffer[1024] = {0};
            char *pStart = (char *)strstr(pBuffer, "=");
            if (pStart)
            {
                pStart = pStart + strlen("=");
                int length = strlen(pStart);
                if (length < sizeof(mbsBuffer) && length > 0)
                {
                    str8 = pStart;
                }
            }
        }
        else if (strncmp(argv[i], "--log_method=", 13) == 0)
        {
            const char *pBuffer = argv[i];
            char mbsBuffer[1024] = {0};
            char *pStart = (char *)strstr(pBuffer, "=");
            if (pStart)
            {
                pStart = pStart + strlen("=");
                int length = strlen(pStart);
                if (length < sizeof(mbsBuffer) && length > 0)
                {
                    str9 = pStart;
                }
            }
        }
        else if (strncmp(argv[i], "--help", 6) == 0)
        {
            cout << helpinfo << endl;
            return -1;
        }
        else
        {
            cout << helpinfo << endl;
            return -1;
        }
    }
    global_daemon_path_with_name = str1.c_str();
    global_daemon_config_xml_path_with_name = str2.c_str();
    global_run_modify_load_path_with_name = str3.c_str();
    global_kv_plugin_name = str4.c_str();
    global_restart_main_daemon_script_path_with_name = str5.c_str();
    global_scanpath = str6.c_str();
    global_specialpath = str7.c_str();
    global_scanoption = EFlags2Int(str8);
    global_log_method = atoi(str9.c_str());
    return 0;
}

int main(int argc, char *argv[])
{
    if (get_command_option(argc,argv) != 0){
        return -1;
    }
    global_testinfo = "check environment";
    if (!check_environment())
    {
        printf("---- environment check failed , exit ----\n");
        return -1;
    }
    printf("*** NOTE : the  philosophy of linux is : the best result is no output ***\n");
    test_file_count();
    test_scan_time();
    prinf_variblity();
    TEST_process();
    printf("---- all test finished ----\n");
    return 0;
}
