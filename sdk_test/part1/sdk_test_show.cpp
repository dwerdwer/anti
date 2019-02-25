// #include "Util.h"
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
#include <vector>
#include "sdk_includes/antivirus_interface.h"

// 所用json来自： https://github.com/nlohmann/json/releases 当前所用：release 3.4.0, release date  2018-10-30
#include "json.hpp"
using json = nlohmann::json;

using namespace std;



// enum EFlags {
// 		Unzip		    = 0x01,
// 		Unpack		  = 0x02,
// 		StopOnOne	  = 0x10,
// 		ProgramOnly = 0x100,
// 		OriginalMd5 = 0x1000,
// 		UseFigner	  = 0x2000,
// 		UseCloud	  = 0x4000,
// 		Backup		  = 0x8000,
// 		ForceUnzip	= 0x100000
// };

// 一些全局变量
string global_test_content = "";
string global_log_filename = "sdk_test_show.log";
string global_daemon_path_with_name = "/home/jiangmin_antivirus/linux_kv_sdk_test/scripts/run_daemon.sh";
string global_daemon_config_xml_path_with_name = "/home/jiangmin_antivirus/linux_kv_sdk_test/etc/customize_edr.xml";
string global_run_modify_load_path_with_name = "/home/jiangmin_antivirus/linux_kv_sdk_test/sdk_executable/run_modify_load";
string global_kv_plugin_name = "sysinfo";
string global_restart_main_daemon_script_path_with_name = "/home/jiangmin_antivirus/linux_kv_sdk_test/scripts/restart_deamon.sh";
string global_scanpath = "/home/jiangmin_antivirus/linux_kv_sdk_test/sdk_scan_path";            //扫描路径,路径下最少需要两个文件
string global_specialpath = "/home/jiangmin_antivirus/linux_kv_sdk_test/sdk_special_scan_path"; //只含有一个病毒文件的路径
uint32_t global_scanoption = Unzip;
int global_log_method = 0; //记录日志方式：0-仅显示错误（标准输出），1-全部显示（标准输出），2-仅显示错误（文件），3-全部显示（文件）
int global_retrycount = 3; //设置全局重试次数为3，超过重试次数即为失败
const void *global_ptr = NULL;
uint32_t golbal_scancount = 0; //记录扫描文件数量
int global_waitminute = 0;     //扫描用时（分钟），确保在此时间内可以扫描完
uint32_t global_filecount = 0; //总文件数量
string global_virusnewfilename = "";
uint32_t global_quarantinefilecount = 0;              //实时记录的隔离区总文件数量
uint32_t global_quarantineallfilecount = 0;           //扫描后隔离区会存在的总文件数量
string global_testinfo = "chapter 01 - test 01"; //测试编号
bool global_called_notify = false;//记录回调函数是否触发
bool global_callback_test_result = false;// 记录回调函数测试结果
bool global_test_result = false;




// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t testdatanotify(const char *data, uint32_t length, int32_t flag, void *ptr_param)
{
    cout << "[testdatanotify] data : " << data << "length : " << length << "flag : " << flag << "ptr_param :" << ptr_param << endl;
    global_called_notify = true;
    return flag;
}

//正常扫描回调函数
uint32_t testdatanotify_scan(const char *data, uint32_t length, int32_t flag, void *ptr_param)
{
    cout << "[testdatanotify_scan] data : " << data << "length : " << length << "flag : " << flag << "ptr_param :" << ptr_param << endl;
    golbal_scancount++;
    global_called_notify = true;
    return flag;
}

