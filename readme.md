
## Introduction
This module is built for demonstration purpose. 

The repository consisted of the code for the kernel module, Makefile rule to compile, and a stand-alone C program to test the string-splitting algorithm.

The kernel module has the following tasks:

- Create a proc file (`/proc/avm_procfile`) for exchanging data between the module and userspace.
- Handle data written to the proc file: 
    - Raw string is stored in a dynamically allocated memory area (`internal_storage`).
    - The words will be extracted from the string and stored in a linked list (`storage_list`).
- Handle reading data from the proc file: 
    - Return the raw string to user. If no user input has been provided, nothing will be returned (WIP).
- Create a timer (timeout in ms `BACKGROUND_SLEEP_INTERVAL`) to print the next word in the linked list `storage_list`. The next word is selected by reading the word at index `word_index_to_read` and increase it after reading. See `my_timer_callback` function.
- Protect internal storage with `semaphore`.

Our observation while developing and testing the software:
- At least 10 times the VM must be force-restarted during development. In most cases, invalid memory access was to be blamed.
- Although its quite complicated to prove in a VM environment on a busy workstation, the timestamps of lock and non-lock tests indicate inaccurate timer on the locked-version. This can most likely be explained by our incorrect implementation, or the busy host machine, or a combination of both. 

    <details>
    <summary>No lock</summary>
        ```properties
        [  +1.124701] Timer callback (1000 ms): Word_th (1) = ### w2 ###
        [  +1.043971] Timer callback (1000 ms): Word_th (2) = ### w3 ###
        [  +1.005721] Timer callback (1000 ms): Word_th (3) = ### w4 ###
        [  +1.654087] Timer callback (1000 ms): Word_th (0) = ### w1 ###
        [  +0.743878] Timer callback (1000 ms): Word_th (1) = ### w2 ###
        [  +0.998623] Timer callback (1000 ms): Word_th (2) = ### w3 ###
        [  +1.019127] Timer callback (1000 ms): Word_th (3) = ### w4 ###
        [  +1.040121] Timer callback (1000 ms): Word_th (0) = ### w1 ###
        [  +1.018234] Timer callback (1000 ms): Word_th (1) = ### w2 ###
        [  +1.002604] Timer callback (1000 ms): Word_th (2) = ### w3 ###
        [  +1.549045] Timer callback (1000 ms): Word_th (3) = ### w4 ###
        [  +0.819934] Timer callback (1000 ms): Word_th (0) = ### w1 ###
        [  +1.153823] Timer callback (1000 ms): Word_th (1) = ### w2 ###
        [  +1.137783] Timer callback (1000 ms): Word_th (2) = ### w3 ###
        [  +1.195407] Timer callback (1000 ms): Word_th (3) = ### w4 ###
        [  +1.017555] Timer callback (1000 ms): Word_th (0) = ### w1 ###
        [  +1.185454] Timer callback (1000 ms): Word_th (1) = ### w2 ###
        [  +1.010378] Timer callback (1000 ms): Word_th (2) = ### w3 ###
        [  +1.034324] Timer callback (1000 ms): Word_th (3) = ### w4 ###
        [  +1.059736] Timer callback (1000 ms): Word_th (0) = ### w1 ###
        [  +1.009410] Timer callback (1000 ms): Word_th (1) = ### w2 ###
        [  +1.149600] Timer callback (1000 ms): Word_th (2) = ### w3 ###
        [  +3.508520] Timer callback (1000 ms): Word_th (3) = ### w4 ###
        [  +1.219596] Timer callback (1000 ms): Word_th (0) = ### w1 ###
        [  +1.089996] Timer callback (1000 ms): Word_th (1) = ### w2 ###
        [  +1.389988] Timer callback (1000 ms): Word_th (2) = ### w3 ###
        [  +1.047358] Timer callback (1000 ms): Word_th (3) = ### w4 ###
        [  +1.037662] Timer callback (1000 ms): Word_th (0) = ### w1 ###
        [  +1.058331] Timer callback (1000 ms): Word_th (1) = ### w2 ###
        [  +1.012996] Timer callback (1000 ms): Word_th (2) = ### w3 ###
        [  +1.089922] Timer callback (1000 ms): Word_th (3) = ### w4 ###
        [  +1.001338] Timer callback (1000 ms): Word_th (0) = ### w1 ###
        [  +1.122789] Timer callback (1000 ms): Word_th (1) = ### w2 ###
        [  +1.052789] Timer callback (1000 ms): Word_th (2) = ### w3 ###
        [  +1.533810] Timer callback (1000 ms): Word_th (3) = ### w4 ###
        [  +1.040383] Timer callback (1000 ms): Word_th (0) = ### w1 ###
        [  +1.229761] Timer callback (1000 ms): Word_th (1) = ### w2 ###
        [  +1.021698] Timer callback (1000 ms): Word_th (2) = ### w3 ###
        ```
    </details>

    <details>
    <summary>With lock</summary>
        ```properties
        [  +1.023174] Timer callback (1000 ms): Word_th (2) = ### w3 ###
        [  +1.023206] Timer callback (1000 ms): Word_th (3) = ### w4 ###
        [  +1.590024] Timer callback (1000 ms): Word_th (4) = ### w5 ###
        [  +2.033190] Timer callback (1000 ms): Word_th (0) = ### w1 ###
        [  +2.021964] Timer callback (1000 ms): Word_th (1) = ### w2 ###
        [  +4.735019] Timer callback (1000 ms): Word_th (2) = ### w3 ###
        [  +1.262133] Timer callback (1000 ms): Word_th (3) = ### w4 ###
        [  +4.898465] Timer callback (1000 ms): Word_th (4) = ### w5 ###
        [  +5.829424] Timer callback (1000 ms): Word_th (0) = ### w1 ###
        [  +2.046367] Timer callback (1000 ms): Word_th (1) = ### w2 ###
        [  +8.914703] Timer callback (1000 ms): Word_th (2) = ### w3 ###
        [ +10.054122] Timer callback (1000 ms): Word_th (3) = ### w4 ###
        [  +3.025667] Timer callback (1000 ms): Word_th (4) = ### w5 ###
        [Mar15 10:18] Timer callback (1000 ms): Word_th (0) = ### w1 ###
        [  +3.844662] Timer callback (1000 ms): Word_th (1) = ### w2 ###
        [ +10.086410] Timer callback (1000 ms): Word_th (2) = ### w3 ###
        [ +10.831835] Timer callback (1000 ms): Word_th (3) = ### w4 ###
        [ +10.956735] Timer callback (1000 ms): Word_th (4) = ### w5 ###
        [ +10.960509] Timer callback (1000 ms): Word_th (0) = ### w1 ###
        [Mar15 10:19] Timer callback (1000 ms): Word_th (1) = ### w2 ###
        [  +1.145155] Timer callback (1000 ms): Word_th (2) = ### w3 ###
        [  +1.082311] Timer callback (1000 ms): Word_th (3) = ### w4 ###
        [  +1.810456] Timer callback (1000 ms): Word_th (4) = ### w5 ###
        [  +1.932116] Timer callback (1000 ms): Word_th (0) = ### w1 ###
        [  +1.032857] Timer callback (1000 ms): Word_th (1) = ### w2 ###
        [ +10.933888] Timer callback (1000 ms): Word_th (2) = ### w3 ###
        [  +8.102293] Timer callback (1000 ms): Word_th (3) = ### w4 ###
        [  +3.057148] Timer callback (1000 ms): Word_th (4) = ### w5 ###
        [  +1.229301] Timer callback (1000 ms): Word_th (0) = ### w1 ###
        [  +1.018627] Timer callback (1000 ms): Word_th (1) = ### w2 ###

        [  +1.508165] Timer callback (1000 ms): Word_th (2) = ### w3 ###


        [  +1.195471] Timer callback (1000 ms): Word_th (3) = ### w4 ###

        [  +1.791043] Timer callback (1000 ms): Word_th (4) = ### w5 ###
        [  +8.987796] Timer callback (1000 ms): Word_th (0) = ### w1 ###
        [  +1.682387] Timer callback (1000 ms): Word_th (1) = ### w2 ###
        [  +5.434465] Timer callback (1000 ms): Word_th (2) = ### w3 ###
        [  +3.844820] Timer callback (1000 ms): Word_th (3) = ### w4 ###
        [Mar15 10:20] Timer callback (1000 ms): Word_th (4) = ### w5 ###
        [ +10.833134] Timer callback (1000 ms): Word_th (0) = ### w1 ###
        [ +10.709669] Timer callback (1000 ms): Word_th (1) = ### w2 ###
        [  +1.234556] Timer callback (1000 ms): Word_th (2) = ### w3 ###
        ```
    </details>


