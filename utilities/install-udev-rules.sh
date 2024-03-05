#!/usr/bin/env bash

GROUP_NAME=plugdev

# Check if the script is running on a Linux system
if [ "$(uname)" != "Linux" ]; then
    echo "Not running on a Linux system."
    exit 1
fi

# Check if the user is not in the group
if ! groups "$USER" | grep -qw "$GROUP_NAME"; then
    # Add user to the group
    sudo usermod -aG "$GROUP_NAME" "$USER"
    echo "User $USER added to group $GROUP_NAME."
    echo "Restart your system to see the effect"
else
    echo "User $USER is already in group $GROUP_NAME."
fi

# copy without overwrite
sudo cp -n utilities/udev-rules/* /etc/udev/rules.d/