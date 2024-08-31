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

function vec4 operator+(vec4 vector, float value)
{
   vector.r += value;
   vector.g += value;
   vector.b += value;
   vector.a += value;

   return(vector);
}

function vec4 operator*(vec4 vector, float value)
{
   vector.r *= value;
   vector.g *= value;
   vector.b *= value;
   vector.a *= value;

   return(vector);
}

function vec4 operator*(float value, vec4 vector)
{
   vec4 result = vector * value;
   return(result);
}

function vec4 operator*=(vec4 &vector, float value)
{
   vector = vector * value;
   return(vector);
}

struct rectangle
{
   i32 x;
   i32 y;
   i32 width;
   i32 height;
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
};
#pragma pack(pop)

struct texture
{
   i32 width;
   i32 height;
   u32 *memory;

   i32 offsetx;
   i32 offsety;
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

struct desktop_input
{
   i32 mousex;
   i32 mousey;

   i32 previous_mousex;
   i32 previous_mousey;

   struct input_state mouse_buttons[MOUSE_BUTTON_COUNT];

   u32 frame_count;
   float frame_seconds_elapsed;
   float target_seconds_per_frame;
};

struct desktop_storage
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

enum window_interaction_type
{
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
};

enum window_region_type
{
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
};

#define DRAW_REGION(name) void name(texture *destination, struct desktop_window *window, bool is_active_window)
typedef DRAW_REGION(draw_region);

struct window_region_entry
{
   window_interaction_type interaction;
   cursor_type cursor;
   draw_region *draw;
};

function DRAW_REGION(draw_border_n);
function DRAW_REGION(draw_border_s);
function DRAW_REGION(draw_border_w);
function DRAW_REGION(draw_border_e);
function DRAW_REGION(draw_corner_nw);
function DRAW_REGION(draw_corner_ne);
function DRAW_REGION(draw_corner_sw);
function DRAW_REGION(draw_corner_se);
function DRAW_REGION(draw_content);
function DRAW_REGION(draw_titlebar);

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

window_region_entry region_invariants[] =
{
   // IMPORTANT(law): Keep these entries in the same order as the
   // window_region_type enum. Or switch back to C for array designated
   // initializers.
   {WINDOW_INTERACTION_CLOSE,     CURSOR_ARROW},
   {WINDOW_INTERACTION_MAXIMIZE,  CURSOR_ARROW},
   {WINDOW_INTERACTION_MINIMIZE,  CURSOR_ARROW},
   {WINDOW_INTERACTION_MOVE,      CURSOR_MOVE, draw_titlebar},
   {WINDOW_INTERACTION_RAISE,     CURSOR_ARROW, draw_content},

   {WINDOW_INTERACTION_RESIZE_NW, CURSOR_RESIZE_DIAG_L, draw_corner_nw},
   {WINDOW_INTERACTION_RESIZE_NE, CURSOR_RESIZE_DIAG_R, draw_corner_ne},
   {WINDOW_INTERACTION_RESIZE_SW, CURSOR_RESIZE_DIAG_R, draw_corner_sw},
   {WINDOW_INTERACTION_RESIZE_SE, CURSOR_RESIZE_DIAG_L, draw_corner_se},

   {WINDOW_INTERACTION_RESIZE_N,  CURSOR_RESIZE_VERT, draw_border_n},
   {WINDOW_INTERACTION_RESIZE_S,  CURSOR_RESIZE_VERT, draw_border_s},
   {WINDOW_INTERACTION_RESIZE_W,  CURSOR_RESIZE_HORI, draw_border_w},
   {WINDOW_INTERACTION_RESIZE_E,  CURSOR_RESIZE_HORI, draw_border_e},
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
   i32 z;
};

struct hit_result
{
   u32 region_index;
};

struct desktop_window
{
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

struct desktop_configuration
{
   bool focus_follows_mouse;
};

// TODO(law): Since 0 is a valid index, we're using one outside the valid range
// of the array. Maybe reserve index 0 instead?
#define DESKTOP_WINDOW_NULL_INDEX (DESKTOP_WINDOW_MAX_COUNT)
#define DESKTOP_REGION_NULL_INDEX (WINDOW_REGION_COUNT)

struct desktop_state
{
   arena permanent;
   arena scratch;

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
};
