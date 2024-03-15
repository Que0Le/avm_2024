

## Prepare

Kernel v5.8 supports [proc_ops](https://elixir.bootlin.com/linux/v5.8/source/include/linux/proc_fs.h#L29):
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


sudo insmod demo_module.ko
lsmod | grep demo_module 
/sbin/modinfo demo_module.ko

sudo sh -c "echo ' w1 w2 ,  w3 w4 w5' >/proc/avm_proc_file"
cat /proc/avm_proc_file
dd bs=20 count=1 < /proc/avm_proc_file

sudo rmmod demo_module.ko
```
## Misc
- At least 10 times VM must be force-restarted during development.
- The split string algorithm can be tested by running `gcc -g napkin_string_split.c -o nss && ./nss`. Check for mem leak: `valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose ./nss`.
- Formatting with Clang format downloaded from kernel source code. Works well with `vscode` on `macOS` using `cmd + K` `cmd + F`.
- Get tickrate: `grep 'CONFIG_HZ=' /boot/config-$(uname -r)`