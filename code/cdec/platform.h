#pragma once

/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#include <cdec.h>

typedef struct {
   u8 *memory;
   size size;
} platform_file;

#define PLATFORM_ALLOCATE(name) void *name(size s)
#define PLATFORM_LOAD_FILE(name) platform_file name(arena *a, char *path)
#define PLATFORM_FREE_FILE(name) void name(platform_file *file)
#define PLATFORM_SAVE_FILE(name) b32 name(void *memory, size s, char *path)
#define PLATFORM_LOG(name) void name(char *format, ...)

PLATFORM_ALLOCATE(platform_allocate);
PLATFORM_LOAD_FILE(platform_load_file);
PLATFORM_FREE_FILE(platform_free_file);
PLATFORM_SAVE_FILE(platform_save_file);
PLATFORM_LOG(platform_log);
