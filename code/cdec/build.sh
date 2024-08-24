#!/usr/bin/env bash

CFLAGS="-g -std=c99 -fdiagnostics-absolute-paths -fno-threadsafe-statics -I ../code/shared/"
CFLAGS="${CFLAGS} -Werror -Wall"
CFLAGS="${CFLAGS} -Wno-missing-braces"
CFLAGS="${CFLAGS} -Wno-writable-strings"
CFLAGS="${CFLAGS} -Wno-char-subscripts"
CFLAGS="${CFLAGS} -Wno-unused-variable"
CFLAGS="${CFLAGS} -Wno-unused-function"

mkdir -p ../../build
pushd ../../build > /dev/null

clang $CFLAGS ../code/cdec/platform_unix.c ../code/cdec/cdec_main.c -DDEVELOPMENT_BUILD=1 -Og -o cdec_debug   $LDFLAGS
clang $CFLAGS ../code/cdec/platform_unix.c ../code/cdec/cdec_main.c -DDEVELOPMENT_BUILD=0 -O2 -o cdec_release $LDFLAGS

popd
