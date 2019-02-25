#!/bin/bash
echo " "
echo "start install..."

install_path=/usr/local/vc_linux_client
target_config=/usr/local/vc_linux_client/etc/linux_client.xml

packet_path=./vc_linux_client.tar.gz
run_shell_name=run_vc_client.sh

if [ ! -d $install_path ] 
then
    mkdir $install_path
fi

if [ ! -d $install_path/log ] 
then
    mkdir $install_path/log
fi

if [ ! -f $packet_path ] 
then
    echo "install fail missing $packet_path"
    exit 1
else
    tar -zxf $packet_path -C $install_path
fi

config_file=./linux_client.xml

if [ ! -f $config_file ] 
then
    echo "install error missing $config_file"
    exit 1
fi

mv $config_file $target_config 

rm -rf $packet_path

chmod +x *.sh 

get_system_name()
{
    if grep -Eqi "CentOS" /etc/issue || grep -Eq "CentOS" /etc/*-release; then
        name='CentOS'
    elif grep -Eqi "Red Hat Enterprise Linux Server" /etc/issue || grep -Eq "Red Hat Enterprise Linux Server" /etc/*-release; then
        name='rhel server'
    elif grep -Eqi "Fedora" /etc/issue || grep -Eq "Fedora" /etc/*-release; then
        name='Fedora'
    elif grep -Eqi "Debian" /etc/issue || grep -Eq "Debian" /etc/*-release; then
        name='Debian'
    elif grep -Eqi "Ubuntu" /etc/issue || grep -Eq "Ubuntu" /etc/*-release; then
        name='Ubuntu'
    elif grep -Eqi "SUSE" /etc/issue || grep -Eq "SUSE" /etc/*-release; then
        name='SUSE'
    elif grep -Eqi "Mandriva" /etc/issue || grep -Eq "Mandriva" /etc/*-release; then
        name='Mandriva'
    else
        name='unknow'
    fi
    echo $name
}
system_name=`get_system_name`

if [ $(getconf WORD_BIT) = '32' ] && [ $(getconf LONG_BIT) = '64' ]
then
    bit_str="(64-bit)"
else
    bit_str="(32-bit)"
    echo "This program cannot run on the 32 bit"
    exit 1
fi

system_version="<param_9>"${system_name}${bit_str}"</param_9> <!--system version-->"

sed -i "53c \            ${system_version}" $target_config

# move engine libs to /usr/lib
sudo mv  $install_path/lib/engine_lib/* /usr/lib

# PowerBoot

boot_run=/etc/rc.d/rc.local
cp $run_shell_name /etc/init.d/ 

if [ "$system_name" = "Ubuntu" ];then
    boot_run=/etc/rc.local
fi
# Check if the "chkconfig" exists
if hash chkconfig 2> /dev/null; then
    chkconfig --add $run_shell_name
else
    if [ -f "$boot_run" ];then
        chmod +x $boot_run

        if grep -Eqi $install_path/script/$run_shell_name $boot_run; then
            echo " "
        else
            echo $install_path/script/$run_shell_name >> $boot_run
        fi
    fi
fi


echo "install finish"

# run now
./$run_shell_name

