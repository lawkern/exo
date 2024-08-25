#!/usr/bin/env bash

CODEPATH=$(pwd)

mkdir -p ../../build
pushd ../../build > /dev/null

echo Assembling boot code...
nasm $CODEPATH/boot.asm -felf32 -o boot.o

echo Compiling kernel...
COMPILER_FLAGS="-g -ffreestanding -Wall -Wextra -Wno-unused-function -I ../code/shared/"

i686-elf-gcc $CODEPATH/kernel_main.c -c -Og -DDEVELOPMENT_BUILD=1 $COMPILER_FLAGS -o kernel_debug.o
i686-elf-gcc $CODEPATH/kernel_main.c -c -O2 -DDEVELOPMENT_BUILD=0 $COMPILER_FLAGS -o kernel_release.o

i686-elf-gcc -T$CODEPATH/kernel_linker.ld -o exo_debug.bin   $COMPILER_FLAGS -nostdlib -lgcc boot.o kernel_debug.o
i686-elf-gcc -T$CODEPATH/kernel_linker.ld -o exo_release.bin $COMPILER_FLAGS -nostdlib -lgcc boot.o kernel_release.o

echo Done

popd > /dev/null
