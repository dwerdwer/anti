#!/bin/bash

reboot_shell=run_vc_client.sh
install_path=/usr/local/vc_linux_client
kill_shell_name=$install_path/script/kill_vc_client.sh

if [ -f $kill_shell_name ]
then
    $kill_shell_name
fi

if [ -d $install_path ]
then
    rm $install_path /etc/init.d/$reboot_shell -rf
fi

if [ -f "/etc/rc.d/rc.local" ]
then
    sed -i /$reboot_shell/d /etc/rc.d/rc.local
fi

if [ -f "/etc/rc.local" ]
then
    sed -i /$reboot_shell/d /etc/rc.local
fi

echo "uninstall finish"


