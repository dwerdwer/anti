#!/bin/sh

g++ ../demo/main.cpp -lpcap -lnet_monitor -I../inc -Wall -L. -Wl,-rpath,. -std=c++98 -o net_monitor_demo
