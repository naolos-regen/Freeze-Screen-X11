#!/bin/bash


USERNAME="$1"
PASSWORD="$2"

echo "$PASSWORD" | sudo -S -u "$USERNAME" whoami > /dev/null 2>&1

if [ $? -eq 0 ]; then
	exit 0
else
	exit 1
fi


