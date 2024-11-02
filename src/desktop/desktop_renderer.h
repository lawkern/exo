#pragma once

/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#if __cplusplus
#   define EXTERN_C extern "C"
#else
#   define EXTERN_C
#endif

#define CLEAR(name) void name(texture *destination, vec4 color)
#define DRAW_RECTANGLE(name) void name(texture *backbuffer, i32 posx, i32 posy, i32 width, i32 height, vec4 color)
#define DRAW_TEXTURE_BOUNDED(name) void name(texture *destination, texture *texture, i32 posx, i32 posy, i32 width, i32 height)
#define DRAW_TEXTURE(name) void name(texture *destination, texture *texture, i32 posx, i32 posy)
#define DRAW_OUTLINE(name) void name(texture *destination, i32 x, i32 y, i32 width, i32 height, vec4 color)

EXTERN_C CLEAR(clear);
EXTERN_C DRAW_RECTANGLE(draw_rectangle);
EXTERN_C DRAW_TEXTURE_BOUNDED(draw_texture_bounded);
EXTERN_C DRAW_TEXTURE(draw_texture);
EXTERN_C DRAW_OUTLINE(draw_outline);
