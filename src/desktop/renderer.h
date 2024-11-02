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
#define DRAW_RECTANGLE(name) void name(texture *backbuffer, s32 posx, s32 posy, s32 width, s32 height, vec4 color)
#define DRAW_TEXTURE_BOUNDED(name) void name(texture *destination, texture *texture, s32 posx, s32 posy, s32 width, s32 height)
#define DRAW_TEXTURE(name) void name(texture *destination, texture *texture, s32 posx, s32 posy)
#define DRAW_OUTLINE(name) void name(texture *destination, s32 x, s32 y, s32 width, s32 height, vec4 color)

EXTERN_C CLEAR(clear);
EXTERN_C DRAW_RECTANGLE(draw_rectangle);
EXTERN_C DRAW_TEXTURE_BOUNDED(draw_texture_bounded);
EXTERN_C DRAW_TEXTURE(draw_texture);
EXTERN_C DRAW_OUTLINE(draw_outline);
