#!/bin/bash

# cd src/
#source /mnt/HDD/Project/qt-kobo/koxtoolchain/refs/x-compile.sh kobo env
# /mnt/HDD/Project/qt-kobo/x-tools/arm-kobo-linux-gnueabihf/bin/arm-kobo-linux-gnueabihf-g++ -o ../inkbox-power-deamon main.cpp

export PATH=$PATH:/home/build/inkbox/kernel/toolchain/armv7l-linux-musleabihf-cross/bin
rm -rf build
mkdir build
cd build
# https://clangd.llvm.org/installation.html#project-setup
cmake ../ -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_TOOLCHAIN_FILE=../kobo.cmake
cmake --build .
cd ..

servername="root@10.42.0.28"
passwd="root"

sshpass -p $passwd ssh $servername "bash -c \"ifsctl mnt rootfs rw\""
sshpass -p $passwd ssh $servername "bash -c \"rm /inkaudio\""

sshpass -p $passwd ssh $servername "bash -c \"killall -9 inkaudio\""
sshpass -p $passwd ssh $servername "bash -c \"rm /dev/iaudio.socket\""

sshpass -p $passwd scp build/inkaudio $servername:/

sshpass -p $passwd ssh $servername "bash -c \"sync\""

# Normal launch
sshpass -p $passwd ssh $servername "bash -c \"/usr/bin/env DEBUG=true /inkaudio\""
