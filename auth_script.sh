#!/bin/bash

# Disable history expansion and special character interpretation
set +H   # Disable history expansion (!)
set -f   # Disable pathname expansion (* and ?)

# Get username and password arguments
USERNAME="$1"
PASSWORD="$2"

# Set max length for username and password
MAX_USERNAME_LENGTH=32
MAX_PASSWORD_LENGTH=64

# Check if username and password exceed the allowed length
if [ ${#USERNAME} -gt $MAX_USERNAME_LENGTH ] || [ ${#PASSWORD} -gt $MAX_PASSWORD_LENGTH ]; then
    exit 1
fi

# Validate username for special characters
if [[ "$USERNAME" =~ [^a-zA-Z0-9._-] ]]; then
    exit 1
fi

# Optionally, validate password (example pattern)
# Uncomment and adjust if needed
# if [[ "$PASSWORD" =~ [^a-zA-Z0-9@#%^&*_+=] ]]; then
#     exit 1
# fi

# Escape username to prevent issues with special characters
ESCAPED_USERNAME=$(printf '%q' "$USERNAME")

# Authenticate the user by piping the password into sudo
echo "$PASSWORD" | sudo -S -u "$ESCAPED_USERNAME" whoami > /dev/null 2>&1

# Check the result of the authentication
if [ $? -eq 0 ]; then
    exit 0  # Authentication successful
else
    exit 1  # Authentication failed
fi
