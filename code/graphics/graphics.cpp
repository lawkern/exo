/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

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

#define MAXIMUM(a, b) ((a > b) ? a : b)
#define MINIMUM(a, b) ((a < b) ? a : b)

#define EXO_WINDOW_MIN_WIDTH  100
#define EXO_WINDOW_MIN_HEIGHT 100
#define EXO_WINDOW_TITLE_BAR_DIM 18
#define EXO_WINDOW_EDGE_DIM 5

struct v4
{
   float r;
   float g;
   float b;
   float a;
};

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

function bool is_pressed(struct input_state button)
{
   // NOTE(law): Check if the button is currently being pressed this frame,
   // regardless of what frame it was initially pressed.
   bool result = button.is_pressed;
   return(result);
}

function bool was_pressed(struct input_state button)
{
   // NOTE(law): Check if the button transitioned to being pressed on the
   // current frame.
   bool result = button.is_pressed && button.changed_state;
   return(result);
}

struct exo_storage
{
   size_t size;
   u8 *memory;
};

struct rectangle
{
   s32 x;
   s32 y;
   s32 width;
   s32 height;
};

function rectangle create_rectangle(s32 x, s32 y, s32 width, s32 height)
{
   rectangle result = {x, y, width, height};
   return(result);
}

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

global cursor_type frame_cursor;

enum window_region_type
{
   WINDOW_REGION_TITLE_BAR,
   WINDOW_REGION_BORDER_N,
   WINDOW_REGION_BORDER_S,
   WINDOW_REGION_BORDER_W,
   WINDOW_REGION_BORDER_E,
   WINDOW_REGION_CORNER_NW,
   WINDOW_REGION_CORNER_NE,
   WINDOW_REGION_CORNER_SW,
   WINDOW_REGION_CORNER_SE,

   WINDOW_REGION_COUNT,
};

struct window_region
{
   rectangle bounds;
   window_region_type type;
   cursor_type cursor;
   window_interaction interaction;
   v4 color;
};

union window_layout
{
   window_region regions[WINDOW_REGION_COUNT];
};

struct exo_window
{
   union
   {
      struct
      {
         s32 posx;
         s32 posy;
         s32 width;
         s32 height;
      };
      rectangle bounds;
   };

   bool is_interacting;
   window_interaction interaction;

   void initialize(window_layout *layout, exo_input *input);
};

