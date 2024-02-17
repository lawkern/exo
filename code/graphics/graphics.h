/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#pragma once

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define function static
#define global static

#define ARRAY_LENGTH(a) (sizeof(a) / sizeof((a)[0]))
#define MAXIMUM(a, b) (((a) > (b)) ? (a) : (b))
#define MINIMUM(a, b) (((a) < (b)) ? (a) : (b))

// TODO(law): Make these configurable.
#define EXO_SCREEN_RESOLUTION_X 960
#define EXO_SCREEN_RESOLUTION_Y 540

#define EXO_WINDOW_MIN_WIDTH  100
#define EXO_WINDOW_MIN_HEIGHT 100
#define EXO_WINDOW_TITLEBAR_DIM 18
#define EXO_WINDOW_EDGE_DIM 5
#define EXO_WINDOW_CORNER_DIM 10
#define EXO_WINDOW_MAX_COUNT 256

struct v4
{
   float r;
   float g;
   float b;
   float a;
};

struct rectangle
{
   s32 x;
   s32 y;
   s32 width;
   s32 height;
};

#pragma pack(push, 1)
struct bitmap_header
{
   // File Header
   u16 file_type;
   u32 file_size;
   u16 reserved1;
   u16 reserved2;
   u32 bitmap_offset;

   // Bitmap Header
   u32 size;
   s32 width;
   s32 height;
   u16 planes;
   u16 bits_per_pixel;
   u32 compression;
   u32 size_of_bitmap;
   s32 horz_resolution;
   s32 vert_resolution;
   u32 colors_used;
   u32 colors_important;
};
#pragma pack(pop)

struct exo_texture
{
   s32 width;
   s32 height;
   u32 *memory;

   s32 offsetx;
   s32 offsety;
};

struct input_state
{
   bool is_pressed;
   bool changed_state;
};

enum mouse_button_type
{
   MOUSE_BUTTON_LEFT,
   MOUSE_BUTTON_MIDDLE,
   MOUSE_BUTTON_RIGHT,
   MOUSE_BUTTON_X1,
   MOUSE_BUTTON_X2,

   MOUSE_BUTTON_COUNT,
};

struct exo_input
{
   s32 mousex;
   s32 mousey;
   s32 previous_mousex;
   s32 previous_mousey;

   struct input_state mouse_buttons[MOUSE_BUTTON_COUNT];
};

struct exo_storage
{
   size_t size;
   u8 *memory;
};

enum cursor_type
{
   CURSOR_ARROW,
   CURSOR_MOVE,
   CURSOR_RESIZE_VERT,
   CURSOR_RESIZE_HORI,
   CURSOR_RESIZE_DIAG_L,
   CURSOR_RESIZE_DIAG_R,

   CURSOR_COUNT,
};

enum window_interaction
{
   WINDOW_INTERACTION_NONE,
   WINDOW_INTERACTION_RAISE,
   WINDOW_INTERACTION_MOVE,
   WINDOW_INTERACTION_RESIZE_N,
   WINDOW_INTERACTION_RESIZE_S,
   WINDOW_INTERACTION_RESIZE_W,
   WINDOW_INTERACTION_RESIZE_E,
   WINDOW_INTERACTION_RESIZE_NW,
   WINDOW_INTERACTION_RESIZE_NE,
   WINDOW_INTERACTION_RESIZE_SW,
   WINDOW_INTERACTION_RESIZE_SE,

   WINDOW_INTERACTION_COUNT,
};

enum window_region_type
{
   // NOTE(law): These are ordered in terms of descending precedence when it
   // comes interactions (in the case that two regions happen to overlap
   // (e.g. oversized corners).
   WINDOW_REGION_CORNER_NW,
   WINDOW_REGION_CORNER_NE,
   WINDOW_REGION_CORNER_SW,
   WINDOW_REGION_CORNER_SE,

   WINDOW_REGION_BORDER_N,
   WINDOW_REGION_BORDER_S,
   WINDOW_REGION_BORDER_W,
   WINDOW_REGION_BORDER_E,

   WINDOW_REGION_CONTENT,
   WINDOW_REGION_TITLEBAR,

   WINDOW_REGION_COUNT,
};

v4 COLOR_CONTENT = {0.196f, 0.188f, 0.184f, 1.0f};
v4 COLOR_TITLE   = {0.392f, 0.376f, 0.368f, 1.0f};
v4 COLOR_BORDER  = {0.0f, 0.0f, 1.0f, 1.0f};
v4 COLOR_CORNER  = {0.0f, 1.0f, 0.0f, 1.0f};

struct window_region_entry
{
   window_interaction interaction;
   cursor_type cursor;
   v4 color;
   v4 debug_color;
};

window_region_entry region_invariants[] =
{
   // IMPORTANT(law): Keep these entries in the same order as the
   // window_region_type enum. Or switch back to C for array designated
   // initializers.
   {WINDOW_INTERACTION_RESIZE_NW, CURSOR_RESIZE_DIAG_L, COLOR_CORNER},
   {WINDOW_INTERACTION_RESIZE_NE, CURSOR_RESIZE_DIAG_R, COLOR_CORNER},
   {WINDOW_INTERACTION_RESIZE_SW, CURSOR_RESIZE_DIAG_R, COLOR_CORNER},
   {WINDOW_INTERACTION_RESIZE_SE, CURSOR_RESIZE_DIAG_L, COLOR_CORNER},

   {WINDOW_INTERACTION_RESIZE_N,  CURSOR_RESIZE_VERT,   COLOR_BORDER},
   {WINDOW_INTERACTION_RESIZE_S,  CURSOR_RESIZE_VERT,   COLOR_BORDER},
   {WINDOW_INTERACTION_RESIZE_W,  CURSOR_RESIZE_HORI,   COLOR_BORDER},
   {WINDOW_INTERACTION_RESIZE_E,  CURSOR_RESIZE_HORI,   COLOR_BORDER},

   {WINDOW_INTERACTION_RAISE,     CURSOR_ARROW,         COLOR_CONTENT},
   {WINDOW_INTERACTION_MOVE,      CURSOR_MOVE,          COLOR_TITLE},
};

union window_region
{
   rectangle bounds;
   struct
   {
      s32 posx;
      s32 posy;
      s32 width;
      s32 height;
   };
};

enum window_state
{
   WINDOW_STATE_CLOSED,
   WINDOW_STATE_NORMAL,
   WINDOW_STATE_MINIMIZED,
   WINDOW_STATE_MAXIMIZED,
};

struct window_sort_entry
{
   u32 index;
   s32 z;
};

struct exo_window
{
   window_state state;

   window_region regions[WINDOW_REGION_COUNT];
   s32 z;

   bool hit_test(struct exo_state *, exo_input *);
   void interact(struct exo_state *state, exo_input *);
};

// TODO(law): Since 0 is a valid index, we're using one outside the valid range
// of the windows array. Maybe reserve index 0 instead?
#define EXO_WINDOW_NULL_INDEX (EXO_WINDOW_MAX_COUNT)

struct exo_state
{
   u32 window_count;
   exo_window windows[EXO_WINDOW_MAX_COUNT];
   window_sort_entry window_order[EXO_WINDOW_MAX_COUNT];

   u32 active_window_index;
   u32 hot_window_index;
   u32 hot_region_index;

   cursor_type frame_cursor;
   exo_texture cursors[CURSOR_COUNT];

   bool is_initialized;
};
