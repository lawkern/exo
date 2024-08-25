#!/usr/bin/env bash

CODEPATH=$(pwd)

COMPILER_FLAGS="-g -std=c99 -fdiagnostics-absolute-paths -fno-threadsafe-statics -I ../code/shared/"
COMPILER_FLAGS="${COMPILER_FLAGS} -Werror -Wall"
COMPILER_FLAGS="${COMPILER_FLAGS} -Wno-missing-braces"
COMPILER_FLAGS="${COMPILER_FLAGS} -Wno-writable-strings"
COMPILER_FLAGS="${COMPILER_FLAGS} -Wno-char-subscripts"
COMPILER_FLAGS="${COMPILER_FLAGS} -Wno-unused-variable"
COMPILER_FLAGS="${COMPILER_FLAGS} -Wno-unused-function"

mkdir -p ../../build
pushd ../../build > /dev/null

clang $COMPILER_FLAGS $CODEPATH/platform_unix.c $CODEPATH/cdec_main.c -DDEVELOPMENT_BUILD=1 -Og -o cdec_debug   $LDFLAGS
clang $COMPILER_FLAGS $CODEPATH/platform_unix.c $CODEPATH/cdec_main.c -DDEVELOPMENT_BUILD=0 -O2 -o cdec_release $LDFLAGS

popd
