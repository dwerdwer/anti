#!/bin/bash


echo "`date` start download version file"

if [ -z "${1}" ] || [ -z "${2}" ]; then
    echo "params error"
    exit 255
fi

remote_url="${1}"
remote_file=$(basename ${remote_url})

download_dir="${2%/*}"
download_file=$(basename ${2})

function download_package()
{
    echo "starting download [${remote_url}] -> [${PWD}/${remote_file}]"

    which curl
    if [ $? -ne 0 ]; then
        echo "no curl finded, error"
        return 255
    fi

    curl ${remote_url} -O
    ret=$?
    if [ ${ret} -ne 0 ]; then
        echo "curl ${remote_url} -O error:${ret}"
        return 255
    elif [ ! -e ${remote_file} ]; then
        echo "${remote_file} doesn't exist"
        return 255
    fi
    echo "downloaded ${remote_file}"
    return 0
}

cd ${download_dir}
download_package
if [ $? -eq 0 ]; then
    exit 0
fi
exit 255