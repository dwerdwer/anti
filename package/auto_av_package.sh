#!/bin/bash
echo " "
echo "Start pack..."

# file pat
content_path=./kv_av_client

tmp_path="./bin ./etc ./lib ./log ./script"

target="$content_path""/kv_av_client.tar.gz"

if [ ! -d "$content_path" ] 
then
    echo "$content_path" "does not exist"
    echo "end"
exit 1
fi

if [ ! -d "./bin" ] 
then
    mkdir $tmp_path
fi

cp ../build/main_daemon ./bin/kv_av_daemon

cp -rf ../etc/customize_edr.xml $content_path 

cp -rf ../etc/vc_daemon_config.xml ./etc 

cp -rf ../etc/customize_edr.xml ./etc 

cp -rf ../etc/*.xml ./etc/

cp ../lib/*.so* ./lib

cp ../lib/*.ko ./lib

cp $content_path/*.sh ./script

rm -rf ./lib/*av*

echo "Wait a minute..."
tar zcf "$target" $tmp_path  

rm -rf  $tmp_path

tar zcf "${content_path}_x64$(date +_%Y%m%d).tar.gz" "${content_path}"

echo "Finish"
echo " "
