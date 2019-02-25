#!/bin/bash

reboot_shell=run_kv_edr_client.sh
install_path=/usr/local/kv_edr_client
kill_shell_name=$install_path/script/kill_kv_edr_client.sh
clean_kernel_mod_script=./clean_kmod.sh


if [ -f $kill_shell_name ];then
    $kill_shell_name
fi

if [ -d $install_path ];then
    rm $install_path /etc/init.d/$reboot_shell -rf
fi

if [ -f "/etc/rc.d/rc.local" ];then
    sed -i /$reboot_shell/d /etc/rc.d/rc.local
fi

if [ -f "/etc/rc.local" ];then
    sed -i /$reboot_shell/d /etc/rc.local
fi

# clean kernel mod
sh $clean_kernel_mod_script

echo "uninstall finish"


