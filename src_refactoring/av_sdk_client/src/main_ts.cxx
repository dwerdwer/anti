// #include "../material/use_sdk/include/antivirus_interface.h"
#include "antivirus_interface.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/stddef.h>

#define nullptr NULL

#define FUNCTIONSTARTINFO printf("[i] start test %s \n",__FUNCTION__)
#define DEBUGINFO printf("file : %s --- line : %d --- function : %s\n",__FILE__, __LINE__, __FUNCTION__)
// 定义文件总个数，用于判断扫描是否完成，需要手动修改
#define FILECOUNT 5
// 定义等待扫描完成时间，用于判断中途停止后是否还在扫描，需要确保在此时间内扫描可以完成，需要手动修改
#define WAITMINUTE 3

#define DEAMON_PATH_WITH_NAME "/home/snail/svn/virus_checking_linux_client/build/main_daemon"//需要手动修改
#define CONFIG_XML_PATH "/home/zen/work_dir/project/06_kv_linux_sdk_test/material/sdk_test/tools/test_config.xml"//需要手动修改
#define RUN_MODIFY_LOAD_PATH_WITH_NAME "/home/zen/work_dir/project/06_kv_linux_sdk_test/material/sdk_test/tools/run_modify_load"//需要手动修改
#define KV_PLUGIN_NAME "file_monitor"//需要手动修改
#define RESTART_DEAMON_SCRIPT_PATH_WITH_NAME "/home/zen/work_dir/project/06_kv_linux_sdk_test/material/sdk_test/tools/restart_deamon.sh"//需要手动修改

using namespace std;

// enum EFlags {
// 		Unzip		    	= 0x01,	// 解开压缩包
// 		Unpack		= 0x02,	// 脱壳
// 		StopOnOne	  	= 0x10,	// 扫描到一个病毒文件时停止继续扫描
// 		ProgramOnly 	= 0x100,	// 未使用
// 		OriginalMd5 	= 0x1000,	// 未使用
// 		UseFinger	  	= 0x2000,	// 未使用
// 		UseCloud	  	= 0x4000,	// 未使用
// 		Backup		= 0x8000,	// 备份病毒文件
// 		ForceUnzip		= 0x100000	// 解开隐藏的压缩包
// };

//一些全局变量
int global_retrycount = 3;//设置全局重试次数为3，超过重试次数即为失败
void * global_ptr = NULL;
int golbal_scancount = 0;//记录扫描文件数量
int global_waitminute = 0;//扫描用时（分钟），确保在此时间内可以扫描完
int global_filecount = 0;//总文件数量
uint32_t global_scanoption = Unzip;
const char* global_scanpath = "/home";//扫描路径,路径下最少需要两个文件,需要手动修改
const char* global_specialpath = "/home";//只含有一个病毒文件的路径,需要手动修改
// const char* global_knownvirusfilename = "";
const char* global_virusnewfilename = "";
int global_quarantinefilecount = 0;//实时记录的隔离区总文件数量
int global_quarantineallfilecount = 0;//扫描后隔离区会存在的总文件数量
const char* global_testnumber = 0;//测试编号

//finished
void print_test_failed_message(const char * error_message,const char * test_number=""){
    printf("[-] %s --- test for %s failed\n",test_number,error_message);
}
																									
//判断文件夹是否存在
bool file_exist(const char* path_with_name){
    if(access(path_with_name,F_OK) == 0){
        return true;
    }
    return false;
}

//运行shell命令,不同于system
string ShellCommand(string cmd,int timeout)
{
    FILE* pp = popen(cmd.c_str(),"r");
    if(!pp)
        return "";
    char buffer[1024*2]={0};
    string str_buffer = "";
    while(fgets(buffer,sizeof(buffer),pp) != NULL){
        str_buffer.append(buffer);
    }
    pclose(pp);
    return str_buffer;
}

// typedef uint32_t (*Notify)(const char * file,uint32_t flag,uint64_t record,const char * description, void *ptr_param);


