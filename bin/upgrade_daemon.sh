#!/bin/bash

install_path="/usr/local/LinuxClient/"
daemon_dir="virus_checking_linux_client"
backup_dir="${install_path}/${daemon_dir}_backup"
daemon_exec="main_daemon"
stap_compiler="compile_stap.sh"
stap_tar_name="kernel-3.10.0-stap.tar.gz"
ip_config="center_ip.txt"


web_package="$1"
tar_name="$(basename ${web_package})"
version_file="$2"

function revert_daemon_proc()
{
    echo "starting old execute daemon"
    cd ${install_path}/${daemon_dir}/bin/
    ./${daemon_exec}
    sleep 2
    old_procs=`ps aux | grep ${daemon_exec} | grep -v grep | awk '{print $2}'`
    if [ "${old_procs}" != "" ]; then
        echo "detect old process running on pid ${old_procs}"
        echo "upgrade revert normally exit"
    else
        echo "detect old process running failed"
        echo "upgrade revert failed"
    fi
}

function revert_daemon_dir()
{
    echo "starting revert old version daemon"
    rm -rf ${install_path}/${daemon_dir}_failed/
    mv ${install_path}/${daemon_dir}/ ${install_path}/${daemon_dir}_failed/
    mv ${backup_dir} ${install_path}/${daemon_dir}
}

function kill_daemon_process()
{
    echo "starting kill old daemon process"
    old_procs=`ps aux | grep ${daemon_exec} | grep -v grep | awk '{print $2}'`
    for proc in ${old_procs}
    do
        kill -9 ${proc}
        echo "killed ${proc}"
    done

    echo "starting rm old ko"
    old_kos=`lsmod | grep edr | awk '{print $1}'`
    for ko in ${old_kos}
    do
        rmmod ${ko}
        echo "rmmod ${ko}"
    done
}

function backup_daemon_dir()
{
    echo "starting backup old daemon dir"
    rm -rf ${backup_dir}
    mv ${install_path}/${daemon_dir} ${backup_dir}
}

function download_package()
{
    echo "starting download [${web_package}] -> [${PWD}/${tar_name}]"

    which curl
    if [ $? -ne 0 ]; then
        echo "no curl finded, error"
        return 255
    fi

    curl ${web_package} -O
    ret=$?
    if [ ${ret} -ne 0 ]; then
        echo "curl ${web_package} -O error:${ret}"
        return 255
    elif [ ! -e ${tar_name} ]; then
        echo "${tar_name} doesn't exist"
        return 255
    fi
    echo "downloaded ${tar_name}"
    return 0
}

function decompress_tar_package()
{
    echo "starting decompress [${tar_name}] to [${install_path}]"
    tar zxf ${tar_name} -C ${install_path}
    return $?
}

function compile_systemtap()
{
    echo "starting compile systemtap"

    if [ ! -d ${install_path}/${daemon_dir} ]; then
        echo "${install_path}/${daemon_dir} is not director, error"
        return 255
    fi
    cd ${install_path}/${daemon_dir}/

    if [ ! -e ${install_path}/${daemon_dir}/${stap_compiler} ]; then
        echo "${install_path}/${daemon_dir}/${stap_compiler} doesn't exist, error"
        return 255
    fi
    bash ${stap_compiler} ${stap_tar_name}
    return $?
}

function start_daemon_process()
{

    if [ ! -x ${install_path}/${daemon_dir}/bin/${daemon_exec} ]; then
        echo "${install_path}/${daemon_dir}/bin/${daemon_exec} can't be execute, error"
        return 255
    fi

    echo "starting new execute daemon"
    cd ${install_path}/${daemon_dir}/bin/
    ./${daemon_exec}
    sleep 2
    new_procs=`ps aux | grep ${daemon_exec} | grep -v grep | awk '{print $2}'`
    if [ "${new_procs}" = "" ]; then
        echo "detect new process running failed"
        echo "upgrade failed"
        return 255
    fi
    echo "detect new process running on pid ${new_procs}"
    return 0
}

echo "`date` start upgrade daemon"

kill_daemon_process
backup_daemon_dir

cd /tmp
download_package
if [ $? -ne 0 ]; then
    revert_daemon_dir
    revert_daemon_proc
    exit 255
fi

decompress_tar_package
if [ $? -ne 0 ]; then
    echo "decompress_tar_package failed, error"
    revert_daemon_dir
    revert_daemon_proc
    exit 255
fi

echo "cp -f ${backup_dir}/etc/${ip_config} ${install_path}/${daemon_dir}/etc/${ip_config}"
echo "cp -f ${version_file} ${install_path}/${daemon_dir}/etc/"
cp -f ${backup_dir}/etc/${ip_config} ${install_path}/${daemon_dir}/etc/${ip_config}
cp -f ${version_file} ${install_path}/${daemon_dir}/etc/

compile_systemtap
if [ $? -ne 0 ]; then
    echo "compile systemtap error"
    # revert_daemon_dir
    # revert_daemon_proc
    # exit 255
fi

start_daemon_process
if [ $? -ne 0 ]; then
    echo "start daemon process error"
    revert_daemon_dir
    revert_daemon_proc
    exit 255
fi

echo "finally:upgrade daemon success"
exit 0