#!/bin/sh

execute_bin=kv_edr_daemon

echo "	"

# Check if execute_bin exists
ps -fe|grep "$execute_bin"| grep -v grep

if [ $? -ne 0 ]
then
    echo "kv_edr_daemon already killed"
	echo "	"
exit 0 
else
	echo "now kill kv_edr_daemon"
	echo "	"
fi

sudo kill -9 `ps aux | grep "$execute_bin"|grep -v grep| awk '{print $2}'`