uint32_t testdatanotify_scan_01(const char *data, uint32_t length, int32_t flag, void *ptr_param)
{
    cout << "[testdatanotify_scan_01] data : " << data << "length : " << length << "flag : " << flag << "ptr_param :" << ptr_param << endl;
    if (ptr_param != global_ptr)
    {
        global_callback_test_result = false;
    }
    else
    {
        global_callback_test_result = true;
    }
    global_called_notify = true;
    return flag;
}
// chapter 02 - test 08,todo
uint32_t testdatanotify_scan_02(const char *data, uint32_t length, int32_t flag, void *ptr_param)
{
    cout << "[testdatanotify_scan_02] data : " << data << "length : " << length << "flag : " << flag << "ptr_param :" << ptr_param << endl;
    if (data != nullptr)
    {
        global_callback_test_result = true;
    }
    else
    {
        global_callback_test_result = false;
    }
    global_called_notify = true;
    return flag;
}
// chapter 02 - test 09,todo
uint32_t testdatanotify_scan_03(const char *data, uint32_t length, int32_t flag, void *ptr_param)
{
    cout << "[testdatanotify_scan_03] data : " << data << "length : " << length << "flag : " << flag << "ptr_param :" << ptr_param << endl;
    if (data != nullptr)
    {
        global_callback_test_result = true;
    }
    else
    {
        global_callback_test_result = false;
    }
    global_called_notify = true;
    return flag;
}
// chapter 03 - test 10,todo
uint32_t testdatanotify_scan_04(const char *data, uint32_t length, int32_t flag, void *ptr_param)
{
    cout << "[testdatanotify_scan_04] data : " << data << "length : " << length << "flag : " << flag << "ptr_param :" << ptr_param << endl;
    golbal_scancount++;
    if (data != nullptr)
    {
        global_callback_test_result = true;
    }
    else
    {
        global_callback_test_result = false;
    }
    global_called_notify = true;
    return flag;
}
// chapter 03 - test 11,todo
uint32_t testdatanotify_scan_05(const char *data, uint32_t length, int32_t flag, void *ptr_param)
{
    cout << "[testdatanotify_scan_05] data : " << data << "length : " << length << "flag : " << flag << "ptr_param :" << ptr_param << endl;
    golbal_scancount++;
    if (data != nullptr)
    {
        global_callback_test_result = true;
    }
    else
    {
        global_callback_test_result = false;
    }
    global_called_notify = true;
    return flag;
}

