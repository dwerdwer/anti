#!/bin/bash
### BEGIN INIT INFO
# Required-Start:   $remote_fs $syslog
# Required-Stop:    $remote_fs $syslog
# Default-Start:    
# Default-Stop:     
# chkconfig:         2345 70 30
# Provides:          run_vc_client.sh 
# Short-Description: Start daemon at boot time
# Description:       Enable service provided by daemon
### END INIT INFO

execute_path=/usr/local/vc_linux_client/bin/
execute_bin=./vc_client_daemon

lib_path=/usr/local/vc_linux_client/lib

echo "	"
# Check if execute_bin exists
ps -fe|grep "$execute_bin"| grep -v grep

if [ $? -ne 0 ]
then
    echo "start running"
    echo "	"
else
    echo "vc_linux_client already running"
    echo "	"
    exit 0
fi
# placeholder 

export LD_LIBRARY_PATH=$lib_path:$LD_LIBRARY_PATH

if [ ! -d "$execute_path" ]
then
    echo "$execute_path" "not find"
    exit 1
fi

cd $execute_path

if [ ! -x "$execute_bin" ]
then
    echo "$execute_bin" "cannot execute"
    exit 1
else
    $execute_bin
fi

exit 0
