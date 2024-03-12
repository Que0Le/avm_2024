

## Prepare

```bash

apt list linux-*image-*
sudo apt-get install linux-image-unsigned-5.8.0-63-generic -y
sudo apt-get install linux-headers-5.8.0-63-generic -y
# or
sudo apt-get install linux-image-generic


```

## Run

```bash
# Monitor kernel log
sudo dmesg -wH


sudo insmod test.ko

su -c 'echo '12345' >/proc/avm_proc_file'
cat /proc/avm_proc_file

sudo rmmod test.ko
```
## Misc
- Formatting with Clang format downloaded from kernel source code. Works well with `vscode`.