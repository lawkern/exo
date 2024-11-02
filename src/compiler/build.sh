#!/usr/bin/env bash

SRCPATH=$(pwd)

COMPILER_FLAGS="-g -std=c99 -fdiagnostics-absolute-paths -fno-threadsafe-statics -I ../src/shared/"
COMPILER_FLAGS="${COMPILER_FLAGS} -Werror -Wall"
COMPILER_FLAGS="${COMPILER_FLAGS} -Wno-missing-braces"
COMPILER_FLAGS="${COMPILER_FLAGS} -Wno-writable-strings"
COMPILER_FLAGS="${COMPILER_FLAGS} -Wno-char-subscripts"
COMPILER_FLAGS="${COMPILER_FLAGS} -Wno-unused-variable"
COMPILER_FLAGS="${COMPILER_FLAGS} -Wno-unused-function"

mkdir -p ../../build
pushd ../../build > /dev/null

clang $COMPILER_FLAGS ../src/shared/platform_unix.c $SRCPATH/compiler_main.c -DDEVELOPMENT_BUILD=1 -Og -o compiler_x64_debug   $LDFLAGS
clang $COMPILER_FLAGS ../src/shared/platform_unix.c $SRCPATH/compiler_main.c -DDEVELOPMENT_BUILD=0 -O2 -o compiler_x64_release $LDFLAGS

popd