void initialize_layout(window_layout *result)
{
   // Title bar
   result->regions[WINDOW_REGION_TITLE_BAR].type        = WINDOW_REGION_TITLE_BAR;
   result->regions[WINDOW_REGION_TITLE_BAR].cursor      = CURSOR_MOVE;
   result->regions[WINDOW_REGION_TITLE_BAR].interaction = WINDOW_INTERACTION_MOVE;
   result->regions[WINDOW_REGION_TITLE_BAR].color       = {0.196f * 2, 0.188f * 2, 0.1843f * 2, 1.0f};

   // Borders
   v4 border_color = {0.0f, 0.0f, 1.0f, 1.0f};
   result->regions[WINDOW_REGION_BORDER_N].type        = WINDOW_REGION_BORDER_N;
   result->regions[WINDOW_REGION_BORDER_S].type        = WINDOW_REGION_BORDER_S;
   result->regions[WINDOW_REGION_BORDER_W].type        = WINDOW_REGION_BORDER_W;
   result->regions[WINDOW_REGION_BORDER_E].type        = WINDOW_REGION_BORDER_E;

   result->regions[WINDOW_REGION_BORDER_N].color       = border_color;
   result->regions[WINDOW_REGION_BORDER_S].color       = border_color;
   result->regions[WINDOW_REGION_BORDER_W].color       = border_color;
   result->regions[WINDOW_REGION_BORDER_E].color       = border_color;

   result->regions[WINDOW_REGION_BORDER_N].cursor      = CURSOR_RESIZE_VERT;
   result->regions[WINDOW_REGION_BORDER_S].cursor      = CURSOR_RESIZE_VERT;
   result->regions[WINDOW_REGION_BORDER_W].cursor      = CURSOR_RESIZE_HORI;
   result->regions[WINDOW_REGION_BORDER_E].cursor      = CURSOR_RESIZE_HORI;

   result->regions[WINDOW_REGION_BORDER_N].interaction = WINDOW_INTERACTION_RESIZE_N;
   result->regions[WINDOW_REGION_BORDER_S].interaction = WINDOW_INTERACTION_RESIZE_S;
   result->regions[WINDOW_REGION_BORDER_W].interaction = WINDOW_INTERACTION_RESIZE_W;
   result->regions[WINDOW_REGION_BORDER_E].interaction = WINDOW_INTERACTION_RESIZE_E;

   // Corners
   v4 corner_color = {0.0f, 1.0f, 0.0f, 1.0f};
   result->regions[WINDOW_REGION_CORNER_NW].type        = WINDOW_REGION_CORNER_NW;
   result->regions[WINDOW_REGION_CORNER_NE].type        = WINDOW_REGION_CORNER_NE;
   result->regions[WINDOW_REGION_CORNER_SW].type        = WINDOW_REGION_CORNER_SW;
   result->regions[WINDOW_REGION_CORNER_SE].type        = WINDOW_REGION_CORNER_SE;

   result->regions[WINDOW_REGION_CORNER_NW].color       = corner_color;
   result->regions[WINDOW_REGION_CORNER_NE].color       = corner_color;
   result->regions[WINDOW_REGION_CORNER_SW].color       = corner_color;
   result->regions[WINDOW_REGION_CORNER_SE].color       = corner_color;

   result->regions[WINDOW_REGION_CORNER_NW].cursor      = CURSOR_RESIZE_DIAG_L;
   result->regions[WINDOW_REGION_CORNER_NE].cursor      = CURSOR_RESIZE_DIAG_R;
   result->regions[WINDOW_REGION_CORNER_SW].cursor      = CURSOR_RESIZE_DIAG_R;
   result->regions[WINDOW_REGION_CORNER_SE].cursor      = CURSOR_RESIZE_DIAG_L;

   result->regions[WINDOW_REGION_CORNER_NW].interaction = WINDOW_INTERACTION_RESIZE_NW;
   result->regions[WINDOW_REGION_CORNER_NE].interaction = WINDOW_INTERACTION_RESIZE_NE;
   result->regions[WINDOW_REGION_CORNER_SW].interaction = WINDOW_INTERACTION_RESIZE_SW;
   result->regions[WINDOW_REGION_CORNER_SE].interaction = WINDOW_INTERACTION_RESIZE_SE;
}

function bool in_rectangle(rectangle rect, s32 x, s32 y)
{
   bool result = (x >= rect.x && x < (rect.x + rect.width) &&
                  y >= rect.y && y < (rect.y + rect.height));

   return(result);
}

void exo_window::initialize(window_layout *layout, exo_input *input)
{
   s32 edge = EXO_WINDOW_EDGE_DIM;
   s32 title = EXO_WINDOW_TITLE_BAR_DIM;

   // Compute the bounds of window regions based on window size.
   layout->regions[WINDOW_REGION_TITLE_BAR].bounds = create_rectangle(posx, posy - title + edge, width, title);
   layout->regions[WINDOW_REGION_BORDER_N].bounds = create_rectangle(posx, posy - title, width, edge);
   layout->regions[WINDOW_REGION_BORDER_S].bounds = create_rectangle(posx, posy + height, width, edge);
   layout->regions[WINDOW_REGION_BORDER_W].bounds = create_rectangle(posx - edge, posy - title + edge, edge, height + title - edge);
   layout->regions[WINDOW_REGION_BORDER_E].bounds = create_rectangle(posx + width, posy - title + edge, edge, height + title - edge);
   layout->regions[WINDOW_REGION_CORNER_NW].bounds = create_rectangle(posx - edge, posy - title, edge, edge);
   layout->regions[WINDOW_REGION_CORNER_NE].bounds = create_rectangle(posx + width, posy - title, edge, edge);
   layout->regions[WINDOW_REGION_CORNER_SW].bounds = create_rectangle(posx - edge, posy + height, edge, edge);
   layout->regions[WINDOW_REGION_CORNER_SE].bounds = create_rectangle(posx + width, posy + height, edge, edge);

   for(u32 region_index = 0; region_index < WINDOW_REGION_COUNT; ++region_index)
   {
      window_region *region = layout->regions + region_index;
      if(in_rectangle(region->bounds, input->mousex, input->mousey))
      {
         frame_cursor = region->cursor;
         if(was_pressed(input->mouse_buttons[MOUSE_BUTTON_LEFT]))
         {
            is_interacting = true;
            interaction = region->interaction;
         }
      }

      if(!is_pressed(input->mouse_buttons[MOUSE_BUTTON_LEFT]))
      {
         is_interacting = false;
         interaction = WINDOW_INTERACTION_NONE;
      }
   }
}

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

