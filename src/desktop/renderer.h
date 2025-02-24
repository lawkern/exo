#pragma once

/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#if __cplusplus
#   define EXTERN_C extern "C"
#else
#   define EXTERN_C
#endif

#if __ARM_NEON
#   define SIMD_WIDTH 1 // TODO: Implement NEON support.
#else
#   define SIMD_WIDTH 4
#endif

#define CLEAR(name) void name(texture *destination, vec4 color)
#define DRAW_RECTANGLE(name) void name(texture *destination, int posx, int posy, int width, int height, vec4 color)
#define DRAW_TEXTURE_BOUNDED(name) void name(texture *destination, texture *texture, int posx, int posy, int width, int height)
#define DRAW_TEXTURE(name) void name(texture *destination, texture *texture, int posx, int posy)
#define DRAW_OUTLINE(name) void name(texture *destination, int x, int y, int width, int height, vec4 color)

#define DRAW_RECTANGLE_25(name) void name(texture *destination, int x, int y, int width, int height)
#define DRAW_RECTANGLE_50(name) void name(texture *destination, int x, int y, int width, int height)
#define DRAW_RECTANGLE_75(name) void name(texture *destination, int x, int y, int width, int height)

EXTERN_C CLEAR(clear);
EXTERN_C DRAW_RECTANGLE(draw_rectangle);
EXTERN_C DRAW_TEXTURE_BOUNDED(draw_texture_bounded);
EXTERN_C DRAW_TEXTURE(draw_texture);
EXTERN_C DRAW_OUTLINE(draw_outline);

EXTERN_C DRAW_RECTANGLE_25(draw_rectangle_25);
EXTERN_C DRAW_RECTANGLE_50(draw_rectangle_50);
EXTERN_C DRAW_RECTANGLE_75(draw_rectangle_75);
