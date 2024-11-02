#pragma once

/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#include <cdec.h>
#include <stdio.h>
#include <stdlib.h>

// TODO(law): Make these configurable.
#define DESKTOP_SCREEN_RESOLUTION_X (1280 << 0)
#define DESKTOP_SCREEN_RESOLUTION_Y (720  << 0)
#define DESKTOP_TASKBAR_HEIGHT 32

#define DESKTOP_WINDOW_MAX_COUNT 256
#define DESKTOP_WINDOW_MIN_WIDTH  120
#define DESKTOP_WINDOW_MIN_HEIGHT 100

#define DESKTOP_WINDOWTAB_WIDTH_MAX 120

#define DESKTOP_WINDOW_DIM_BUTTON 24
#define DESKTOP_WINDOW_DIM_EDGE 6
#define DESKTOP_WINDOW_DIM_CORNER 16
#define DESKTOP_WINDOW_DIM_TITLEBAR (DESKTOP_WINDOW_DIM_BUTTON + DESKTOP_WINDOW_DIM_EDGE)

#define DESKTOP_WINDOW_HALFDIM_BUTTON (DESKTOP_WINDOW_DIM_BUTTON / 2)
#define DESKTOP_WINDOW_HALFDIM_EDGE (DESKTOP_WINDOW_DIM_EDGE / 2)
#define DESKTOP_WINDOW_HALFDIM_TITLEBAR (DESKTOP_WINDOW_DIM_TITLEBAR / 2)

typedef struct {
   i32 x;
   i32 y;
   i32 width;
   i32 height;
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
   i32 width;
   i32 height;
   u16 planes;
   u16 bits_per_pixel;
   u32 compression;
   u32 size_of_bitmap;
   i32 horz_resolution;
   i32 vert_resolution;
   u32 colors_used;
   u32 colors_important;
} bitmap_header;
#pragma pack(pop)

typedef struct {
   i32 width;
   i32 height;
   u32 *memory;

   i32 offsetx;
   i32 offsety;
} texture;

typedef struct {
   bool is_pressed;
   bool changed_state;
} input_state;

typedef enum {
   MOUSE_BUTTON_LEFT,
   MOUSE_BUTTON_MIDDLE,
   MOUSE_BUTTON_RIGHT,
   MOUSE_BUTTON_X1,
   MOUSE_BUTTON_X2,

   MOUSE_BUTTON_COUNT,
} mouse_button_type;

typedef struct {
   i32 mousex;
   i32 mousey;

   i32 previous_mousex;
   i32 previous_mousey;

   input_state mouse_buttons[MOUSE_BUTTON_COUNT];

   u32 frame_count;
   float frame_seconds_elapsed;
   float target_seconds_per_frame;
} desktop_input;

typedef struct {
   size_t size;
   u8 *memory;
} desktop_storage;

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

struct desktop_window {
   s8 title;
   window_state state;
   i32 z;

   texture texture;
   union
   {
      rectangle content;
      struct
      {
         i32 x;
         i32 y;
         i32 width;
         i32 height;
      };
   };
   rectangle unmaximized;

   desktop_window *prev;
   desktop_window *next;
};

typedef struct {
   bool focus_follows_mouse;
} desktop_configuration;

// TODO(law): Since 0 is a valid index, we're using one outside the valid range
// of the array. Maybe reserve index 0 instead?
#define DESKTOP_WINDOW_NULL_INDEX (DESKTOP_WINDOW_MAX_COUNT)
#define DESKTOP_REGION_NULL_INDEX (WINDOW_REGION_COUNT)

typedef struct {
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

   desktop_window *mouse_window;
   desktop_window *active_window;

   desktop_window *hot_window;
   u32 hot_region_index;

   cursor_type frame_cursor;
   texture cursor_textures[CURSOR_COUNT];
   texture region_textures[WINDOW_REGION_COUNT];

   bool is_initialized;
} desktop_state;
