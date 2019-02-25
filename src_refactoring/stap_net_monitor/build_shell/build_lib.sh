#!/bin/sh

g++ ../src/*.cpp ../src/*.c -I../inc/ -I../private_include -I../../common -lexpat -lpthread -lpcap -shared -fPIC -Wall -std=c++98 -fvisibility=hidden -o libnet_monitor.so
