#!/bin/bash
gcc -shared -fPIC -Wall sqlite3.c -o libsqlite.so

mv ./libsqlite.so ../../lib -v

echo "Compilation complete"
