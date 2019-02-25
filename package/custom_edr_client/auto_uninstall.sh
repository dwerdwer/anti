#!/bin/bash

RETURN_SUCCESS=0
ERROR_REPEAT_RUN=16
ERROR_WRITE_REG=17
ERROR_NOT_INSTALL=32
ERROR_INSTALLED=33
ERROR_RUNNING=34
ERROR_NEED_REBOOT=35



kv_exe="kv_edr_daemon"
uninstall_script=./uninstall.sh
reboot_shell=run_kv_edr_client.sh
install_path=/usr/local/kv_edr_client
kill_shell_name=$install_path/script/kill_kv_edr_client.sh


# ensure self is not running before
# repeat_run_num=`ps aux | grep $0 | grep -v grep | wc -l`
# if [[ $repeat_run_num -gt 1 ]]
# then
#     echo "$0 is already running"
#     exit $ERROR_REPEAT_RUN
# fi


# waiting for $kv_exe exit
kv_pid=0
while ((1));
do
    if [[ $kv_pid = 0 ]]
    then
        kv_pid=`ps aux | grep $kv_exe | grep -v grep | awk '{print $2}'`
        if [[ $kv_pid -eq 0 ]]
        then
            echo "$kv_exe is not start."
            break
        fi
    else
        kv_proc="/proc/$kv_pid"
        if [ ! -d $kv_proc ]
        then
            echo "$kv_exe is stop."
            break
        else
            sleep 1
        fi
    fi
done

# uninstall $kv_exe
echo "uninstall start"
$uninstall_script
echo "uninstall finish"

exit $ERROR_NEED_REBOOT
