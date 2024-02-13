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

struct texture
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

struct input
{
   float mouseu;
   float mousev;

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

struct storage
{
   size_t size;
   u8 *memory;
};

struct rect
{
   s32 x;
   s32 y;
   s32 width;
   s32 height;
};

enum window_movement_type
{
   WINDOW_MOVEMENT_NONE,
   WINDOW_MOVEMENT_MOVE,
   WINDOW_MOVEMENT_N_RESIZE,
   WINDOW_MOVEMENT_S_RESIZE,
   WINDOW_MOVEMENT_W_RESIZE,
   WINDOW_MOVEMENT_E_RESIZE,
   WINDOW_MOVEMENT_NW_RESIZE,
   WINDOW_MOVEMENT_NE_RESIZE,
   WINDOW_MOVEMENT_SW_RESIZE,
   WINDOW_MOVEMENT_SE_RESIZE,
};

struct window
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
      struct rect bounds;
   };

   enum window_movement_type movement_type;
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

struct texture load_bitmap(char *file_path)
{
   struct texture result = {0};

   FILE *file = fopen(file_path, "rb");
   assert(file);

   fseek(file, 0, SEEK_END);
   size_t size = ftell(file);
   fseek(file, 0, SEEK_SET);

   u8 *memory = malloc(size);
   assert(memory);

   size_t bytes_read = fread(memory, 1, size, file);
   assert(bytes_read == size);

   struct bitmap_header *header = (struct bitmap_header *)memory;

   assert(header->file_type == 0x4D42); // "BM"
   assert(header->bits_per_pixel == 32);

   result.width = header->width;
   result.height = header->height;
   result.memory = malloc(sizeof(u32) * result.width * result.height);

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

typedef struct
{
   float r;
   float g;
   float b;
   float a;
} v4;

void draw_rectangle(struct texture *backbuffer, s32 posx, s32 posy, s32 width, s32 height, v4 color)
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

void draw_rect(struct texture *backbuffer, struct rect rectangle, v4 color)
{
   draw_rectangle(backbuffer, rectangle.x, rectangle.y, rectangle.width, rectangle.height, color);
}

void draw_bitmap(struct texture *backbuffer, struct texture *bitmap, s32 posx, s32 posy)
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

enum cursor_type
{
   CURSOR_ARROW,
   CURSOR_MOVE,
   CURSOR_VERTICAL_RESIZE,
   CURSOR_HORIZONTAL_RESIZE,
   CURSOR_DIAGONAL_LEFT,
   CURSOR_DIAGONAL_RIGHT,
   CURSOR_COUNT,
};

struct state
{
   struct window window;
   struct texture cursors[CURSOR_COUNT];

   s32 previous_mousex;
   s32 previous_mousey;

