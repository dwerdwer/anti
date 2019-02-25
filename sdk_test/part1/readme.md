# 本程序为Linux杀毒sdk测试程序

## 文档信息
|作者|联系方式|说明|时间|
|:-:|:-:|:-:|:-:|
|叶智慧|yezhihui@jiangmin.com|增加文档内容|20181126 10:59|
|叶智慧|yezhihui@jiangmin.com|增加测试说明|20181127 17:06|
|叶智慧|yezhihui@jiangmin.com|增加演示程序使用说明|20181130 10:40|

### 需要软件：
unzip
g++
ruby(开发可能需要用到)


### 文件结构
`linux_kv_sdk_test.zip`内容
```text
|-build/
|   |-main_daemon                                       主要daemon程序

|-etc/                                                  main_daemon配置文件目录,main_daemon调用时是 ../etc/*.xml
|   |-customize_edr.xml                                 main_daemon程序挂载的配置文件
|   |-edr_config.xml
|   |-linux_client.xml
|   |-module_config.xml
|   |-vc_daemon_config.xml
|-scripts/     
|   |-restart_daemon.sh                                 重启daemon脚本
|   |-build_sdk_test.sh                                 生成并运行sdk测试程序脚本
|   |-build_sdk_test_show.sh                            生成并运行sdk测试程序脚本
|   |-run_daemon.sh                                     运行daemon的脚本
|   |-log.txt                                           main_daemon程序调用日志记录
|-lib/                                                  main_daemon程序动态链接库依赖目录
|-log/
|   |-edr_client_log                                    main_daemon程序运行日志记录
|-virus_library/                                        病毒库文件
|   |-modules/                                          扫描病毒模块文件
|-sdk_includes/
|   |-antivirus_interface.h                             sdk提供的对外接口
|-sdk_executable/
|   |-run_modify_load                                   修改daemon挂载插件(.xml)的小程序
|-sdk_libs/                                             sdk_test.cpp所用链接库的位置
|   |-librpc_client.so
|   |-libavxsdk.so
|-sdk_scan_path/                                        测试扫描目录，路径下最少需要两个文件
|-sdk_special_scan_path/                                测试扫描目录，路径下只含有一个病毒文件的路径
|-json.hpp                                              测试（演示）程序依赖
|-sdk_test.run                                          预编译的sdk测试程序，如果不能使用，则需要按照说明手动编译
|-sdk_test.cpp                                          sdk测试程序
|-sdk_test.log                                          sdk测试程序的日志记录文件
|-sdk_test_show.cpp                                     sdk测试程序的演示程序
|-sdk_test_show.run                                     预编译的sdk测试程序的演示程序，如果不能使用，则需要按照说明手动编译
|-scan_config.json                                      开发时的扫描配置
|-generate_scan_config_json.rb                          生成scan_config.json的ruby脚本      
|-sdk_test_show_list.txt                                sdk演示程序的使用说明           
-readme文件
```
### 使用说明：
以下命令：

`zen@os:~$ ls`以普通用户（我这里是zen用户，你的可能不同）运行`ls`命令，当前在`~`目录下，即`/home/zen`目录下

`root@os:/home/zen# pwd` 以超级用户root运行`pwd`命令，当前在`/home/zen`目录下

使用时只需要在相同目录下用相关的用户输入相同命令即可，不要复制前面类似于`zen@os:~$ `或者`root@os:/home/zen# `的命令提示符

`zen@os:`在每台机器上不同，格式为`用户名@主机名`，根据自己实际情况来


### 测试人员使用方式：

配置环境，解压文件：
```shell
zen@os:/home# mkdir jiangmin_antivirus
zen@os:~$ cp linux_kv_sdk_test.zip /home/jiangmin_antivirus
zen@os:/home/jiangmin_antivirus$ unzip linux_kv_sdk_test.zip
zen@os:/home/jiangmin_antivirus/linux_kv_sdk_test$ chmod +x build/main_daemon
zen@os:/home/jiangmin_antivirus/linux_kv_sdk_test$ chmod +x scripts/*
zen@os:/home/jiangmin_antivirus/linux_kv_sdk_test$ chmod +x sdk_executable/run_modify_load
```

