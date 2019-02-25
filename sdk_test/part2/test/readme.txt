
sdk_test 

此程序专用于测试 kv_engine.h 的接口 
必须保证执行目录可以访问 "../../tools/run_modify_load" 与 "../../tools/restart_daemon.sh"

程序内部会依次调用每个测试用例 顺序对照 test_list.txt

若程序内部define了 CONTINUE_RUN 某个测试函数出错也会继续运行(default)

-d daemon 程序绝对路径

-f 指定配置文件路径

例:

./sdk_test -d ~/svn/virus_checking_linux_client/build/main_daemon -f test_config.xml


