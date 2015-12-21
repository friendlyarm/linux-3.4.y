#!/bin/sh
make ARCH=arm clean
make ARCH=arm -j4
cp nx_vpu.ko ../../../../../hardware/samsung_slsi/slsiap/prebuilt/modules/
