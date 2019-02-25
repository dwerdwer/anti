#!/bin/sh

g++ ../demo/main.cpp -lpthread -lproc_monitor -L. -Wl,-rpath,. -I../inc -Wall -o proc_monitor_demo -std=c++98 -g -DDEBUG
