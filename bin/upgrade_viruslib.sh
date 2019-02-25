#!/bin/bash


install_path="/usr/local/LinuxClient/"
daemon_dir="virus_checking_linux_client"
daemon_exec="main_daemon"

web_package=$1
tar_name=$(basename ${web_package})
virus_library=$(basename ${tar_name} .tar.gz)

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

function backup_virus_library()
{
    rm -rf ${install_path}/${daemon_dir}/${virus_library}_backup
    if [ -d ${install_path}/${daemon_dir}/${virus_library} ]; then
        echo "starting backup_virus_library"
        mv ${install_path}/${daemon_dir}/${virus_library} ${install_path}/${daemon_dir}/${virus_library}_backup
    fi
}

function revert_virus_library()
{
    if [ -d ${install_path}/${daemon_dir}/${virus_library}_backup ]; then
        echo "starting revert_virus_library"
        rm -rf ${install_path}/${daemon_dir}/${virus_library}
        mv ${install_path}/${daemon_dir}/${virus_library}_backup ${install_path}/${daemon_dir}/${virus_library}
        return $?
    fi
    return 255
}

function decompress_tar_package()
{
    echo "starting decompress [${tar_name}] to [${install_path}/${daemon_dir}]"
    tar zxf ${tar_name} -C ${install_path}/${daemon_dir}
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


echo "`date` start upgrade virus_library"

kill_daemon_process
backup_virus_library

cd /tmp
download_package
if [ $? -ne 0 ]; then
    revert_virus_library
    exit 255
fi

decompress_tar_package
if [ $? -ne 0 ]; then
    echo "decompress_tar_package failed, error"
    revert_virus_library
    exit 255
fi

start_daemon_process

echo "finally:upgrade virus_library success"
exit 0