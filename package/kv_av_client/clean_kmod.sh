#!/bin/sh

remain_kmod=`lsmod | grep edr_stap | grep -v grep | awk '{print $2}' | wc -l`

if [[ $remain_kmod -eq 0 ]]; then
    echo "Nothing to do with rmmod, skip."
else
    lsmod | grep edr_stap | grep -v grep | awk '{print $2}' | xargs rmmod
fi
