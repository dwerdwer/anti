#!/bin/bash
g++ -shared -fPIC  -Wall -lcurl NetUtils.cpp -o libNetUtils.so

cp ./libNetUtils.so ../../lib -v


