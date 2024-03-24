

#!/bin/bash

if [ $# -eq 0 ]; then
    echo "Usage: $0 [option]"
    exit 1
fi

case "$1" in
    i)
        echo "Inserting ..."
        sudo insmod demo_module.ko
        ;;
    d)
        echo "Removing module ..."
        sudo rmmod demo_module
        ;;
    r)
        echo "Read from proc file ..."
        dd bs=128 count=1 status=none < /proc/avm_proc_file
        ;;
    w)
        echo "Write to proc file ..."
        sudo sh -c "echo ' w1 w2 ,  w3 w4 w5 w123456789' >/proc/avm_proc_file"
        ;;
    m)
        echo "Monitor kernel log with dmesg ..."
        dmesg --time-format ctime -w
        ;;
    nss)
        echo "Compile and run napkin_string_split ..."
        gcc -g napkin_string_split.c -o nss && ./nss
        ;;
    *)
        echo "Invalid option: $1"
        echo "Usage: $0 [i d r w m nss]"
        exit 1
        ;;
esac

exit 0