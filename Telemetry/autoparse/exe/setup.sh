#!/bin/bash

cd `dirname $0`

if [[ "$1" == "--undo" ]]; then 
format=unix2dos; op=-x;
else
format=dos2unix; op=+x;
fi

$format ./console
$format ./influx_server
$format ./parse_folder
$format ./upload
$format ../app/console.py
$format ../app/database_client.py
$format ../app/mqtt_server.py

chmod $op ./console
chmod $op ./influx_server
chmod $op ./parse_folder
chmod $op ./upload

chmod $op ../app/console.py
chmod $op ../app/database_client.py
chmod $op ../app/mqtt_server.py