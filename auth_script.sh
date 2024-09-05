#!/bin/bash

# Disable history expansion and special character interpretation
set +H   # Disable history expansion (!)
set -f   # Disable pathname expansion (* and ?)

# Get username and password arguments, ensuring they are fully quoted
USERNAME="$1"
PASSWORD="$2"

# Authenticate the user by piping the password into sudo
echo "$PASSWORD" | sudo -S -u "$USERNAME" whoami > /dev/null 2>&1

# Check the result of the authentication
if [ $? -eq 0 ]; then
    exit 0  # Authentication successful
else
    exit 1  # Authentication failed
fi