exo_texture load_bitmap(char *file_path, u32 offsetx = 0, u32 offsety = 0)
{
   exo_texture result = {0};
   result.offsetx = offsetx;
   result.offsety = offsety;

   FILE *file = fopen(file_path, "rb");
   assert(file);

   fseek(file, 0, SEEK_END);
   size_t size = ftell(file);
   fseek(file, 0, SEEK_SET);

   u8 *memory = (u8 *)malloc(size);
   assert(memory);

   size_t bytes_read = fread(memory, 1, size, file);
   assert(bytes_read == size);

   struct bitmap_header *header = (struct bitmap_header *)memory;

   assert(header->file_type == 0x4D42); // "BM"
   assert(header->bits_per_pixel == 32);

   result.width = header->width;
   result.height = header->height;
   result.memory = (u32 *)malloc(sizeof(u32) * result.width * result.height);

   u32 *source_memory = (u32 *)(memory + header->bitmap_offset);
   u32 *row = source_memory + (result.width * (result.height - 1));

   for(s32 y = 0; y < result.height; ++y)
   {
      for(s32 x = 0; x < result.width; ++x)
      {
         u32 color = *(row + x);
         float r = (float)((color >> 16) & 0xFF);
         float g = (float)((color >>  8) & 0xFF);
         float b = (float)((color >>  0) & 0xFF);
         float a = (float)((color >> 24) & 0xFF);

         float anormal = a / 255.0f;
         r *= anormal;
         g *= anormal;
         b *= anormal;

         result.memory[(y * result.width) + x] = (((u32)(r + 0.5f) << 16) |
                                                  ((u32)(g + 0.5f) << 8) |
                                                  ((u32)(b + 0.5f) << 0) |
                                                  ((u32)(a + 0.5f) << 24));
      }

      row -= result.width;
   }

   free(memory);
   fclose(file);

   return(result);
}

function void draw_rectangle(exo_texture *backbuffer, s32 posx, s32 posy, s32 width, s32 height, v4 color)
{
   s32 minx = MAXIMUM(posx, 0);
   s32 miny = MAXIMUM(posy, 0);
   s32 maxx = MINIMUM(posx + width, backbuffer->width - 1);
   s32 maxy = MINIMUM(posy + height, backbuffer->height - 1);

   float sr = color.r * 255.0f;
   float sg = color.g * 255.0f;
   float sb = color.b * 255.0f;
   float sa = color.a * 255.0f;

   if(color.a != 1.0f)
   {
      for(s32 y = miny; y < maxy; ++y)
      {
         for(s32 x = minx; x < maxx; ++x)
         {
            u32 *destination_pixel = backbuffer->memory + (y * backbuffer->width) + x;

            u32 destination_color = *destination_pixel;
            float dr = (float)((destination_color >> 16) & 0xFF);
            float dg = (float)((destination_color >>  8) & 0xFF);
            float db = (float)((destination_color >>  0) & 0xFF);
            float da = (float)((destination_color >> 24) & 0xFF);

            float sanormal = color.a; // sa / 255.0f;

            float r = ((1.0f - sanormal) * dr) + sr;
            float g = ((1.0f - sanormal) * dg) + sg;
            float b = ((1.0f - sanormal) * db) + sb;
            float a = ((1.0f - sanormal) * da) + sa;

            *destination_pixel = (((u32)(r + 0.5f) << 16) |
                                  ((u32)(g + 0.5f) << 8) |
                                  ((u32)(b + 0.5f) << 0) |
                                  ((u32)(a + 0.5f) << 24));
         }
      }
   }
   else
   {
      u32 packed_color = (((u32)(sr + 0.5f) << 16) |
                          ((u32)(sg + 0.5f) << 8) |
                          ((u32)(sb + 0.5f) << 0) |
                          ((u32)(sa + 0.5f) << 24));

      for(s32 y = miny; y < maxy; ++y)
      {
         for(s32 x = minx; x < maxx; ++x)
         {
            backbuffer->memory[(y * backbuffer->width) + x] = packed_color;
         }
      }
   }
}