uint32_t testscannotify(const char * file,uint32_t flag,uint64_t record,const char * description,void * ptr_param){
    cout<<"[notify] file : "<<file<<"flag : "<<flag<<"record : "<<record<<"description : "<<description<<"ptr_param :"<<ptr_param<<endl;
    golbal_scancount++;
    return flag;
}

uint32_t testscannotify1(const char * file,uint32_t flag,uint64_t record,const char * description,void * ptr_param){
    if (ptr_param != global_ptr){
        printf("[-] test for ;触发回调函数，回调函数的ptr_param参数和av_scan_target传入的相等 failed\n");
    }
    cout<<"[notify] file : "<<file<<"flag : "<<flag<<"record : "<<record<<"description : "<<description<<"ptr_param :"<<ptr_param<<endl;
    return flag;
}

uint32_t testscannotify2(const char * file,uint32_t flag,uint64_t record,const char * description,void * ptr_param){
    if (nullptr == file){
        printf("[-] test for ;触发回调函数，回调函数的file参数不为NULL failed\n");
    }
    cout<<"[notify] file : "<<file<<"flag : "<<flag<<"record : "<<record<<"description : "<<description<<"ptr_param :"<<ptr_param<<endl;
    return flag;
}

uint32_t testscannotify3(const char * file,uint32_t flag,uint64_t record,const char * description,void * ptr_param){
    if (flag != 0){
        if (nullptr == description){
            printf("[-] test for ;触发回调函数，回调函数的flag参数不为0时，description参数不为NULL failed\n");
        }
    }
    cout<<"[notify] file : "<<file<<"flag : "<<flag<<"record : "<<record<<"description : "<<description<<"ptr_param :"<<ptr_param<<endl;
    return flag;
}

uint32_t testscannotify4(const char * file,uint32_t flag,uint64_t record,const char * description,void * ptr_param){
    uint32_t ret_status = av_sdk_pause(global_ptr);
    if (ret_status != 1){
        print_test_failed_message("以正常参数调用av_scan_target扫描含有两个以上文件的目录，在第一次触发回调函数时，在回调函数中以正常参数调用av_sdk_pause，返回1");
    }
    cout<<"[notify] file : "<<file<<"flag : "<<flag<<"record : "<<record<<"description : "<<description<<"ptr_param :"<<ptr_param<<endl;
    golbal_scancount++;
    return flag;
}

uint32_t testscannotify5(const char * file,uint32_t flag,uint64_t record,const char * description,void * ptr_param){
    uint32_t ret_status = av_sdk_stop(global_ptr);
    if (ret_status != 1){
        print_test_failed_message("以正常参数调用av_scan_target扫描含有两个以上文件的目录，在第一次触发回调函数时，在回调函数中以正常参数调用av_sdk_stop，返回1");
    }
    cout<<"[notify] file : "<<file<<"flag : "<<flag<<"record : "<<record<<"description : "<<description<<"ptr_param :"<<ptr_param<<endl;
    golbal_scancount++;
    return flag;
}

//typedef uint32_t (*ListNotify)(const char * file_old, const char * file_new,const char * sha,uint64_t size);

//隔离区枚举回调函数隔离区枚举回调函数
uint32_t testlistnotify(const char * file_old, const char * file_new,const char * sha,uint64_t size){
    cout<<"[listnotify] file_old : "<<file_old<<"file_new : "<<file_new<<"sha(hash) : "<<sha<<"size : "<<size<<endl;
    global_quarantinefilecount++;
    global_virusnewfilename = file_new;
    return 0;
}

uint32_t testlistnotify1(const char * file_old, const char * file_new,const char * sha,uint64_t size){
    cout<<"[listnotify] file_old : "<<file_old<<"file_new : "<<file_new<<"sha(hash) : "<<sha<<"size : "<<size<<endl;
    global_quarantinefilecount++;
    global_virusnewfilename = file_new;
    if (!(file_old != nullptr && file_new != nullptr && sha != nullptr && size != 0)){
        print_test_failed_message("触发ListNotify回调函数时，函数参数file_old、file_new、sha不为空，size参数不为0",global_testnumber);
    }
    return 0;
}