   bool is_initialized;
};

function bool in_rectangle(struct rect rectangle, s32 x, s32 y)
{
   bool result = (x >= rectangle.x && x < (rectangle.x + rectangle.width) &&
                  y >= rectangle.y && y < (rectangle.y + rectangle.height));

   return(result);
}

void update(struct texture *backbuffer, struct input *input, struct storage *storage)
{
   struct state *state = (struct state *)storage->memory;

   s32 mousex = (s32)((input->mouseu * (float)backbuffer->width) + 0.5f);
   s32 mousey = (s32)((input->mousev * (float)backbuffer->height) + 0.5f);

   if(!state->is_initialized)
   {
      state->window = (struct window){30, 20, 300, 200};

      state->cursors[CURSOR_ARROW] = load_bitmap("cursor_arrow.bmp");

      state->cursors[CURSOR_VERTICAL_RESIZE] = load_bitmap("cursor_vertical_resize.bmp");
      state->cursors[CURSOR_VERTICAL_RESIZE].offsetx = 4;
      state->cursors[CURSOR_VERTICAL_RESIZE].offsety = 8;

      state->cursors[CURSOR_HORIZONTAL_RESIZE] = load_bitmap("cursor_horizontal_resize.bmp");
      state->cursors[CURSOR_HORIZONTAL_RESIZE].offsetx = 8;
      state->cursors[CURSOR_HORIZONTAL_RESIZE].offsety = 4;

      state->cursors[CURSOR_DIAGONAL_LEFT] = load_bitmap("cursor_diagonal_left.bmp");
      state->cursors[CURSOR_DIAGONAL_LEFT].offsetx = 7;
      state->cursors[CURSOR_DIAGONAL_LEFT].offsety = 7;

      state->cursors[CURSOR_DIAGONAL_RIGHT] = load_bitmap("cursor_diagonal_right.bmp");
      state->cursors[CURSOR_DIAGONAL_RIGHT].offsetx = 7;
      state->cursors[CURSOR_DIAGONAL_RIGHT].offsety = 7;

      state->cursors[CURSOR_MOVE] = load_bitmap("cursor_move.bmp");
      state->cursors[CURSOR_MOVE].offsetx = 8;
      state->cursors[CURSOR_MOVE].offsety = 8;

      state->previous_mousex = mousex;
      state->previous_mousey = mousey;

      state->is_initialized = true;
   }

   struct input_state mouse_button_left = input->mouse_buttons[MOUSE_BUTTON_LEFT];

   // Draw background.
   v4 background_color = {0.157f, 0.157f, 0.157f, 1.0f};
   draw_rectangle(backbuffer, 0, 0, backbuffer->width, backbuffer->height, background_color);

   // Deterimine the size of window sections.
   struct window *window = &state->window;
   s32 edge_dim = 5;
   s32 title_dim = 18;

   struct rect title_bar = {window->posx, window->posy - title_dim + edge_dim, window->width, title_dim};

   struct rect n_border = {window->posx, window->posy - title_dim, window->width, edge_dim};
   struct rect s_border = {window->posx, window->posy + window->height, window->width, edge_dim};
   struct rect w_border = {window->posx - edge_dim, window->posy - title_dim + edge_dim, edge_dim, window->height + title_dim - edge_dim};
   struct rect e_border = {window->posx + window->width, window->posy - title_dim + edge_dim, edge_dim, window->height + title_dim - edge_dim};

   struct rect nw_corner = {window->posx - edge_dim, window->posy - title_dim, edge_dim, edge_dim};
   struct rect ne_corner = {window->posx + window->width, window->posy - title_dim, edge_dim, edge_dim};
   struct rect sw_corner = {window->posx - edge_dim, window->posy + window->height, edge_dim, edge_dim};
   struct rect se_corner = {window->posx + window->width, window->posy + window->height, edge_dim, edge_dim};

   v4 window_color = {0.196f, 0.188f, 0.1843f, 1.0f};
   v4 corner_color = {0.0f, 1.0f, 0.0f, 1.0f};
   v4 border_color = {0.0f, 0.0f, 1.0f, 1.0f};
   v4 title_bar_color = {0.196f * 2, 0.188f * 2, 0.1843f * 2, 1.0f};

   // Determine if window is being moved or resized.
   enum cursor_type cursor = CURSOR_ARROW;
   if(window->movement_type == WINDOW_MOVEMENT_NONE)
   {
      if(in_rectangle(n_border, mousex, mousey))
      {
         cursor = CURSOR_VERTICAL_RESIZE;
         if(was_pressed(mouse_button_left))
         {
            window->movement_type = WINDOW_MOVEMENT_N_RESIZE;
         }
      }
      else if(in_rectangle(s_border, mousex, mousey))
      {
         cursor = CURSOR_VERTICAL_RESIZE;
         if(was_pressed(mouse_button_left))
         {
            window->movement_type = WINDOW_MOVEMENT_S_RESIZE;
         }
      }
      else if(in_rectangle(w_border, mousex, mousey))
      {
         cursor = CURSOR_HORIZONTAL_RESIZE;
         if(was_pressed(mouse_button_left))
         {
            window->movement_type = WINDOW_MOVEMENT_W_RESIZE;
         }
      }
      else if(in_rectangle(e_border, mousex, mousey))
      {
         cursor = CURSOR_HORIZONTAL_RESIZE;
         if(was_pressed(mouse_button_left))
         {
            window->movement_type = WINDOW_MOVEMENT_E_RESIZE;
         }
      }
      else if(in_rectangle(nw_corner, mousex, mousey))
      {
         cursor = CURSOR_DIAGONAL_LEFT;
         if(was_pressed(mouse_button_left))
         {
            window->movement_type = WINDOW_MOVEMENT_NW_RESIZE;
         }
      }
      else if(in_rectangle(sw_corner, mousex, mousey))
      {
         cursor = CURSOR_DIAGONAL_RIGHT;
         if(was_pressed(mouse_button_left))
         {
            window->movement_type = WINDOW_MOVEMENT_SW_RESIZE;
         }
      }
      else if(in_rectangle(ne_corner, mousex, mousey))
      {
         cursor = CURSOR_DIAGONAL_RIGHT;
         if(was_pressed(mouse_button_left))
         {
            window->movement_type = WINDOW_MOVEMENT_NE_RESIZE;
         }
      }
      else if(in_rectangle(se_corner, mousex, mousey))
      {
         cursor = CURSOR_DIAGONAL_LEFT;
         if(was_pressed(mouse_button_left))
         {
            window->movement_type = WINDOW_MOVEMENT_SE_RESIZE;
         }
      }
      else if(in_rectangle(title_bar, mousex, mousey))
      {
         cursor = CURSOR_MOVE;
         if(was_pressed(mouse_button_left))
         {
            window->movement_type = WINDOW_MOVEMENT_MOVE;
         }
      }
   }


   // Update window size and position.
   if(is_pressed(mouse_button_left))
   {
      switch(window->movement_type)
      {
         case WINDOW_MOVEMENT_NONE: {} break;

         case WINDOW_MOVEMENT_N_RESIZE:
         {
            cursor = CURSOR_VERTICAL_RESIZE;
            s32 delta = mousey - state->previous_mousey;
            window->posy += delta;
            window->height -= delta;
         } break;

         case WINDOW_MOVEMENT_S_RESIZE:
         {
            cursor = CURSOR_VERTICAL_RESIZE;
            s32 delta = mousey - state->previous_mousey;
            window->height += delta;
         } break;

         case WINDOW_MOVEMENT_W_RESIZE:
         {
            cursor = CURSOR_HORIZONTAL_RESIZE;
            s32 delta = mousex - state->previous_mousex;
            window->posx += delta;
            window->width -= delta;
         } break;

         case WINDOW_MOVEMENT_E_RESIZE:
         {
            cursor = CURSOR_HORIZONTAL_RESIZE;
            s32 delta = mousex - state->previous_mousex;
            window->width += delta;
         } break;

         case WINDOW_MOVEMENT_NW_RESIZE:
         {
            cursor = CURSOR_DIAGONAL_LEFT;
            s32 deltax = mousex - state->previous_mousex;
            s32 deltay = mousey - state->previous_mousey;
            window->posx += deltax;
            window->posy += deltay;
            window->width -= deltax;
            window->height -= deltay;
         } break;

         case WINDOW_MOVEMENT_SW_RESIZE:
         {
            cursor = CURSOR_DIAGONAL_RIGHT;
            s32 deltax = mousex - state->previous_mousex;
            s32 deltay = mousey - state->previous_mousey;
            window->posx += deltax;
            window->width -= deltax;
            window->height += deltay;
         } break;

         case WINDOW_MOVEMENT_NE_RESIZE:
         {
            cursor = CURSOR_DIAGONAL_RIGHT;
            s32 deltax = mousex - state->previous_mousex;
            s32 deltay = mousey - state->previous_mousey;
            window->posy += deltay;
            window->width += deltax;
            window->height -= deltay;
         } break;

         case WINDOW_MOVEMENT_SE_RESIZE:
         {
            cursor = CURSOR_DIAGONAL_LEFT;
            s32 deltax = mousex - state->previous_mousex;
            s32 deltay = mousey - state->previous_mousey;
            window->width += deltax;
            window->height += deltay;
         } break;

         case WINDOW_MOVEMENT_MOVE:
         {
            cursor = CURSOR_MOVE;
            window_color.a = 0.75f;
            window->posx += (mousex - state->previous_mousex);
            window->posy += (mousey - state->previous_mousey);
         } break;
      }
   }
   else
   {
      window->movement_type = WINDOW_MOVEMENT_NONE;
   }

   // Draw window.
   draw_rect(backbuffer, window->bounds, window_color);

   draw_rect(backbuffer, nw_corner, corner_color);
   draw_rect(backbuffer, ne_corner, corner_color);
   draw_rect(backbuffer, sw_corner, corner_color);
   draw_rect(backbuffer, se_corner, corner_color);

   draw_rect(backbuffer, n_border, border_color);
   draw_rect(backbuffer, s_border, border_color);
   draw_rect(backbuffer, w_border, border_color);
   draw_rect(backbuffer, e_border, border_color);

   draw_rect(backbuffer, title_bar, title_bar_color);

   // Draw cursor.
   draw_bitmap(backbuffer, state->cursors + cursor, mousex, mousey);

   // Store this frame's mouse position for next frame.
   state->previous_mousex = mousex;
   state->previous_mousey = mousey;
}
