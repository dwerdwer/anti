#!/bin/sh

if [ $# -lt 1 ] 
then
    echo "resart error missing args"
    exit 1
else
    execute_bin=$1
fi

if [ ! -f $execute_bin ] 
then
    echo "resart error target: $execute_bin"
    exit 1
fi

target_dir=${execute_bin%/*}

if [ ! -d $target_dir ] 
then
    echo "resart error target: $target_dir"
    exit 1
fi
echo "------------------------------------------------------------------" >>/tmp/log
execute_bin=${execute_bin##*/}

cd $target_dir

ps -fe|grep "\./$execute_bin"| grep -v grep 2>&1

if [ $? -ne 0 ]
then
    echo "no daemon, run .."
    nohup ./$execute_bin >>/tmp/log 2>&1 &
    cd - > /dev/null 2>&1
exit 0 
else
	echo "	"
fi

kill -9 `ps aux | grep "\./$execute_bin"|grep -v grep| awk '{print $2}'`
echo "kill daemon, run .."
sleep 5
nohup ./$execute_bin >>/tmp/log 2>&1 &

cd - > /dev/null 2>&1
#echo "finish"

