#!/bin/bash

PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin

if [ $# -ne 1 ]; then
    echo "please input what to run"
fi

dir_exe="$1"
exe="${dir_exe##*/}"
dir="${dir_exe%/*}"
log="/tmp/run_${exe}.log"

proc=`ps axu | awk '{ print $11 }' | grep "${exe}"`
if [ -z "${proc}" ]; then
    echo "`date` start running [${exe}]" >>${log}
    cd ${dir}
    ./${exe}
else
    echo "`date` already running [${exe}] on pid [${proc}]" >>${log}
fi