void draw_rectangle(exo_texture *backbuffer, struct rectangle rect, v4 color)
{
   draw_rectangle(backbuffer, rect.x, rect.y, rect.width, rect.height, color);
}

void draw_bitmap(exo_texture *backbuffer, exo_texture *bitmap, s32 posx, s32 posy)
{
   posx -= bitmap->offsetx;
   posy -= bitmap->offsety;

   s32 minx = MAXIMUM(posx, 0);
   s32 miny = MAXIMUM(posy, 0);
   s32 maxx = MINIMUM(posx + bitmap->width, backbuffer->width - 1);
   s32 maxy = MINIMUM(posy + bitmap->height, backbuffer->height - 1);

   for(s32 destinationy = miny; destinationy < maxy; ++destinationy)
   {
      for(s32 destinationx = minx; destinationx < maxx; ++destinationx)
      {
         s32 sourcex = destinationx - minx;
         s32 sourcey = destinationy - miny;

         u32 source_color = bitmap->memory[(sourcey * bitmap->width) + sourcex];
         float sr = (float)((source_color >> 16) & 0xFF);
         float sg = (float)((source_color >>  8) & 0xFF);
         float sb = (float)((source_color >>  0) & 0xFF);
         float sa = (float)((source_color >> 24) & 0xFF);

         u32 *destination_pixel = backbuffer->memory + (destinationy * backbuffer->width) + destinationx;

         u32 destination_color = *destination_pixel;
         float dr = (float)((destination_color >> 16) & 0xFF);
         float dg = (float)((destination_color >>  8) & 0xFF);
         float db = (float)((destination_color >>  0) & 0xFF);
         float da = (float)((destination_color >> 24) & 0xFF);

         float sanormal = sa / 255.0f;

         float r = ((1.0f - sanormal) * dr) + sr;
         float g = ((1.0f - sanormal) * dg) + sg;
         float b = ((1.0f - sanormal) * db) + sb;
         float a = ((1.0f - sanormal) * da) + sa;

         u32 color = (((u32)(r + 0.5f) << 16) |
                      ((u32)(g + 0.5f) << 8) |
                      ((u32)(b + 0.5f) << 0) |
                      ((u32)(a + 0.5f) << 24));

         *destination_pixel = color;
      }
   }
}

struct exo_state
{
   exo_window window;
   exo_texture cursors[CURSOR_COUNT];
   window_layout base_layout;

   bool is_initialized;
};