## Environment
We tested and developed the software on a x86 VM using VSCode remote development extension. 

```bash
# VirtualBox macOS Intel Version 7.0.14 r161095 (Qt5.15.2)
lsb_release -a
# Distributor ID: Ubuntu
# Description:    Ubuntu 20.04.6 LTS
# Release:        20.04
# Codename:       focal
uname -r
# 5.8.0-63-generic
dpkg --list | grep linux-image
# ...
# linux-image-unsigned-5.8.0-63-generic 5.8.0-63.71~20.04.1
# Linux kernel image for version 5.8.0 on 64 bit x86 SMP

# gcc (Ubuntu 9.4.0-1ubuntu1~20.04.2) 9.4.0
```

## Prepare

We need Kernel v5.8 for its support for [proc_ops](https://elixir.bootlin.com/linux/v5.8/source/include/linux/proc_fs.h#L29):
```bash
apt list linux-*image-*
sudo apt-get install linux-image-unsigned-5.8.0-63-generic -y
sudo apt-get install linux-headers-5.8.0-63-generic -y
# or
sudo apt-get install linux-image-generic
```

## Run

```bash
# Monitor kernel log in another terminal
sudo dmesg -wH
# or
dmesg --time-format ctime -w
# [Fri Mar 15 12:01:41 2024] Timer callback (1000 ms): Word_th (4) = ### w5 ###
# [Fri Mar 15 12:01:42 2024] Timer callback (1000 ms): Word_th (0) = ### w1 ###
# [Fri Mar 15 12:01:43 2024] Timer callback (1000 ms): Word_th (1) = ### w2 ###

# Insert module
sudo insmod demo_module.ko
# Inspection
lsmod | grep demo_module 
/sbin/modinfo demo_module.ko
# Remove the module
sudo rmmod demo_module.ko
```

Interaction with the module:
```bash
# Write to proc file:
sudo sh -c "echo ' w1 w2 ,  w3 w4 w5' >/proc/avm_proc_file"
# The module will store:
# - the words in a linked list
# - and also will store the raw data in the internal storage,
#   which can be accesed with cat or dd commands:
cat /proc/avm_proc_file
#  w1 w2 ,  w3 w4 w5
#  w1 w2 ,  w3 w4 w5
#  w1 w2 ,  w3 w4 w5
dd bs=20 count=1 status=none < /proc/avm_proc_file
#  w1 w2 ,  w3 w4 w5
```
## Misc
- The split string algorithm can be tested by running `gcc -g napkin_string_split.c -o nss && ./nss`. Check for mem leak: `valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose ./nss`.
- Formatting with Clang format downloaded from kernel source code. Works well with `vscode` on `macOS` using `cmd + K` `cmd + F`.
- Get tickrate: `grep 'CONFIG_HZ=' /boot/config-$(uname -r)`


## Some takes
The macOS host is an obsoleted machine and therefore makes isolating the causes to lags and stutters complicated.



## Disclaimer
Due to the lack of experience, this example is held together by a combination of duct tape, chewing gum, shoe strings and hope. 
The author therefore cannot be held responsible for any damage and will place the blame onto the user of the software.