uint32_t testlistnotify2(const char * file_old, const char * file_new,const char * sha,uint64_t size){
    cout<<"[listnotify] file_old : "<<file_old<<"file_new : "<<file_new<<"sha(hash) : "<<sha<<"size : "<<size<<endl;
    global_quarantinefilecount++;
    global_virusnewfilename = file_new;
    if (file_old == nullptr){
        if (av_restore_isolation(global_ptr,file_new) != 1){
            print_test_failed_message("触发ListNotify回调函数时，如果file_old指向的文件不存在，以正常参数调用av_restore_isolation，返回1",global_testnumber);
        }
        else{
            if(!file_exist("file_old")){
                print_test_failed_message("触发ListNotify回调函数时，如果file_old指向的文件不存在，以正常参数调用av_restore_isolation，返回1。确认file_old指向的文件存在。",global_testnumber);
            }
        }
    }
    else{
        print_test_failed_message("触发ListNotify回调函数时，如果file_old指向的文件不存在",global_testnumber);
    }
    return 0;
}

uint32_t testlistnotify4(const char * file_old, const char * file_new,const char * sha,uint64_t size){
    cout<<"[listnotify] file_old : "<<file_old<<"file_new : "<<file_new<<"sha(hash) : "<<sha<<"size : "<<size<<endl;
    global_quarantinefilecount++;
    global_virusnewfilename = file_new;
    struct stat statbuf;
    if(file_exist(file_old)){
        stat(file_old,&statbuf);
        time_t modifytime= statbuf.st_mtim.tv_sec;
        if (av_restore_isolation(global_ptr,file_new) != 1){
            print_test_failed_message("触发ListNotify回调函数时，如果file_old指向的文件存在，以file_new为参数，其它参数正常调用av_restore_isolation，返回1",global_testnumber);
        }
        else{
            stat(file_old,&statbuf);
            if (statbuf.st_mtim.tv_sec == modifytime){
                print_test_failed_message("触发ListNotify回调函数时，如果file_old指向的文件存在，以file_new为参数，其它参数正常调用av_restore_isolation，返回1。确认file_old指向的文件已经被覆盖（修改日期）。");
            }
        }
    }
    return 0;
}

// promote
void test_file_count(){
    printf("----- test file count -----\n");

    printf("----- test file count finish -----\n");
}
// promote
void test_scan_time(){
    printf("----- test scan time -----\n");

    printf("----- test scan time finish-----\n");
}

void restart_deamon(){
    string cmd = "";
    cmd += RESTART_DEAMON_SCRIPT_PATH_WITH_NAME;
    cmd += " ";
    cmd += DEAMON_PATH_WITH_NAME;
    ShellCommand(cmd,30);
}

void modify_config_mount_kv_plugin(){
    string cmd = "";
    cmd += RUN_MODIFY_LOAD_PATH_WITH_NAME;
    cmd += " -f ";
    cmd += CONFIG_XML_PATH;
    cmd += " -m ";
    cmd += KV_PLUGIN_NAME;
    cmd += " -l y";
    ShellCommand(cmd,30);
}
void modify_config_not_mount_kv_plugin(){
    string cmd = "";
    cmd += RUN_MODIFY_LOAD_PATH_WITH_NAME;
    cmd += " -f ";
    cmd += CONFIG_XML_PATH;
    cmd += " -m ";
    cmd += KV_PLUGIN_NAME;
    cmd += " -l n";
    ShellCommand(cmd,30);
}

//检查运行环境，成功返回1，否则为0
int check_environment(){
    return 1;
}
//finished
void TEST_av_sdk_init_andav_sdk_uninit(){
    FUNCTIONSTARTINFO;
    void * ptr = nullptr;
    // test 01 修改配置文件不挂载杀毒引擎插件；重启Daemon；调用av_sdk_init，返回NULL。
    modify_config_not_mount_kv_plugin();
    restart_deamon();
    ptr = av_sdk_init();
    if (ptr != nullptr){
        print_test_failed_message("");
    }
    // test 02 修改配置文件挂载杀毒引擎插件；重启Daemon；调用av_sdk_init获得指针；以此指针调用av_sdk_uninit，返回1。
    modify_config_mount_kv_plugin(); 
    restart_deamon();
    ptr = av_sdk_init();
    if (av_sdk_uninit(ptr) != 1){
        print_test_failed_message("修改配置文件挂载杀毒引擎插件；重启Daemon；调用av_sdk_init获得指针；以此指针调用av_sdk_uninit，返回1");
    }
    //test 03 用空指针调用av_sdk_uninit，程序崩溃。
    printf("[i] try call av_sdk_uninit with nullptr...\n");
    av_sdk_uninit(nullptr);
    printf("[i] try call av_sdk_uninit with nullptr done,so what happened?\n");
}

