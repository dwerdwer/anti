#!/bin/bash

stap_compiler="compile_edr_monitor.sh"

if [ $# -ne 1 ]; then
    echo "please set systemtap.tar.gz "
    exit -1
elif [ ! -e $1 ]; then
    echo "$1 doesn't exist !"
    exit -1
fi

cwd=`pwd`
abs_dir_name="$1"
name="${abs_dir_name##*/}"
base_name="${name%.tar.gz}"


rm ${base_name}_backup -rf
if [ -e ${base_name} ]; then
    mv ${base_name} ${base_name}_backup
fi

tar zxf ${abs_dir_name} -C ${cwd}
if [ $? -ne 0 ]; then
    echo "tar zxf ${abs_dir_name} -C ${cwd} -> $? , error"
    exit -1
fi

cd ${base_name}/bin/
bash ${stap_compiler} >./${stap_compiler}.log 2>&1
if [ $? -eq -1 ]; then
    echo "compile systemtap error, log in ${stap_compiler}.log"
    exit -1
fi
cp *.ko ${cwd}/lib/ -f
rm ${base_name} -rf
rm ${abs_dir_name} -f