将普通测试文件放到`/home/jiangmin_antivirus/linux_kv_sdk_test/sdk_scan_path`目录下，至少保证有两个文件（可以包含有可疑文件）

将一个病毒文件放到`/home/jiangmin_antivirus/linux_kv_sdk_test/sdk_special_scan_path`目录下

按照以上建立指定文件夹等操作在本文里称之为"默认配置"

编译测试程序：
```shell
zen@os:/home/jiangmin_antivirus/linux_kv_sdk_test/scripts$ ./build_sdk_test.sh
```

启动杀毒后台进程：
```shell
zen@os:/home/jiangmin_antivirus/linux_kv_sdk_test/scripts$ ./run_daemon.sh #请已此方式运行而不是sh run_daemon.sh运行，下同
```

再开一个终端，运行测试程序：
```shell
zen@os:/home/jiangmin_antivirus/linux_kv_sdk_test$ export LD_LIBRARY_PATH=$PWD/sdk_libs
zen@os:/home/jiangmin_antivirus/linux_kv_sdk_test$ ./sdk_test.run --help #先查看提示信息
zen@os:/home/jiangmin_antivirus/linux_kv_sdk_test$ ./sdk_test.run #根据默认配置运行测试程序
```

测试程序使用说明，菜单如下：
```text
====================================================================================================================================================
|                                               This application is designed for testing jiangmin kv sdk                                           |
----------------------------------------------------------------------------------------------------------------------------------------------------
|  Usege:                                                                                                                                          |
|      --run_daemon_script=<path_with_name>                script(recommend beacause of environment variable) or program to run main_daemon        |
|                                                          dafault: /home/jiangmin_antivirus/linux_kv_sdk_test/scripts/run_daemon.sh               |
|      --config_xml=<path_with_name>                       main_daemon config .xml path,generally with some path relation to main_daemon           |
|                                                          default: /home/jiangmin_antivirus/linux_kv_sdk_test/etc/customize_edr.xml               |
|      --run_modify_load=<path_with_name>                  a small tool that modify .xml some arguments                                            |
|                                                          default: /home/jiangmin_antivirus/linux_kv_sdk_test/sdk_executable/run_modify_load      |
|      --kv_plugin_name=<name>                             main_daemon config .xml plugin name you want to test                                    |
|                                                          default: sysinfo                                                                        |
|      --restart_script=<path_with_name>                   script used to restart main_daemon                                                      |
|                                                          default: /home/jiangmin_antivirus/linux_kv_sdk_test/scripts/restart_daemon.sh           |
|      --scan_path=<path>                                  generally scan path,make sure at least include 2 normal files                           |
|                                                          default: /home/jiangmin_antivirus/linux_kv_sdk_test/sdk_scan_path                       |
|      --special_scan_path=<path>                          special scan path,make sure just include ONE virus files                                |
|                                                          default: /home/jiangmin_antivirus/linux_kv_sdk_test/sdk_special_scan_path               |
|      --scan_option=<option>                              scan option: Unzip(default), Unpack, StopOnOne, ProgramOnly, OriginalMd5,               |
|                                                                  UseFigner, UseCloud, Backup, ForceUnzip                                         |
|      --log_method=<method>                               custome log method: 0-show only errors(stdout,default), 1-show all(stdout),             |
|                                                          2-show only errors(file), 3-show all(file), log file default name: sdk_test.log         |
|      --help                                              print this infomation                                                                   |
----------------------------------------------------------------------------------------------------------------------------------------------------
|  Example:                                                                                                                                        |
|  ./sdk_test.run --run_daemon_script=/path/with/name --config_xml=/path/with/name --run_modify_load=/path/with/name --kv_plugin_name=sysinfo \    |
|          --restart_script=/path/with/name --scan_path=/path --special_scan_path=/path --scan_option=Unzip --log_method=1                         |
----------------------------------------------------------------------------------------------------------------------------------------------------
|  author: yezhihui@jiangmin.com   codedate: 20181127                                                                                              |
====================================================================================================================================================
```
如果按照以上步骤操作，即"默认配置"，启动程序时直接运行`./sdk_test.run`即可

