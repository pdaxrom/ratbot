#!/bin/sh

cd picuntu-3.0.8-alok

make CROSS_COMPILE=arm-linux-gnueabihf- distclean || error

cd ..

make -C mkbootimg clean

rm -f fakeramdisk.gz linuxboot.img
rm -rf kernel-out
