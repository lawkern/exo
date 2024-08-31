#pragma once

/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

// TODO: Implement proper string support for the s8 and s16 types.

// NOTE: This library generally assumes a single translation unit build. In that
// context, marking most functions/globals as static makes sense.
#define function static
#define global static

// NOTE: A few of the most common macros get to be lowercase for the sake of
// better readability.
#define countof(array) (sizeof(array) / sizeof((array)[0]))
#define lengthof(string) (countof(string) - 1)

// NOTE: The less common macros keep the traditional ALL CAPS.
#define MAXIMUM(a, b) (((a) > (b)) ? (a) : (b))
#define MINIMUM(a, b) (((a) < (b)) ? (a) : (b))

// NOTE: Technically these should be KIBI, MEBI, etc. Oh well :)
#define KILOBYTES(v) ((v) * 1024LL)
#define MEGABYTES(v) (KILOBYTES(v) * 1024LL)
#define GIGABYTES(v) (MEGABYTES(v) * 1024LL)
#define TERABYTES(v) (GIGABYTES(v) * 1024LL)

// NOTE: Define debuggable assertions based on the compiler being used.
#define MSVC_ASSERT(expression) do { if(!(expression)) { __debugbreak(); } } while(0)
#define LLVM_ASSERT(expression) do { if(!(expression)) { __builtin_trap(); } } while(0)
#define JANK_ASSERT(expression) do { if(!(expression)) { *(volatile int *)0 = 0; } } while(0)

#if defined(_MSC_VER)
#   define assert MSVC_ASSERT
#elif defined(__GNUC__) || defined(__clang__)
#   define assert LLVM_ASSERT
#else
#   define assert JANK_ASSERT
#endif

// NOTE: Define shorter names for the basic, fixed-size integer types.
#include <stdint.h>
typedef   int8_t i8;
typedef  uint8_t u8;
typedef  int16_t i16;
typedef uint16_t u16;
typedef  int32_t i32;
typedef uint32_t u32;
typedef  int64_t i64;
typedef uint64_t u64;

// NOTE: Define a custom 32-bit bool type
#include <stdbool.h>
typedef int32_t b32;

// NOTE: Define a signed type memory size.
#include <stddef.h>
typedef ptrdiff_t size;

typedef union {
   struct {float x, y;};
   struct {float u, v;};
   struct {float s, t;};
   float elements[2];
} vec2;

typedef union {
   struct {float x, y, z;};
   struct {float r, g, b;};
   struct {vec2 xy; float z_;};
   struct {float _x; vec2 yz;};
   float elements[3];
} vec3;

typedef union {
   struct {float x, y, z, w;};
   struct {float r, g, b, a;};
   struct {vec2 xy, zw;};
   struct {vec3 xyz; float w_;};
   struct {float x_; vec3 yzw;};
   float elements[4];
} vec4;

typedef union {
   vec3 columns[3];
   float elements[9];
} mat3;

typedef union {
   vec4 columns[4];
   float elements[16];
} mat4;

function void zero_memory(void *address, size count)
{
   u8 *bytes = (u8 *)address;
   for(size index = 0; index < count; index++)
   {
	  bytes[index] = 0;
   }
}

// NOTE: A basic arena allocator.
typedef struct {
   u8 *base;
   size cap;
   size used;
} arena;

function void arena_initialize(arena *a, u8 *base, size cap)
{
   a->base = base;
   a->cap = cap;
   a->used = 0;
}

#define arena_allocate(a, type, count) (type *)arena_allocate_((a), sizeof(type), (count))
#define arena_allocate_size(a, size) arena_allocate_((a), (size), 1)

function void *arena_allocate_(arena *a, size unit_size, size count)
{
   void *result = 0;

   size allocation_size = unit_size * count;
   if((a->used + allocation_size) <= a->cap)
   {
      result = a->base + a->used;
      a->used += allocation_size;
   }

   return(result);
}

function void arena_reset(arena *a)
{
   a->used = 0;
}

typedef struct {
   arena *a;
   size used;
} arena_marker;

function arena_marker arena_marker_set(arena *a)
{
   arena_marker result;
   result.a = a;
   result.used = a->used;

   return(result);
}

function void arena_marker_restore(arena_marker marker)
{
   marker.a->used = marker.used;
}

// NOTE: Define custom types for UTF-8 and UTF-16 string.

#if __cplusplus
#define s8(s) s8{(u8 *)s, lengthof(s)}
#define s16(s) s16{u##s, lengthof(u##s)}
#else
#define s8(s) (s8){(u8 *)s, lengthof(s)}
#define s16(s) (s16){u##s, lengthof(u##s)}
#endif

typedef struct {
   u8 *data;
   size length;
} s8;

// #include <uchar.h>
// typedef char16_t c16;

// typedef struct {
//    c16 *data;
//    size length;
// } s16;

function s8 s8new(u8 *data, size length)
{
   s8 result = {data, length};
   return(result);
}

function s8 s8allocate(arena *a, u8 *data, size length)
{
   s8 result;
   result.data = arena_allocate(a, u8, length + 1);
   result.length = length;

   for(size index = 0; index < length; index++)
   {
      result.data[index] = data[index];
   }
   result.data[length] = 0;

   return(result);
}

function b32 s8equals(s8 a, s8 b)
{
   if(a.length != b.length)
   {
      return(false);
   }

   for(size index = 0; index < a.length; index++)
   {
      if(a.data[index] != b.data[index])
      {
         return(false);
      }
   }

   return(true);
}