程序帮助菜单里有说明（见每个参数的default字段），如果环境不对，可指定相关参数

程序输出由`--log_method`指定，默认输出到终端，只显示错误信息


### 开发人员使用方式：
解压

编译出测试程序： `gcc -std=c++11 sdk_test.cpp -lavxsdk -lrpc_client -L(libavxsdk.so和librpc_client.so的位置) -o 生成可执行程序的名称`
`main_daemon`和其配置文件由于依赖问题，编写启动`main_daemon`程序的脚本，保证单独调用此脚本可以正确运行`main_daemon`程序，正确运行后有类似输出：
```shell
省略
Create Worker Success !
Message:省略
init scanner success
start upload thread success
省略
```

运行可执行程序时指定相关参数，脚本功能必须正确

### 演示程序使用方式：
先按照　"测试人员使用方式"　编译出测试程序，运行测试程序时记录测试通过的`chapter`和`test`

编译演示程序：
```shell
zen@os:/home/jiangmin_antivirus/linux_kv_sdk_test/scripts$ ./build_sdk_test_show.sh
```

运行演示程序：
```shell
zen@os:/home/jiangmin_antivirus/linux_kv_sdk_test$ export LD_LIBRARY_PATH=$PWD/sdk_libs
zen@os:/home/jiangmin_antivirus/linux_kv_sdk_test$ ./sdk_test_show.run -h #先查看提示信息
zen@os:/home/jiangmin_antivirus/linux_kv_sdk_test$ ./sdk_test_show.run -c 1 -t 2 #假设需要测试chapter 01 - test 02
```

测试演示程序使用说明，菜单如下
```text
------------------------------------------------------------------------
|This application is designed for showing special test jiangmin kv sdk |
------------------------------------------------------------------------
|  Usage:                                                              |
|      -c                          Chapter you want to show test       |
|      -t                          test Number in chapter              |
|      -h                          print this help                     |
------------------------------------------------------------------------
|  Example:                                                            |
|      ./sdk_test_show -c 1 -t 2                                       |
------------------------------------------------------------------------
|  Author: yezhihui@jiangmin.com Codedate: xxxxxxxx                    |
------------------------------------------------------------------------
```
使用`-c`参数指定测试`chapter`,`-t`参数指定`test number`,两个参数都需要，具体参数详情

演示程序其余的配置(如重启脚本，配置文件等)同测试程序，手动指定运行参数时需要先用ruby脚本生成对应json,然后`g++`编译时加上`-DADVANCE`参数，在生成的新版可执行程序运行时指定隐藏的`-j`参数

注意，传递给测试程序的`-c`和`-t`参数不要在数字前加`0`

程序默认输出结果到终端

### 日志结果说明
```text
[+] chapter 02 - test 01 ---> some message
[-] chapter 02 - test 02 ---> some message
[i] chapter 00 - test 00 ---> some message
```

结果类似于上面所示

`[+]`表示测试成功，`[-]`表示测试失败，`[i]`表示信息（用于调试）

`chapter 00`表示初始化，`chapter 01`表示测试第一部分，`test 01`表示第一个测试

后面紧接着测试提示信息，可以定位到具体测试到哪里

### TODO
1. 部分接口尚未完成，尚未提供新版测试文档和测试用例
2. 接口数据尚未确定，暂时只判断是否有数据
3. 演示程序问题同测试程序

### 错误说明：
Q: `main_daemon`程序或者脚本无法运行成功
***
A: 原因如下
1. `main_daemon`和其配置文件存在相对路径问题，可参考`linux_kv_sdk_test.zip`目录结构
2. `main_daemon`程序依赖问题，需在启动脚本里设置正确的`LD_LIBRARY_PATH`环境变量
3. `main_daemon`程序本身问题，如确定有此问题，找开发（或此文档提供者）要

Q: `sdk_test.run`或者`sdk_test_show.run`无法编译或者运行
***
A:　请加入`LD_LIBRARY_PATH`环境变量，变量值为`sdk_libs`的绝对路径，如在当前终端运行命令`LD_LIBRARY_PATH=/home/jiangmin_antivirus/linux_kv_sdk_test/sdk_libs:$LD_LIBRARY_PATH`

其他原因请邮件反馈