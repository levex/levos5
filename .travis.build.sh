#!/bin/sh

echo "Building LEVOS 5 on TRAVIS"

make CC=gcc CFLAGS=" -D_ARCH__x86__ -ffreestanding -nostdlib -lgcc -Iinclude -std=gnu99" clean all