void single_test_av_scan_target(void * ptr_sdk,uint32_t option,ScanNotify testscannotify,const char * path,void *ptr_param,uint32_t expect_pass_flag,const char * test_message,bool use_passed_ptr_sdk = false){
    void *ptr = av_sdk_init();
    uint32_t ret_target = 0;
    if (use_passed_ptr_sdk){
        ret_target = av_scan_target(ptr_sdk,option,path,testscannotify,ptr_param);
    }
    else{
        ret_target = av_scan_target(ptr,option,path,testscannotify,ptr_param);
    }  
    if (ret_target != expect_pass_flag){
        printf("[-] test FAILED : expect %u but return %u while test for %s \n",expect_pass_flag,ret_target,test_message);
    }
    if (!ptr){
        av_sdk_uninit(ptr);
    }
}
//finished
void TEST_av_scan_target(){ 
    FUNCTIONSTARTINFO;
    // void *ptr = av_sdk_init();
    uint32_t option = global_scanoption;
    const char* path = global_scanpath;
    uint32_t ret_target = 0;
    void * test_ptr = NULL;
    global_ptr = test_ptr;
    //test 01
    single_test_av_scan_target(nullptr,option,testscannotify,path,test_ptr,0,"以ptr_sdk为空指针调用av_scan_target，程序崩溃",true);
    //test 02
    single_test_av_scan_target(nullptr,0,testscannotify,path,test_ptr,0,"以uint32_option为0而其它参数正常调用av_scan_target，返回0",false);
    //test 03
    single_test_av_scan_target(nullptr,option,testscannotify,nullptr,test_ptr,0,"以ptr_path为NULL而其它参数正常调用av_scan_target，返回0",false);
    //test 04
    single_test_av_scan_target(nullptr,option,nullptr,path,test_ptr,1,"以ptr_notify为NULL而其它参数正常调用av_scan_target，返回1",false);
    //test 05
    single_test_av_scan_target(nullptr,option,testscannotify,path,nullptr,1,"以ptr_param为NULL而其它参数正常调用av_scan_target，返回1",false);
    //test 06
    single_test_av_scan_target(nullptr,option,testscannotify,path,test_ptr,1,"以所有参数正常时调用av_scan_target，返回1",false);
    //test 07 ---
    single_test_av_scan_target(nullptr,option,testscannotify1,path,test_ptr,1,"以所有参数正常时调用av_scan_target，返回1",false);
    // ret_target = av_scan_target(ptr,option,testscannotify,path);
    // if (ret_target == 1){
    //     DEBUGINFO;
    //     printf("[-] return value : %d --- test failed : 以所有参数正常时调用av_scan_target，返回1；触发回调函数，回调函数的ptr_param参数和av_scan_target传入的相等。",ret_target);
    // }
    //test 08 ---
    single_test_av_scan_target(nullptr,option,testscannotify1,path,test_ptr,1,"以所有参数正常时调用av_scan_target，返回1",false);

    // // ret_target = av_scan_target(ptr,option,testscannotify,path);
    // if (ret_target == 1){
    //     DEBUGINFO;
    //     printf("[-] return value : %d --- test failed : 以所有参数正常时调用av_scan_target，返回1；触发回调函数，回调函数的file参数不为NULL。",ret_target);
    // }
    //test 09
    single_test_av_scan_target(nullptr,option,testscannotify1,path,test_ptr,1,"以所有参数正常时调用av_scan_target，返回1",false);
    // // ret_target = av_scan_target(ptr,option,testscannotify,path);
    // if (ret_target == 1){
    //     DEBUGINFO;
    //     printf("[-] return value : %d --- test failed : 以所有参数正常时调用av_scan_target，返回1；触发回调函数，回调函数的flag参数不为0时，description参数不为NULL。",ret_target);
    // }
}
//finished, need to promote
void TEST_av_scan_stop_and_av_sdk_pause_and_av_scan_resume(){
    FUNCTIONSTARTINFO;
    const char * test_number="";
    golbal_scancount = 0;
    void *ptr = av_sdk_init();
    if (ptr == nullptr){
        print_test_failed_message("TEST_av_scan_stop_and_av_sdk_pause_and_av_scan_resume");
        return;
    }
    global_ptr = ptr;
    uint32_t option = global_scanoption;
    const char* path = global_scanpath;
    uint32_t ret_status = 0;
    ret_status = av_scan_target(ptr,option,path,testscannotify,nullptr);
    //test 01
    test_number = "01";
    ret_status = av_sdk_stop(nullptr);
    if (ret_status == 1){
        print_test_failed_message("以空指针调用av_sdk_stop，程序崩溃。",test_number);
    }
    //test 02
    test_number = "02";
    ret_status = av_sdk_pause(nullptr);
    if(ret_status == 1){
        print_test_failed_message("以空指针调用av_sdk_pause，程序崩溃。",test_number);
    }
    //tesst 03
    test_number = "03";
    ret_status = av_sdk_resume(nullptr);
    if (ret_status == 1){
        print_test_failed_message("以空指针调用av_sdk_resume，程序崩溃。",test_number);
    }
    //test 04 重启Daemon；在未调用av_scan_target时，以正常参数调用av_sdk_stop，返回0。 
    test_number = "04";
    restart_deamon();
    if(av_sdk_stop(ptr) != 0){
        print_test_failed_message("重启Daemon；在未调用av_scan_target时，以正常参数调用av_sdk_stop，返回0");
    }
    //test 05 重启Daemon；在未调用av_scan_target时，以正常参数调用av_sdk_pause，返回0。
    test_number = "05";
    restart_deamon();
    if(av_sdk_pause(ptr) != 0){
        print_test_failed_message("重启Daemon；在未调用av_scan_target时，以正常参数调用av_sdk_pause，返回0。");
    }
    //test 06 重启Daemon；在未调用av_scan_target时，以正常参数调用av_sdk_resume，返回0。
    test_number = "06";
    restart_deamon();
    if(av_sdk_resume(ptr) != 0){
        print_test_failed_message("重启Daemon；在未调用av_scan_target时，以正常参数调用av_sdk_resume，返回0。");
    }
    //test 07 以正常参数调用av_scan_target；在确认扫描结束之后，以正常参数调用av_sdk_stop，返回0。
    test_number = "07";
    av_sdk_stop(ptr);
    av_scan_target(ptr,option,path,testscannotify5,nullptr);
    sleep(WAITMINUTE*60);
    ret_status = av_sdk_stop(ptr);
    if (ret_status != 0){
        print_test_failed_message("在确认扫描结束之后，以正常参数调用av_sdk_stop，返回0",test_number);
    }
    //test 08 以正常参数调用av_scan_target；在确认扫描结束之后，以正常参数调用av_sdk_pause，返回0。
    test_number = "08";
    av_sdk_stop(ptr);
    av_scan_target(ptr,option,path,testscannotify5,nullptr);
    sleep(WAITMINUTE*60);
    ret_status = av_sdk_pause(ptr);
    if (ret_status != 0){
        print_test_failed_message("在确认扫描结束之后，以正常参数调用av_sdk_pause，返回0",test_number);
    }
    //test 09 以正常参数调用av_scan_target；在确认扫描结束之后，以正常参数调用av_sdk_resume，返回0。
    test_number = "09";
    av_sdk_stop(ptr);
    av_scan_target(ptr,option,path,testscannotify5,nullptr);
    sleep(WAITMINUTE*60);
    ret_status = av_sdk_resume(ptr);
    if (ret_status != 0){
        print_test_failed_message("在确认扫描结束之后，以正常参数调用av_sdk_resume，返回0",test_number);
    }
    //test 10 以正常参数调用av_scan_target扫描含有两个以上文件的目录，在第一次触发回调函数时，在回调函数中以正常参数调用av_sdk_stop，返回1。且后续不再有回调函数触发。
    test_number = "10";
    av_sdk_stop(ptr);
    golbal_scancount = 0;
    ret_status = av_scan_target(ptr,option,path,testscannotify5,nullptr);
    while(golbal_scancount != 1){
        av_sdk_stop(ptr);
        golbal_scancount = 0;
        ret_status = av_scan_target(ptr,option,path,testscannotify5,nullptr);
    }
    golbal_scancount = 0;
    sleep(WAITMINUTE*60);
    if (golbal_scancount != 0){
        print_test_failed_message("且后续不再有回调函数触发",test_number);
    }
    //test 11 以正常参数调用av_scan_target扫描含有两个以上文件的目录，在第一次触发回调函数时，在回调函数中以正常参数调用av_sdk_pause，返回1；第一次回调结束后，后续不再有回调函数触发；以正常参数调用av_sdk_resume，后续有回调函数触发，且后续回调函数触发的总个数等于文件个数减1。
    test_number = "11";
    av_sdk_stop(ptr);
    golbal_scancount = 0;
    ret_status = av_scan_target(ptr,option,path,testscannotify4,nullptr);
    while(golbal_scancount != 1){
        av_sdk_stop(ptr);
        golbal_scancount = 0;
        ret_status = av_scan_target(ptr,option,path,testscannotify4,nullptr);
    }
    golbal_scancount = 0;
    sleep(WAITMINUTE*60);
    if (golbal_scancount != 0){
        print_test_failed_message("第一次回调结束后，后续不再有回调函数触发");
    }
    golbal_scancount = 0;
    ret_status = av_sdk_resume(ptr);
    if (ret_status !=1 ){
        print_test_failed_message("以正常参数调用av_sdk_resume");
    }
    sleep(WAITMINUTE*60);
    if (golbal_scancount == 0){
        print_test_failed_message("后续有回调函数触发");
    }
    if (golbal_scancount != FILECOUNT-1){
        print_test_failed_message("且后续回调函数触发的总个数等于文件个数减1");
    }
    //test 12 以正常参数调用av_scan_target；在确认扫描结束之前，在回调函数之外，以正常参数调用av_sdk_stop，返回1；后续不再有回调函数触发。
    test_number = "12";
    av_sdk_stop(ptr);
    ret_status = av_scan_target(ptr,option,path,testscannotify,nullptr);
    while(golbal_scancount != FILECOUNT){
        printf("[i] scan not finished, wait for 10 seconds");
        ret_status = av_sdk_stop(ptr);
        if (ret_status != 1){
            print_test_failed_message("以正常参数调用av_scan_target；在确认扫描结束之前，在回调函数之外，以正常参数调用av_sdk_stop，返回1");
        }
        av_sdk_resume(ptr);
        sleep(10);
    }
    golbal_scancount = 0;
    sleep(WAITMINUTE*60);
    if (golbal_scancount != 0){
        print_test_failed_message("后续不再有回调函数触发");
    }
    //test 13 以正常参数调用av_scan_target；在确认扫描结束之后，在回调函数之外，以正常参数调用av_sdk_pause，返回1；后续不再有回调函数触发；以正常参数调用av_sdk_resume，后续有回调函数触发，且总回调函数个数等于文件个数。
    test_number = "13";
    av_sdk_stop(ptr);
    ret_status = av_scan_target(ptr,option,path,testscannotify,nullptr);
    while(golbal_scancount != FILECOUNT){
        printf("[i] scan not finished, wait for 10 seconds");
        sleep(10);
    }
    golbal_scancount = 0;
    ret_status = av_sdk_pause(ptr);
    if (ret_status != 1){
        print_test_failed_message("以正常参数调用av_scan_target；在确认扫描结束之后，在回调函数之外，以正常参数调用av_sdk_pause，返回1");
    }
    sleep(WAITMINUTE*60);
    if (golbal_scancount != 0){
        print_test_failed_message("后续不再有回调函数触发");
    }
    golbal_scancount = 0;
    ret_status = av_sdk_resume(ptr);
    sleep(WAITMINUTE*60);
    if (golbal_scancount == 0){
        print_test_failed_message("以正常参数调用av_sdk_resume，后续有回调函数触发");
    }
    if (golbal_scancount != FILECOUNT){
        print_test_failed_message("且总回调函数个数等于文件个数");
    }
    //擦屁股
    if (!ptr){
        av_sdk_uninit(ptr);
    }
}
//finished
void TEST_av_list_file_in_guarantine_and_av_restore_isolation(){
    FUNCTIONSTARTINFO;
    const char * test_number="";
    uint32_t ret_status = 0;
    void * ptr_sdk = av_sdk_init();
    global_ptr = ptr_sdk;
    int retry_count = 0;
    // void * ptr = nullptr;
    global_quarantinefilecount = 0;
    //test 01 global_specialpath以ptr_sdk为空指针调用av_list_isolation，程序崩溃。
    test_number = "01";
    ret_status = av_list_isolation(nullptr,testlistnotify,nullptr);
    if (ret_status != 0){
        print_test_failed_message("以ptr_sdk为空指针调用av_list_isolation，程序崩溃",test_number);
    }
    //test 02 以ptr_sdk为空指针调用av_restore_isolation，程序崩溃。
    test_number = "02";
    ret_status = av_restore_isolation(nullptr,"");
    if (ret_status != 0){
        print_test_failed_message("以ptr_sdk为空指针调用av_restore_isolation，程序崩溃",test_number);
    }
    //test 03 以ptr_notify为空指针其它参数正常调用av_list_isolation，返回0。
    test_number = "03";
    ret_status = av_list_isolation(ptr_sdk,nullptr,nullptr);
    if (ret_status != 0){
        print_test_failed_message("以ptr_notify为空指针其它参数正常调用av_list_isolation，返回0。",test_number);
    }
    //test 04 以ptr_name为空指针其它参数正常调用av_restore_isolation，返回0。
    test_number = "04";
    ret_status = av_restore_isolation(ptr_sdk,nullptr);
    if (ret_status != 0){
        print_test_failed_message("以ptr_name为空指针其它参数正常调用av_restore_isolation，返回0",test_number);
    }
    //test 05 以所有参数正常时调用av_list_isolation，返回1。
    test_number = "05";
    global_quarantinefilecount = 0;
    global_virusnewfilename = nullptr;
    ret_status = av_list_isolation(ptr_sdk,testlistnotify,nullptr);
    if (ret_status != 1){
        print_test_failed_message("以ptr_sdk为空指针调用av_list_isolation，程序崩溃",test_number);
    }
    //test 06 以所有参数正常时调用av_restore_isolation，返回1。
    test_number = "06";
    while (global_virusnewfilename == nullptr){
        sleep(1);
    }
    ret_status = av_restore_isolation(ptr_sdk,global_virusnewfilename);
    if (ret_status != 1){
        print_test_failed_message("以ptr_sdk为空指针调用av_restore_isolation，程序崩溃",test_number);
    }
    //test 07 调用av_scan_target扫描一个病毒文件，保证有文件被隔离；以所有参数正常时调用av_list_isolation，返回1；触发ListNotify回调函数时，函数参数file_old、file_new、sha不为空，size参数不为0。
    global_testnumber = test_number = "07";
    golbal_scancount = 0;
    while (golbal_scancount != 1){
        ret_status = av_scan_target(ptr_sdk,global_scanoption,global_specialpath,testscannotify,nullptr);
        if (ret_status != 1){
            golbal_scancount = 0;
            print_test_failed_message("调用av_scan_target扫描一个病毒文件",test_number);
        }
        if (retry_count == global_retrycount){
            retry_count = 0;
            print_test_failed_message("[retry] 调用av_scan_target扫描一个病毒文件 failed\n",test_number);
        }
    }
    ret_status = av_list_isolation(ptr_sdk,testlistnotify1,nullptr);
    if (ret_status != 1){
        print_test_failed_message("保证有文件被隔离；以所有参数正常时调用av_list_isolation，返回1",test_number);
    }
    //test 08 调用av_scan_target扫描一个病毒文件，保证有文件被隔离；以所有参数正常时调用av_list_isolation，返回1；触发ListNotify回调函数时，如果file_old指向的文件不存在，以正常参数调用av_restore_isolation，返回1。确认file_old指向的文件存在。
    global_testnumber = test_number = "08";
    golbal_scancount = 0;
    if (av_restore_isolation(ptr_sdk,global_virusnewfilename) != 1){
        printf("[-] restore last test environment failed\n");
    }
    while (golbal_scancount != 1){
        ret_status = av_scan_target(ptr_sdk,global_scanoption,global_specialpath,testscannotify,nullptr);
        if (ret_status != 1){
            golbal_scancount = 0;
            print_test_failed_message("调用av_scan_target扫描一个病毒文件",test_number);
        }
        if (retry_count == global_retrycount){
            retry_count = 0;
            print_test_failed_message("[retry] 调用av_scan_target扫描一个病毒文件 failed\n",test_number);
        }
    }
    ret_status = av_list_isolation(ptr_sdk,testlistnotify2,nullptr);
    if (ret_status != 1){
        print_test_failed_message("保证有文件被隔离；以所有参数正常时调用av_list_isolation，返回1",test_number);
    }
    //test 09 调用av_scan_target扫描一个病毒文件，保证有文件被隔离；以所有参数正常时调用av_list_isolation，返回1；触发ListNotify回调函数时，如果file_old指向的文件不存在，获取file_new；以获取的file_new为参数，其它参数正常调用av_restore_isolation，返回1。确认file_old指向的文件存在。
    test_number = "09";
    printf("[i] test 09 is more likely test 08,not test\n");
    //test 10 调用av_scan_target扫描一个病毒文件，保证有文件被隔离；以所有参数正常时调用av_list_isolation，返回1；触发ListNotify回调函数时，如果file_old指向的文件存在，以file_new为参数，其它参数正常调用av_restore_isolation，返回1。确认file_old指向的文件已经被覆盖（修改日期）。
    global_testnumber = test_number = "10";
    golbal_scancount = 0;
    if (av_restore_isolation(ptr_sdk,global_virusnewfilename) != 1){
        printf("[-] restore last test environment failed\n");
    }
    while (golbal_scancount != 1){
        ret_status = av_scan_target(ptr_sdk,global_scanoption,global_specialpath,testscannotify,nullptr);
        if (ret_status != 1){
            golbal_scancount = 0;
            print_test_failed_message("调用av_scan_target扫描一个病毒文件",test_number);
        }
        if (retry_count == global_retrycount){
            retry_count = 0;
            print_test_failed_message("[retry] 调用av_scan_target扫描一个病毒文件 failed\n",test_number);
        }
    }
    ret_status = av_list_isolation(ptr_sdk,testlistnotify4,nullptr);
    if (ret_status != 1){
        print_test_failed_message("保证有文件被隔离；以所有参数正常时调用av_list_isolation，返回1",test_number);
    }
    //擦屁股
    if (!ptr_sdk){
        av_sdk_uninit(ptr_sdk);
    }
}

void TEST_process(){
    cout << "[+] strart running test"<<endl;
    TEST_av_sdk_init_andav_sdk_uninit();
    TEST_av_scan_target();
    TEST_av_scan_stop_and_av_sdk_pause_and_av_scan_resume();
    TEST_av_list_file_in_guarantine_and_av_restore_isolation();
    cout << "[+] running test finished"<<endl;
}

int main(int argc, char const *argv[])
{
    if (!check_environment()){
        printf("---- environment check failed , exit ----\n");
        return 0;
    }
    printf("*** NOTE : the  philosophy of linux is : the best result is no output ***\n");
    // test_file_count();
    test_scan_time();
    /* code */
    TEST_process();
    printf("---- all test finished ----\n");
    return 0;
}