//正常隔离区枚举回调函数,todo
uint32_t testdatanotify_list(const char *data, uint32_t length, int32_t flag, void *ptr_param)
{
    cout << "[testdatanotify_list] data : " << data << "length : " << length << "flag : " << flag << "ptr_param :" << ptr_param << endl;
    global_quarantinefilecount++;
    // TODO: parse virus name from json
    global_virusnewfilename = "neeeeeee";
    global_called_notify = true;
    return flag;
}
//chapter 04 - test 07,todo
uint32_t testdatanotify_list_01(const char *data, uint32_t length, int32_t flag, void *ptr_param)
{
    cout << "[testdatanotify_list_01] data : " << data << "length : " << length << "flag : " << flag << "ptr_param :" << ptr_param << endl;
    global_quarantinefilecount++;
    global_virusnewfilename = "neeeeeee";
    if (data != nullptr)
    {
        global_callback_test_result = true;
    }
    else
    {
        global_callback_test_result = false;
    }
    global_called_notify = true;
    return flag;
}
// chapter 04 - test 08,todo
uint32_t testdatanotify_list_02(const char *data, uint32_t length, int32_t flag, void *ptr_param)
{
    cout << "[testdatanotify_list_02] data : " << data << "length : " << length << "flag : " << flag << "ptr_param :" << ptr_param << endl;
    global_quarantinefilecount++;
    global_virusnewfilename = "neeeeeee";
    if (data != nullptr)
    {
        global_callback_test_result = true;
    }
    else
    {
        global_callback_test_result = false;
    }
    global_called_notify = true;
    return flag;
}
// chapter 04 - test 10,todo
uint32_t testdatanotify_list_03(const char *data, uint32_t length, int32_t flag, void *ptr_param)
{
    cout << "[testdatanotify_list_03] data : " << data << "length : " << length << "flag : " << flag << "ptr_param :" << ptr_param << endl;
    global_quarantinefilecount++;
    global_virusnewfilename = "neeeeeee";
    if (data != nullptr)
    {
        global_callback_test_result = true;
    }
    else
    {
        global_callback_test_result = false;
    }
    global_called_notify = true;
    return flag;
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


string ShellCommand(string cmd, int timeout){
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
    return str_buffer;
}



void restart_daemon()
{
    string cmd = "";
    cmd += global_restart_main_daemon_script_path_with_name;
    // cmd += RESTART_DEAMON_SCRIPT_PATH_WITH_NAME;
    cmd += " ";
    cmd += global_daemon_path_with_name;
    // cmd += DEAMON_PATH_WITH_NAME;
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

//test_chapter_3_test_11 test_chapter_3_test_12 test_chapter_3_test_13
void test_file_count()
{
    string cmd = "ls -lR ";
    cmd += global_scanpath;
    cmd += " |grep \"^-\"|wc -l";
    string ret = ShellCommand(cmd, 30);
    global_filecount = stoi(ret);
    cout<<"----- test file count finish, result : " + to_string(global_filecount) + " files in " + global_scanpath<<endl;
}
//test_chapter_3_test_7 test_chapter_3_test_8 test_chapter_3_test_9 test_chapter_3_test_10 test_chapter_3_test_11 test_chapter_3_test_12 test_chapter_3_test_13
void test_scan_time()
{
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
    global_waitminute = (int)((double)(end - start) / CLOCKS_PER_SEC + 59) / 60+1;
    string str = "----- test scan time finish, result : scan ";
    str += global_scanpath;
    str += "for " + to_string((double)(end - start) / CLOCKS_PER_SEC) + " seconds, set to " + to_string(global_waitminute) + " minute(s) to make sure it can be finished in this time -----";
    cout<<str<<endl;
}

//判断文件夹是否存在
bool FileExist(const char *path_with_name){
    if (access(path_with_name, F_OK) == 0)
    {
        return true;
    }
    return false;
}
//判断文件夹是否存在
bool FolderExist(const char *path_with_name){
    if (opendir(path_with_name) == NULL){
        return false;
    }
    return true;
}

//读取文件内容到string里
string ReadFile2String(string path_with_name){
    return ShellCommand("cat "+path_with_name,30);
}

//检查json合法性，当isfile为ture时，content为包含绝对路径的json文件，否则则是json数据
bool CheckJson(string content,bool isfile){
    //检查json是否存在
    if (!FileExist(content.c_str())){
        cout<<"json file is not exist"<<endl;
        return false;
    }
    try
    {
        if (isfile){
            string json_text = "";
            json_text = ReadFile2String(content);
            if (json_text.compare("") == 0)
                return false;
            json update_json = json::parse(json_text.c_str());
        }
        else{
            json update_json = json::parse(content.c_str());
        }
    }
    catch (json::parse_error& e)
    {
        string message = "";
        message += "json parse error, detail --->";
        message += " message : ";
        message += e.what();
        message += " exception id: ";
        message += e.id;
        message += " byte position of error: ";
        message += e.byte;
        cout<<message<<endl;
        return false;
    }
    return true;
}

void Log2Console(string message){
    cout<<message;
}

//1-info ,2-error, 3-success
void Log(string message,int loglevel){
    string str = "";
    switch(loglevel){
        case 1:
        str += "[info] ";
        break;
        case 2:
        str += "[failed] ";
        break;
        case 3:
        str += "[success] ";
        break;
        default:
        break;
    }
    str += message;
    str += "\n";
    Log2Console(str);
}


// chapter 1 finished, test 3 need to promoted
bool test_chapter_1_test_1(){
    modify_config_not_mount_kv_plugin();
    Log("modify config not mount specific plugin success",1);
    restart_daemon();
    Log("restart daemon success",1);
    void * ptr = av_sdk_init();
    if (ptr != nullptr)
    {
        return false;
    }
    else
    {
        return true;
    }
}
bool test_chapter_1_test_2(){
    modify_config_mount_kv_plugin();
    Log("modify config mount specific plugin success",1);
    restart_daemon();
    Log("restart daemon success",1);
    void * ptr = av_sdk_init();
    if (av_sdk_uninit(ptr) != 1)
    {
        return false;
    }
    else
    {
        return true;
    }
}
bool test_chapter_1_test_3(){
    Log("TODO",1);
    av_sdk_uninit(nullptr);
    return false;
}

bool single_test_av_scan_target(void *ptr_sdk, uint32_t option, DataNotify datanotify, const char *path, void *ptr_param, uint32_t expect_pass_flag, const char *test_message, bool use_passed_ptr_sdk = false)
{
    void *ptr = av_sdk_init();
    uint32_t ret_target = 0;
    Log("start scan task ...",1);
    if (use_passed_ptr_sdk)
    {
        ret_target = av_scan_target(ptr_sdk, option, path, datanotify, ptr_param);
    }
    else
    {
        ret_target = av_scan_target(ptr, option, path, datanotify, ptr_param);
    }
    Log("start scan task finished",1);
    string str = "";
    if (!ptr)
    {
        av_sdk_uninit(ptr);
    }
    if (ret_target != expect_pass_flag)
    {
        // str += "expect " + to_string(expect_pass_flag) + " but return " + to_string(ret_target) + " while test for " + test_message;
        return true;
    }
    else
    {
        // str += "expect " + to_string(expect_pass_flag) + " but return " + to_string(ret_target) + " while test for " + test_message;
        return false;
    }
}

// chapter 2 finished
bool test_chapter_2_test_1(){
    void *test_ptr = NULL;
    return single_test_av_scan_target(nullptr,global_scanoption,testdatanotify_scan,global_scanpath.c_str(),test_ptr,0,global_test_content.c_str(),true);
}
bool test_chapter_2_test_2(){
    void *test_ptr = NULL;
    return single_test_av_scan_target(nullptr,0,testdatanotify_scan,global_scanpath.c_str(),test_ptr,0,global_test_content.c_str(),false);
}
bool test_chapter_2_test_3(){
    void *test_ptr = NULL;
    return single_test_av_scan_target(nullptr,global_scanoption,testdatanotify_scan,nullptr,test_ptr,0,global_test_content.c_str(),false);
}
bool test_chapter_2_test_4(){
    void *test_ptr = NULL;
    return single_test_av_scan_target(nullptr,global_scanoption,nullptr,global_scanpath.c_str(),test_ptr,1,global_test_content.c_str(),false);
}
bool test_chapter_2_test_5(){
    void *test_ptr = NULL;
    return single_test_av_scan_target(nullptr,global_scanoption,testdatanotify_scan,global_scanpath.c_str(),nullptr,1,global_test_content.c_str(),false);
}
bool test_chapter_2_test_6(){
    void *test_ptr = NULL;
    return single_test_av_scan_target(nullptr,global_scanoption,testdatanotify_scan,global_scanpath.c_str(),test_ptr,1,global_test_content.c_str(),false);
}
bool test_chapter_2_test_7(){
    void *test_ptr = NULL;
    global_ptr = test_ptr;
    global_called_notify = false;
    global_callback_test_result = false;
    bool ret =  single_test_av_scan_target(nullptr,global_scanoption,testdatanotify_scan_01,global_scanpath.c_str(),test_ptr,0,global_test_content.c_str(),false);
    while(!global_called_notify){
        Log("wait for callback notify ",1);
        sleep(5);
    }
    return ret && global_callback_test_result;
}
bool test_chapter_2_test_8(){
    void *test_ptr = NULL;
    global_called_notify = false;
    global_callback_test_result = false;
    bool ret = single_test_av_scan_target(nullptr,global_scanoption,testdatanotify_scan_02,global_scanpath.c_str(),test_ptr,0,global_test_content.c_str(),false);
    while(!global_called_notify){
    Log("wait for callback notify ",1);
        sleep(5);
    }
    return ret && global_callback_test_result;
}
bool test_chapter_2_test_9(){
    void *test_ptr = NULL;
    global_called_notify = false;
    global_callback_test_result = false;
    bool ret = single_test_av_scan_target(nullptr,global_scanoption,testdatanotify_scan_03,global_scanpath.c_str(),test_ptr,0,global_test_content.c_str(),false);
    while(!global_called_notify){
        Log("wait for callback notify",1);
        sleep(5);
    }
    return ret && global_callback_test_result;
}

// chapter 3 finished
bool test_chapter_3_test_1(){
    Log("try stop sdk",1);
    uint32_t ret_status = av_sdk_stop(nullptr);
    if (ret_status == 1)
    {
        return false;
    }
    else
    {
        return true;
    }
}
bool test_chapter_3_test_2(){
    Log("try pause sdk",1);
    uint32_t ret_status = av_sdk_pause(nullptr);
    if (ret_status == 1)
    {
        return false;
    }
    else
    {
        return true;
    }
}
bool test_chapter_3_test_3(){
    Log("try resume sdk",1);
    uint32_t ret_status = av_sdk_resume(nullptr);
    if (ret_status == 1)
    {
        return false;
    }
    else
    {
        return true;
    }
}
bool test_chapter_3_test_4(){
    void *ptr = av_sdk_init();
    Log("try restart daemon",1);
    restart_daemon();
    Log("restart daemon finished",1);
    if (av_sdk_stop(ptr) != 0)
    {
       return false;
    }
    else
    {
        return true;
    }
}
bool test_chapter_3_test_5(){
    void *ptr = av_sdk_init();
    Log("try restart daemon",1);
    restart_daemon();
    Log("restart daemon finished",1);
    if (av_sdk_pause(ptr) != 0)
    {
       return false;
    }
    else
    {
        return true;
    }
}
bool test_chapter_3_test_6(){
    void *ptr = av_sdk_init();
    Log("try restart daemon",1);
    restart_daemon();
    Log("restart daemon finished",1);
    if (av_sdk_resume(ptr) != 0)
    {
       return false;
    }
    else
    {
        return true;
    }
}
bool test_chapter_3_test_7(){
    void *ptr = av_sdk_init();
    av_sdk_stop(ptr);
    Log("start scan task ...",1);
    av_scan_target(ptr, global_scanoption, global_scanpath.c_str(), testdatanotify_scan, nullptr);
    sleep(global_waitminute*60);
    Log("make sure scan task finished",1);
    Log("try stop sdk",1);
    uint32_t ret_status = av_sdk_stop(ptr);
    if (ret_status != 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}
bool test_chapter_3_test_8(){
    void *ptr = av_sdk_init();
    av_sdk_stop(ptr);
    Log("start scan task ...",1);
    av_scan_target(ptr, global_scanoption, global_scanpath.c_str(), testdatanotify_scan, nullptr);
    sleep(global_waitminute*60);
    Log("make sure scan task finished",1);
    Log("try pause sdk",1);
    uint32_t ret_status = av_sdk_pause(ptr);
    if (ret_status != 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}
bool test_chapter_3_test_9(){
    void *ptr = av_sdk_init();
    Log("try stop sdk",1);
    av_sdk_stop(ptr);
    av_scan_target(ptr, global_scanoption, global_scanpath.c_str(), testdatanotify_scan, nullptr);
    sleep(global_waitminute*60);
    Log("make sure scan task finished",1);
    Log("try resume sdk",1);
    uint32_t ret_status = av_sdk_resume(ptr);
    if (ret_status != 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}
bool test_chapter_3_test_10(){
    void *ptr = av_sdk_init();
    Log("try stop sdk",1);
    av_sdk_stop(ptr);
    golbal_scancount = 0;
    global_called_notify = false;
    global_callback_test_result = false;
    Log("start scan task ...",1);
    uint32_t ret_status = av_scan_target(ptr, global_scanoption, global_scanpath.c_str(), testdatanotify_scan_04, nullptr);
    Log("start scan task finished",1);
    while (golbal_scancount != 1)
    {
        Log("try stop sdk",1);
        av_sdk_stop(ptr);
        golbal_scancount = 0;
        Log("start scan task ...",1);
        ret_status = av_scan_target(ptr, global_scanoption, global_scanpath.c_str(), testdatanotify_scan_04, nullptr);
        Log("start scan task finished",1);
        sleep(1);
        Log("make sure called notify only once",1);
    }
    golbal_scancount = 0;
    sleep(global_waitminute * 60);
    if (golbal_scancount != 0)
    {
        return false;
    }
    while(!global_called_notify){
        Log("wait for callback",1);
        sleep(5);
    }
    return global_callback_test_result;
}
bool test_chapter_3_test_11(){
    void *ptr = av_sdk_init();
    Log("try stop sdk",1);
    av_sdk_stop(ptr);
    golbal_scancount = 0;
    global_called_notify = false;
    global_callback_test_result = false;
    Log("start scan task ...",1);
    uint32_t ret_status = av_scan_target(ptr, global_scanoption, global_scanpath.c_str(), testdatanotify_scan_05, nullptr);
    Log("start scan task finished",1);
    while (golbal_scancount != 1)
    {
        Log("try stop sdk",1);
        av_sdk_stop(ptr);
        golbal_scancount = 0;
        Log("start scan task ...",1);
        ret_status = av_scan_target(ptr, global_scanoption, global_scanpath.c_str(), testdatanotify_scan_05, nullptr);
        Log("start scan task finished",1);
        sleep(1);
        Log("make sure called notify only once",1);
    }
    golbal_scancount = 0;
    sleep(global_waitminute * 60);
    if (golbal_scancount != 0)
    {
        return false;
    }
    golbal_scancount = 0;
    Log("try resume sdk",1);
    ret_status = av_sdk_resume(ptr);
    if (ret_status != 1)
    {
        return false;
    }
    sleep(global_waitminute * 60);
    if (golbal_scancount == 0)
    {
        return false;
    }
    if (golbal_scancount != global_filecount - 1)
    {
        return false;
    }
    while(!global_called_notify){
        Log("wait for callback",1);
        sleep(5);
    }
    return global_callback_test_result;
}
bool test_chapter_3_test_12(){
    global_called_notify = false;
    global_callback_test_result = false;
    void *ptr = av_sdk_init();
    Log("try stop sdk",1);
    av_sdk_stop(ptr);
    Log("start scan task ...",1);
    uint32_t ret_status = av_scan_target(ptr, global_scanoption, global_scanpath.c_str(), testdatanotify_scan, nullptr);
    Log("start scan task finished",1);
    while (golbal_scancount != global_filecount)
    {
        Log("try stop sdk",1);
        ret_status = av_sdk_stop(ptr);
        if (ret_status != 1)
        {
            return false;
        }
        Log("try resume sdk",1);
        av_sdk_resume(ptr);
        sleep(10);
    }
    golbal_scancount = 0;
    sleep(global_waitminute * 60);
    Log("wait for scan",1);
    if (golbal_scancount != 0)
    {
        return false;
    }
    while(!global_called_notify){
        Log("wait for callback",1);
        sleep(5);
    }
    return global_callback_test_result;
}
bool test_chapter_3_test_13(){
    global_called_notify = false;
    global_callback_test_result = false;
    void *ptr = av_sdk_init();
    av_sdk_stop(ptr);
    Log("start scan task ...",1);
    uint32_t ret_status = av_scan_target(ptr, global_scanoption, global_scanpath.c_str(), testdatanotify_scan, nullptr);
    Log("start scan task finished",1);
    while (golbal_scancount != global_filecount)
    {
        Log("wait for scan ",1);
        sleep(10);
    }
    golbal_scancount = 0;
    Log("try pause scan",1);
    ret_status = av_sdk_pause(ptr);
    if (ret_status != 1)
    {
        return false;
    }
    sleep(global_waitminute * 60);
    if (golbal_scancount != 0)
    {
        return false;
    }
    golbal_scancount = 0;
    Log("try resume scan",1);
    ret_status = av_sdk_resume(ptr);
    sleep(global_waitminute * 60);
    if (golbal_scancount == 0)
    {
        return false;
    }
    if (golbal_scancount != global_filecount)
    {
        return false;
    }
    while(!global_called_notify){
        Log("wait for callback",1);
        sleep(5);
    }
    return global_callback_test_result;
}

//用于在测试chapter 4时扫描并且隔离文件后恢复已隔离文件到speical_scan_path
string get_isolate_virusname(){
    void *ptr_sdk = av_sdk_init();
    av_scan_target(ptr_sdk, global_scanoption, global_specialpath.c_str(), testdatanotify_scan, nullptr);//再扫描一次，确保已经隔离了
    global_virusnewfilename = "";
    av_list_isolation(ptr_sdk,testdatanotify_list,nullptr);
    while (global_virusnewfilename.compare("") == 0){
        Log("wait for callback",1);
        sleep(3);
    }
    return global_virusnewfilename;
}

// chapter 4 finished, TODO: test 6 7 8 9 10
bool test_chapter_4_test_1(){
    Log("try list file in isolation",1);
    uint32_t ret_status = av_list_isolation(nullptr, testdatanotify_list, nullptr);
    if (ret_status != 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}
bool test_chapter_4_test_2(){
    Log("try restore specific file",1);
    uint32_t ret_status = av_restore_isolation(nullptr, "");
    if (ret_status != 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}
bool test_chapter_4_test_3(){
    void *ptr_sdk = av_sdk_init();
    Log("try list file in isolation",1);
    uint32_t ret_status = av_list_isolation(ptr_sdk, nullptr, nullptr);
    if (ret_status != 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}
bool test_chapter_4_test_4(){
    void *ptr_sdk = av_sdk_init();
    Log("try restore specific file",1);
    uint32_t ret_status = av_restore_isolation(ptr_sdk, nullptr);
    if (ret_status != 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}
bool test_chapter_4_test_5(){
    void *ptr_sdk = av_sdk_init();
    Log("try list file in isolation",1);
    uint32_t ret_status = av_list_isolation(ptr_sdk, testdatanotify_list, nullptr);
    if (ret_status != 1)
    {
        return false;
    }
    else
    {
        return true;
    }
}
bool test_chapter_4_test_6(){
    void *ptr_sdk = av_sdk_init();
    Log("try restore specific file",1);
    uint32_t ret_status = av_restore_isolation(ptr_sdk, get_isolate_virusname().c_str());
    if (ret_status != 1)
    {
        return false;
    }
    else
    {
        return true;
    }
}
bool test_chapter_4_test_7(){
    void *ptr_sdk = av_sdk_init();
    int retry_count = 0;
    golbal_scancount = 0;
    uint32_t ret_status = 0;
    global_called_notify = false;
    global_callback_test_result = false;
    //restore isolate virus file
    av_restore_isolation(ptr_sdk,get_isolate_virusname().c_str());

    while (golbal_scancount != 1)
    {
        Log("start scan task ...",1);
        ret_status = av_scan_target(ptr_sdk, global_scanoption, global_specialpath.c_str(), testdatanotify_scan, nullptr);
        if (ret_status != 1)
        {
            golbal_scancount = 0;
            return false;
        }
        if (retry_count == global_retrycount)
        {
            retry_count = 0;
            return false;
        }
    }
    Log("try list file in isolation",1);
    ret_status = av_list_isolation(ptr_sdk, testdatanotify_list_01, nullptr);
    if (ret_status != 1)
    {
        return false;
    }
    while(!global_called_notify){
        Log("wait for callback",1);
        sleep(5);
    }
    return global_callback_test_result;
}
bool test_chapter_4_test_8(){
    void *ptr_sdk = av_sdk_init();
    int retry_count = 0;
    golbal_scancount = 0;
    uint32_t ret_status = 0;
    global_called_notify = false;
    global_callback_test_result = false;
    //restore isolate virus file
    av_restore_isolation(ptr_sdk,get_isolate_virusname().c_str());

    while (golbal_scancount != 1)
    {
        Log("start scan task ...",1);
        ret_status = av_scan_target(ptr_sdk, global_scanoption, global_specialpath.c_str(), testdatanotify_scan, nullptr);
        if (ret_status != 1)
        {
            golbal_scancount = 0;
            return false;
        }
        if (retry_count == global_retrycount)
        {
            retry_count = 0;
            return false;
        }
    }
    Log("try list file in isolation",1);
    ret_status = av_list_isolation(ptr_sdk, testdatanotify_list_02, nullptr);
    if (ret_status != 1)
    {
        return false;
    }
    while(!global_called_notify){
        Log("wait for callback",1);
        sleep(5);
    }
    return global_callback_test_result;
}
bool test_chapter_4_test_9(){
    return test_chapter_4_test_8();
}
bool test_chapter_4_test_10(){
    void *ptr_sdk = av_sdk_init();
    int retry_count = 0;
    golbal_scancount = 0;
    uint32_t ret_status = 0;
    global_called_notify = false;
    global_callback_test_result = false;
    //restore isolate virus file
    av_restore_isolation(ptr_sdk,get_isolate_virusname().c_str());
    while (golbal_scancount != 1)
    {
        ret_status = av_scan_target(ptr_sdk, global_scanoption, global_specialpath.c_str(), testdatanotify_scan, nullptr);
        if (ret_status != 1)
        {
            golbal_scancount = 0;
            return false;
        }
        if (retry_count == global_retrycount)
        {
            retry_count = 0;
            return false;
        }
    }
    Log("try list file in isolation",1);
    ret_status = av_list_isolation(ptr_sdk, testdatanotify_list_03, nullptr);
    if (ret_status != 1)
    {
        return false;
    }
    while(!global_called_notify){
        Log("wait for callback",1);
        sleep(5);
    }
    return global_callback_test_result;
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



//根据命令行给定的chapter和test number召唤对应的函数
bool CallRelativeFunction(int chapter,int number){
    bool ret = false;
    switch(chapter){
        case 1:
        switch(number){
            case 1:
            ret = test_chapter_1_test_1();
            break;
            case 2:
            ret = test_chapter_1_test_1();
            break;
            case 3:
            ret = test_chapter_1_test_1();
            break;
            default:
            Log("specific test number is not found",1);
            return false;
        }
        break;
        case 2:
        switch(number){
            case 1:
            ret = test_chapter_2_test_1();
            break;
            case 2:
            ret = test_chapter_2_test_2();
            break;
            case 3:
            ret = test_chapter_2_test_3();
            break;
            case 4:
            ret = test_chapter_2_test_4();
            break;
            case 5:
            ret = test_chapter_2_test_5();
            break;
            case 6:
            ret = test_chapter_2_test_6();
            break;
            case 7:
            ret = test_chapter_2_test_7();
            break;
            case 8:
            ret = test_chapter_2_test_8();
            break;
            case 9:
            ret = test_chapter_2_test_9();
            break;
            default:
            Log("specific test number is not found",1);
            return false;
        }
        break;
        case 3:
        test_file_count();
        test_scan_time();
        switch(number){
            case 1:
            ret = test_chapter_3_test_1();
            break;
            case 2:
            ret = test_chapter_3_test_2();
            break;
            case 3:
            ret = test_chapter_3_test_3();
            break;
            case 4:
            ret = test_chapter_3_test_4();
            break;
            case 5:
            ret = test_chapter_3_test_5();
            break;
            case 6:
            ret = test_chapter_3_test_6();
            break;
            case 7:
            ret = test_chapter_3_test_7();
            break;
            case 8:
            ret = test_chapter_3_test_8();
            break;
            case 9:
            ret = test_chapter_3_test_9();
            break;
            case 10:
            ret = test_chapter_3_test_10();
            break;
            case 11:
            ret = test_chapter_3_test_11();
            break;
            case 12:
            ret = test_chapter_3_test_12();
            break;
            case 13:
            ret = test_chapter_3_test_13();
            break;
            default:
            Log("specific test number is not found",1);
            return false;
        }
        break;
        case 4:
        switch(number){
            case 1:
            ret = test_chapter_4_test_1();
            break;
            case 2:
            ret = test_chapter_4_test_2();
            break;
            case 3:
            ret = test_chapter_4_test_3();
            break;
            case 4:
            ret = test_chapter_4_test_4();
            break;
            case 5:
            ret = test_chapter_4_test_5();
            break;
            case 6:
            ret = test_chapter_4_test_6();
            break;
            case 7:
            ret = test_chapter_4_test_7();
            break;
            case 8:
            ret = test_chapter_4_test_8();
            break;
            case 9:
            ret = test_chapter_4_test_9();
            break;
            case 10:
            ret = test_chapter_4_test_10();
            break;
            default:
            Log("specific test number is not found",1);
            return false;
        }
        break;
        default:
        Log("specific test chapter is not found",1);
            return false;
    }
    global_test_result = ret;
    return true;
}


void Call_TEST(int test_chapter, int test_number){
    cout<<"------------------- start running chapter "<<test_chapter<<" test "<<test_number<<" -------------------"<<endl;
    // cout<<"===>"<<global_test_content<<endl;
    // test_file_count();
    // test_scan_time();
    if (CallRelativeFunction(test_chapter,test_number)){
        cout<<"--------running chapter "<<test_chapter<<" test "<<test_number<<" finished, test result "<<(global_test_result ? "PASSED" : "FAILED" )<<"------------"<<endl;        
    }
}

int main(int argc, char *argv[]){
    string helpinfo = "";
    helpinfo += "------------------------------------------------------------------------\n";
    helpinfo += "|This application is designed for showing special test jiangmin kv sdk |\n";
    helpinfo += "------------------------------------------------------------------------\n";
    helpinfo += "|  Usage:                                                              |\n";
    helpinfo += "|      -c                          Chapter you want to show test       |\n";
    helpinfo += "|      -t                          test Number in chapter              |\n";
#ifdef ADVANCE
    helpinfo += "|      -j                          scan config json(genarate from .rb) |\n";
#endif
    helpinfo += "|      -h                          print this help                     |\n"; 
    helpinfo += "------------------------------------------------------------------------\n";
    helpinfo += "|  Example:                                                            |\n";
#ifdef ADVANCE    
    helpinfo += "|      ./sdk_test_show -c 1 -t 2 -j scan_config.json                   |\n";
#else
    helpinfo += "|      ./sdk_test_show -c 1 -t 2                                       |\n";
#endif
    helpinfo += "------------------------------------------------------------------------\n";
    helpinfo += "|  Author: yezhihui@jiangmin.com Codedate: 20181129                    |\n";
    helpinfo += "------------------------------------------------------------------------\n";
    int test_chapter = 0;
    int test_number = 0;
    string json_file = "";
    string tmp = "";
    json json_content;
    if (argc == 1){
        cout<<helpinfo<<endl;
        return 0;
    }
#ifdef ADVANCE    
    for (int opt = 0; (opt = getopt(argc, argv, "c:t:j:h:")) != -1; )
#else    
    for (int opt = 0; (opt = getopt(argc, argv, "c:t:h:")) != -1; )
#endif
    {
        switch (opt)
        {
        case 'c':
            test_chapter = atoi(optarg);
            break;
        case 't':
            test_number = atoi(optarg);
            break;
#ifdef ADVANCE            
        case 'j':
            json_file = optarg;
            break;
#endif            
        case 'h':
            cout<<helpinfo<<endl;
            return 0;
        default: /* '?' */
            cout<<helpinfo<<endl;
            return 0;
        }
    }


    if (json_file.compare("") == 0){
        // if (test_chapter>0 && test_chapter <5){
        //     return -1;
        // }
    }
    else{
        if (!CheckJson(json_file,true)){//验证　-j　参数
            cout<<"json文件 "<<json_file<<" 不合法，无法解析"<<endl;
            return -1;
        }
        else{
            tmp = ReadFile2String(json_file);
            json_content = json::parse(tmp);
        }
        // int tmpint = json_content["chapter_count"];
        // if (test_chapter > tmpint || test_chapter < 0){//验证　-ｃ　参数
        //     cout <<"test chapter(-c)指定错误"<<endl;
        //     return -1;
        // }
        // tmp = "";
        // tmpint = json_content["chapter_"+to_string(test_number)+"_count"];
        // if (test_number > tmpint || test_number < 0){//验证　-n　参数
        //     cout <<"test number(-t)指定错误"<<endl;
        //     return -1;
        // }
        //获取测试详情
        // global_test_content = json_content["chapter_"+to_string(test_chapter)][to_string(test_number)];
        //设置环境变量
        global_daemon_path_with_name = json_content["run_daemon_script"];
        global_daemon_config_xml_path_with_name = json_content["config_xml"];
        global_run_modify_load_path_with_name = json_content["run_modify_load"];
        global_kv_plugin_name = json_content["kv_plugin_name"];
        global_restart_main_daemon_script_path_with_name = json_content["restart_script"];
        global_scanpath = json_content["scan_path"];
        global_specialpath = json_content["special_scan_path"];
        tmp = json_content["scan_option"];
        global_scanoption = EFlags2Int(tmp);
        tmp = json_content["log_method"];
        global_log_method = atoi(tmp.c_str());
    }
    Call_TEST(test_chapter,test_number);
    
    return 0;
}




