#!/bin/sh

TOPDIR=$PWD

error() {
    exit 1
}

git clone https://github.com/aloksinha2001/picuntu-3.0.8-alok.git

cp kernel-config picuntu-3.0.8-alok/.config

cd picuntu-3.0.8-alok

make CROSS_COMPILE=arm-linux-gnueabihf- oldconfig || error
make CROSS_COMPILE=arm-linux-gnueabihf- -j4 || error
make CROSS_COMPILE=arm-linux-gnueabihf- INSTALL_MOD_PATH=${TOPDIR}/kernel-out modules_install || error

cd ..

make -C mkbootimg || error

dd if=/dev/zero of=fakeramdisk bs=1M count=1

gzip -9 fakeramdisk

./mkbootimg/mkbootimg --kernel picuntu-3.0.8-alok/arch/arm/boot/Image --ramdisk fakeramdisk.gz --pagesize 16384 --base 0 --board "" --cmdline "" --kernel_offset 0x60408000 --ramdisk_offset 0x62000000 --tags_offset 0x60088000 --second_offset 0x60f00000  -o linuxboot.img

ls -l linuxboot.img
