#!/bin/bash

daemon_upgrader="upgrade_daemon.sh"
viruslib_upgrader="upgrade_viruslib.sh"

bash "./${daemon_upgrader}" "$1" "$2"
daemon_res=$?
bash "./${viruslib_upgrader}" "$3"
viruslib_res=$?

echo "upgrade_daemon -> ${daemon_res}"
echo "upgrade_viruslib -> ${viruslib_res}"