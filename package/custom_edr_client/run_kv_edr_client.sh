#!/bin/bash
### BEGIN INIT INFO
# Required-Start:   $remote_fs $syslog
# Required-Stop:    $remote_fs $syslog
# Default-Start:    
# Default-Stop:     
# chkconfig:         2345 70 30
# Provides:          run_kv_edr_client.sh 
# Short-Description: Start daemon at boot time
# Description:       Enable service provided by daemon
### END INIT INFO

clean_kernel_mod_script=./clean_kmod.sh

execute_path=/usr/local/kv_edr_client/bin/
execute_bin=./kv_edr_daemon

kv_edr_lib_path=/usr/local/kv_edr_client/lib

echo "	"
# Check if kv_edr_client exists
ps -fe|grep "$execute_bin"| grep -v grep

if [ $? -ne 0 ]
then
    echo "kv_edr_daemon start running"
    echo "	"
else
    echo "kv_edr_daemon already running"
    echo "	"
    exit 0
fi

# export LD_LIBRARY_PATH=$kv_edr_lib_path:$LD_LIBRARY_PATH

if [ ! -d "$execute_path" ];
then
    echo "$execute_path" "not find"
    exit 1
fi

# clean kernel mod
sh $clean_kernel_mod_script

cd $execute_path

if [ ! -x "$execute_bin" ];
then
    echo "$execute_bin" "cannot execute"
    exit 1
else
    nohup $execute_bin >/usr/local/kv_edr_client/res.txt 2>&1 &
    # $execute_bin
fi

exit 0
