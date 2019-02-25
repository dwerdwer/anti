#!/bin/sh

#param1: package file-path name
#param2: http or https
#param3: ip:port

PACKAGE_ROOT=LinuxClient_release
CONFIG_DIR=${PACKAGE_ROOT}/daemon/etc
CONFIG_FILE_NAME=customize_edr.xml

echo ${1} ${2} ${3}

tar -zx -f ${1}
echo ${2} ${3} ${CONFIG_DIR}/${CONFIG_FILE_NAME}
./replace_ip.sh ${2} ${3} ${CONFIG_DIR}/${CONFIG_FILE_NAME}
tar -zc -f ${1} ${PACKAGE_ROOT}

