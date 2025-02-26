#pragma once

/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */

#include "shared.h"

#include <stdio.h>
#include <stdlib.h>

// TODO(law): Make these configurable.
#define DESKTOP_TASKBAR_HEIGHT 20

#define DESKTOP_WINDOW_MAX_COUNT 256
#define DESKTOP_WINDOW_MIN_WIDTH  120
#define DESKTOP_WINDOW_MIN_HEIGHT 100

#define DESKTOP_WINDOWTAB_WIDTH_MAX 120

#define DESKTOP_WINDOW_DIM_BUTTON 24
#define DESKTOP_WINDOW_DIM_EDGE 6
#define DESKTOP_WINDOW_DIM_CORNER 16
#define DESKTOP_WINDOW_DIM_TITLEBAR 21

#define DESKTOP_WINDOW_HALFDIM_BUTTON (DESKTOP_WINDOW_DIM_BUTTON / 2)
#define DESKTOP_WINDOW_HALFDIM_EDGE (DESKTOP_WINDOW_DIM_EDGE / 2)
#define DESKTOP_WINDOW_HALFDIM_TITLEBAR (DESKTOP_WINDOW_DIM_TITLEBAR / 2)

typedef struct {
   s32 x;
   s32 y;
   s32 width;
   s32 height;
} rectangle;

#pragma pack(push, 1)
typedef struct {
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
} bitmap_header;
#pragma pack(pop)

typedef struct texture {
   s32 width;
   s32 height;
   u32 *memory;

   s32 offsetx;
   s32 offsety;
} texture;

typedef struct {
   bool is_pressed;
   bool changed_state;
} input_state;

typedef enum {
   INPUT_KEY_TAB,
   INPUT_KEY_MBLEFT,
   INPUT_KEY_MBMIDDLE,
   INPUT_KEY_MBRIGHT,
   INPUT_KEY_MBX1,
   INPUT_KEY_MBX2,

   INPUT_KEY_COUNT,
} input_key_type;

typedef struct {
   s32 mousex;
   s32 mousey;

   s32 previous_mousex;
   s32 previous_mousey;

   input_state keys[INPUT_KEY_COUNT];

   u32 frame_count;
   u32 sleep_ms;
   float frame_seconds_elapsed;
   float target_seconds_per_frame;
} desktop_input;

typedef enum {
   CURSOR_ARROW,
   CURSOR_MOVE,
   CURSOR_RESIZE_VERT,
   CURSOR_RESIZE_HORI,
   CURSOR_RESIZE_DIAG_L,
   CURSOR_RESIZE_DIAG_R,

   CURSOR_COUNT,
} cursor_type;

typedef enum {
   WINDOW_INTERACTION_NONE,
   WINDOW_INTERACTION_RAISE,
   WINDOW_INTERACTION_MOVE,
   WINDOW_INTERACTION_CLOSE,
   WINDOW_INTERACTION_MAXIMIZE,
   WINDOW_INTERACTION_MINIMIZE,
   WINDOW_INTERACTION_RESIZE_N,
   WINDOW_INTERACTION_RESIZE_S,
   WINDOW_INTERACTION_RESIZE_W,
   WINDOW_INTERACTION_RESIZE_E,
   WINDOW_INTERACTION_RESIZE_NW,
   WINDOW_INTERACTION_RESIZE_NE,
   WINDOW_INTERACTION_RESIZE_SW,
   WINDOW_INTERACTION_RESIZE_SE,

   WINDOW_INTERACTION_COUNT,
} window_interaction_type;

typedef enum {
   // NOTE(law): These are ordered in terms of descending precedence when it
   // comes interactions (in the case that two regions happen to overlap
   // (e.g. oversized corners).
   WINDOW_REGION_BUTTON_CLOSE,
   WINDOW_REGION_BUTTON_MAXIMIZE,
   WINDOW_REGION_BUTTON_MINIMIZE,
   WINDOW_REGION_TITLEBAR,
   WINDOW_REGION_CONTENT,

   WINDOW_REGION_CORNER_NW,
   WINDOW_REGION_CORNER_NE,
   WINDOW_REGION_CORNER_SW,
   WINDOW_REGION_CORNER_SE,

   WINDOW_REGION_BORDER_N,
   WINDOW_REGION_BORDER_S,
   WINDOW_REGION_BORDER_W,
   WINDOW_REGION_BORDER_E,

   WINDOW_REGION_COUNT,
} window_region_type;

typedef struct desktop_window desktop_window;

#define DRAW_REGION(name) void name(texture *destination, desktop_window *window, bool is_active_window)
typedef DRAW_REGION(draw_region);

typedef struct {
   window_interaction_type interaction;
   cursor_type cursor;
   draw_region *draw;
} window_region_entry;

global vec4 DEBUG_COLOR_WHITE = {1.0f, 1.0f, 1.0f, 1.0f};
global vec4 DEBUG_COLOR_BLACK = {0.0f, 0.0f, 0.0f, 1.0f};
global vec4 DEBUG_COLOR_GREEN = {0.0f, 1.0f, 0.0f, 1.0f};
global vec4 DEBUG_COLOR_BLUE = {0.0f, 0.0f, 1.0f, 1.0f};

global vec4 PALETTE[] =
{
   {1.0f, 1.0f, 1.0f, 1.0f},
   {0.831f, 0.816f, 0.784f, 1.0f},
   {0.50f, 0.50f, 0.50f, 1.0f},
   {0.25f, 0.25f, 0.25f, 1.0f},
   {0.0f, 0.0f, 0.0f, 1.0f},
};

typedef enum {
   WINDOW_STATE_CLOSED,
   WINDOW_STATE_NORMAL,
   WINDOW_STATE_MINIMIZED,
   WINDOW_STATE_MAXIMIZED,
} window_state;

typedef struct {
   u32 region_index;
} hit_result;

struct desktop_window
{
   string8 title;
   window_state state;
   s32 z;

   union
   {
      rectangle bounds;
      struct
      {
         int x;
         int y;
         int width;
         int height;
      };
   };

   texture canvas;
   rectangle unmaximized;
   bool display_infobar;

   desktop_window *prev;
   desktop_window *next;
};

typedef struct {
   bool focus_follows_mouse;
   bool dark_mode;
} desktop_configuration;

// TODO(law): Since 0 is a valid index, we're using one outside the valid range
// of the array. Maybe reserve index 0 instead?
#define DESKTOP_WINDOW_NULL_INDEX (DESKTOP_WINDOW_MAX_COUNT)
#define DESKTOP_REGION_NULL_INDEX (WINDOW_REGION_COUNT)

typedef struct {
   texture backbuffer;
   desktop_input input;

   arena window_arena;
   arena texture_arena;
   arena scratch_arena;

   // NOTE: The first_window field refers to the top-level window in sorting
   // order. Therefore first_window undergoes hit detection first and rendering
   // last. The opposite is true for last_window.
   desktop_window *first_window;
   desktop_window *last_window;
   desktop_window *free_window;

   desktop_configuration config;

   desktop_window *active_window; // Undgoing action
   desktop_window *hot_window;    // Ready for action

   int active_window_mouse_offsetx;
   int active_window_mouse_offsety;

   cursor_type frame_cursor;
   texture cursor_textures[CURSOR_COUNT];
   texture region_textures[WINDOW_REGION_COUNT];

   bool is_initialized;
} desktop_context;

#define DESKTOP_INITIALIZE(name) void name(desktop_context *desktop, int width, int height)
DESKTOP_INITIALIZE(desktop_initialize);

#define DESKTOP_UPDATE(name) void name(desktop_context *desktop)
DESKTOP_UPDATE(desktop_update);
