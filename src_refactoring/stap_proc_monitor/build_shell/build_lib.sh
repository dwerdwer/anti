#!/bin/sh

g++ ../src/*.c ../src/*.cpp -I../inc/ -I../private_include -I../../common -shared -fPIC -Wall -o libproc_monitor.so -std=c++98 -g -DDEBUG
