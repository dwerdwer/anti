#!/bin/bash
gcc sdk_file_monitor_demo.cpp -I../inc -L../../../lib/ -Wl,-rpath,../../../lib/ -lfile_monitor_sdk -lpthread -o run_demo -Wl,--dynamic-linker,../../../lib/ld-linux-x86-64.so.2 
