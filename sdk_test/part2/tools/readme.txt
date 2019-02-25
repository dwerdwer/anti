
1. run_modify_load

此程序专用于修改 Linux 客户端配置文件挂载选项

-f 指定配置文件路径

-m 指定模块名

-l [y/n] 选择是否挂载

例:

./run_modify_load -f test_config.xml -m file_monitor -l y

before:
<module name="file_monitor" path="libfile_monitor.so" load="no" category="file_monitor" isolation="yes">

after:
<module name="file_monitor" path="libfile_monitor.so" load="yes" category="file_monitor" isolation="yes">


2. restart_daemon.sh

重启 daemon 程序 传入绝对路径(必须)

例:
./restart_daemon.sh /home/snail/svn/virus_checking_linux_client/build/main_daemon

