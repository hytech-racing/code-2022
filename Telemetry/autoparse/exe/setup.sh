#!/bin/bash

if [[ "$1" == "--undo" ]]; then op=-x; else op=+x; fi

chmod $op ./console
chmod $op ./influx_server
chmod $op ./parse_folder
chmod $op ./upload

chmod $op ../app/console.py
chmod $op ../app/database_client.py
chmod $op ../app/mqtt_server.py