void update(exo_texture *backbuffer, exo_input input, exo_storage storage)
{
   exo_state *state = (exo_state *)storage.memory;
   if(!state->is_initialized)
   {
      state->window = exo_window{60, 60, 300, 200};

      state->cursors[CURSOR_ARROW]         = load_bitmap("cursor_arrow.bmp");
      state->cursors[CURSOR_MOVE]          = load_bitmap("cursor_move.bmp", 8, 8);
      state->cursors[CURSOR_RESIZE_VERT]   = load_bitmap("cursor_vertical_resize.bmp", 4, 8);
      state->cursors[CURSOR_RESIZE_HORI]   = load_bitmap("cursor_horizontal_resize.bmp", 8, 4);
      state->cursors[CURSOR_RESIZE_DIAG_L] = load_bitmap("cursor_diagonal_left.bmp", 7, 7);
      state->cursors[CURSOR_RESIZE_DIAG_R] = load_bitmap("cursor_diagonal_right.bmp", 7, 7);

      initialize_layout(&state->base_layout);

      state->is_initialized = true;
   }

   frame_cursor = CURSOR_ARROW;
   window_layout *layout = &state->base_layout;

   // Draw background.
   v4 background_color = {0.157f, 0.157f, 0.157f, 1.0f};
   draw_rectangle(backbuffer, 0, 0, backbuffer->width, backbuffer->height, background_color);

   exo_window *window = &state->window;
   window->initialize(layout, &input);

   if(window->is_interacting)
   {
      switch(window->interaction)
      {
         case WINDOW_INTERACTION_NONE:
         {
            assert(!"Interacting with window without interaction type.");
         } break;

         case WINDOW_INTERACTION_MOVE:
         {
            frame_cursor = CURSOR_MOVE;

            window->posx += (input.mousex - input.previous_mousex);
            window->posy += (input.mousey - input.previous_mousey);
         } break;

         case WINDOW_INTERACTION_RESIZE_N:
         {
            frame_cursor = CURSOR_RESIZE_VERT;

            s32 delta = input.mousey - input.previous_mousey;
            window->posy += delta;
            window->height -= delta;
         } break;

         case WINDOW_INTERACTION_RESIZE_S:
         {
            frame_cursor = CURSOR_RESIZE_VERT;

            s32 delta = input.mousey - input.previous_mousey;
            window->height += delta;
         } break;

         case WINDOW_INTERACTION_RESIZE_W:
         {
            frame_cursor = CURSOR_RESIZE_HORI;

            s32 delta = input.mousex - input.previous_mousex;
            window->posx += delta;
            window->width -= delta;
         } break;

         case WINDOW_INTERACTION_RESIZE_E:
         {
            frame_cursor = CURSOR_RESIZE_HORI;

            s32 delta = input.mousex - input.previous_mousex;
            window->width += delta;
         } break;

         case WINDOW_INTERACTION_RESIZE_NW:
         {
            frame_cursor = CURSOR_RESIZE_DIAG_L;

            s32 deltax = input.mousex - input.previous_mousex;
            s32 deltay = input.mousey - input.previous_mousey;
            window->posx += deltax;
            window->posy += deltay;
            window->width -= deltax;
            window->height -= deltay;
         } break;

         case WINDOW_INTERACTION_RESIZE_SW:
         {
            frame_cursor = CURSOR_RESIZE_DIAG_R;

            s32 deltax = input.mousex - input.previous_mousex;
            s32 deltay = input.mousey - input.previous_mousey;
            window->posx += deltax;
            window->width -= deltax;
            window->height += deltay;
         } break;

         case WINDOW_INTERACTION_RESIZE_NE:
         {
            frame_cursor = CURSOR_RESIZE_DIAG_R;

            s32 deltax = input.mousex - input.previous_mousex;
            s32 deltay = input.mousey - input.previous_mousey;
            window->posy += deltay;
            window->width += deltax;
            window->height -= deltay;
         } break;

         case WINDOW_INTERACTION_RESIZE_SE:
         {
            frame_cursor = CURSOR_RESIZE_DIAG_L;

            s32 deltax = input.mousex - input.previous_mousex;
            s32 deltay = input.mousey - input.previous_mousey;
            window->width += deltax;
            window->height += deltay;
         } break;
      }
   }

   // Draw window.
   v4 window_color = {0.196f, 0.188f, 0.1843f, 1.0f};
   draw_rectangle(backbuffer, window->bounds, window_color);

   for(u32 region_index = 0; region_index < WINDOW_REGION_COUNT; ++region_index)
   {
      window_region *region = layout->regions + region_index;
      draw_rectangle(backbuffer, region->bounds, region->color);
   }

   // Draw cursor.
   draw_bitmap(backbuffer, state->cursors + frame_cursor, input.mousex, input.mousey);
}
