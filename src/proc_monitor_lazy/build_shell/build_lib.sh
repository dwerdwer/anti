#!/bin/sh

g++ ../src/*.cpp -I../inc/ -I../private_include -lpthread -shared -fPIC -Wall -o libproc_monitor.so -std=c++98
