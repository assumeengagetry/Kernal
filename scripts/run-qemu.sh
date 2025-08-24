#!/bin/bash
# QEMU helper script for running the kernel

QEMU_OPTS="-cpu qemu64 \
           -m 256M \
           -nographic \
           -no-reboot \
           -serial mon:stdio"

DEBUG_OPTS="-s -S"

case "$1" in
    "debug")
        qemu-system-x86_64 $QEMU_OPTS $DEBUG_OPTS -kernel out/kernel.elf
        ;;
    "iso")
        qemu-system-x86_64 $QEMU_OPTS -cdrom out/os.iso
        ;;
    *)
        qemu-system-x86_64 $QEMU_OPTS -kernel out/kernel.elf
        ;;
esac
