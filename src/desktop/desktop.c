/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */

#include "desktop.h"
#include "renderer.h"
#include "text.c"

function bool is_pressed(input_state button)
{
   // NOTE: Check if the button is currently being pressed this frame,
   // regardless of what frame it was initially pressed.
   bool result = button.is_pressed;

   return(result);
}

function bool was_pressed(input_state button)
{
   // NOTE: Check if the button transitioned to being pressed on the
   // current frame.
   bool result = button.is_pressed && button.changed_state;
   return(result);
}

function bool was_released(input_state button)
{
   // NOTE: Check if the button transitioned to being pressed on the
   // current frame.
   bool result = !button.is_pressed && button.changed_state;
   return(result);
}

function rectangle create_rectangle(s32 x, s32 y, s32 width, s32 height)
{
   rectangle result = {x, y, width, height};
   return(result);
}

function bool in_rectangle(rectangle rect, s32 x, s32 y)
{
   bool result = (x >= rect.x && x < (rect.x + rect.width) &&
                  y >= rect.y && y < (rect.y + rect.height));

   return(result);
}

function texture load_bitmap(desktop_context *desktop, char *file_path, u32 offsetx, u32 offsety)
{
   texture result = {0};
   result.offsetx = offsetx;
   result.offsety = offsety;

   FILE *file = fopen(file_path, "rb");
   assert(file);

   fseek(file, 0, SEEK_END);
   size_t size = ftell(file);
   fseek(file, 0, SEEK_SET);

   arena_marker marker = arena_marker_set(&desktop->scratch_arena);
   u8 *memory = (u8 *)arena_allocate(&desktop->scratch_arena, u8, size);

   size_t bytes_read = fread(memory, 1, size, file);
   assert(bytes_read == size);

   bitmap_header *header = (bitmap_header *)memory;

   assert(header->file_type == 0x4D42); // "BM"
   assert(header->bits_per_pixel == 32);

   result.width = header->width;
   result.height = header->height;
   result.memory = (u32 *)arena_allocate(&desktop->texture_arena, u32, result.width * result.height);

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

   arena_marker_restore(marker);
   fclose(file);

   return(result);
}

#if 0
function void compute_region_size(rectangle *result, desktop_window *window, window_region_type region)
{
   result->x = 0;
   result->y = 0;
   result->width = 10;
   result->height = 10;

   s32 b = DESKTOP_WINDOW_DIM_BUTTON;
   s32 e = DESKTOP_WINDOW_DIM_EDGE;
   s32 t = DESKTOP_WINDOW_DIM_TITLEBAR;
   s32 c = DESKTOP_WINDOW_DIM_CORNER;
   s32 b2 = DESKTOP_WINDOW_HALFDIM_BUTTON;
   s32 e2 = DESKTOP_WINDOW_HALFDIM_EDGE;
   s32 t2 = DESKTOP_WINDOW_HALFDIM_TITLEBAR;

   s32 x = window->content.x;
   s32 y = window->content.y;
   s32 w = MAXIMUM(window->content.width, DESKTOP_WINDOW_MIN_WIDTH);
   s32 h = MAXIMUM(window->content.height, DESKTOP_WINDOW_MIN_HEIGHT);

   s32 buttonx = x + w - b - e2;
   s32 buttony = y - t2 - b2;

   switch(region)
   {
      case WINDOW_REGION_BUTTON_CLOSE:
      {
         *result = create_rectangle(buttonx, buttony, b, b);
      } break;
      case WINDOW_REGION_BUTTON_MAXIMIZE:
      {
         *result = create_rectangle(buttonx - b, buttony, b, b);
      } break;
      case WINDOW_REGION_BUTTON_MINIMIZE:
      {
         *result = create_rectangle(buttonx - 2*b, buttony, b, b);
      } break;
      case WINDOW_REGION_TITLEBAR:
      {
         *result = create_rectangle(x, y - t, w, t);
      } break;
      case WINDOW_REGION_CONTENT:
      {
         *result = create_rectangle(x, y, w, h);
      } break;

      case WINDOW_REGION_CORNER_NW:
      {
         *result = create_rectangle(x - e, y - t - e, c, c);
      } break;
      case WINDOW_REGION_CORNER_NE:
      {
         *result = create_rectangle(x + w + e - c, y - t - e, c, c);
      } break;
      case WINDOW_REGION_CORNER_SW:
      {
         *result = create_rectangle(x - e, y + h + e - c, c, c);
      } break;
      case WINDOW_REGION_CORNER_SE:
      {
         *result = create_rectangle(x + w + e - c, y + h + e - c, c, c);
      } break;

      case WINDOW_REGION_BORDER_N:
      {
         *result = create_rectangle(x, y - t - e, w, e);
      } break;

      case WINDOW_REGION_BORDER_S:
      {
         *result = create_rectangle(x, y + h, w, e);
      } break;
      case WINDOW_REGION_BORDER_W:
      {
         *result = create_rectangle(x - e, y - t, e, h + t);
      } break;
      case WINDOW_REGION_BORDER_E:
      {
         *result = create_rectangle(x + w, y - t, e, h + t);
      } break;

      default:
      {
         assert(!"Unhandled region type.");
      } break;
   }
}

function void compute_window_bounds(rectangle *result, desktop_window *window)
{
   // TODO: Stop hard-coding offsets like this.

   compute_region_size(result, window, WINDOW_REGION_CONTENT);

   result->x -= DESKTOP_WINDOW_DIM_EDGE;
   result->y -= (DESKTOP_WINDOW_DIM_TITLEBAR + DESKTOP_WINDOW_DIM_EDGE);

   result->width += (2 * DESKTOP_WINDOW_DIM_EDGE);
   result->height += (DESKTOP_WINDOW_DIM_TITLEBAR + (2 * DESKTOP_WINDOW_DIM_EDGE));
}
#endif

function void draw_rectangle_rect(texture *backbuffer, rectangle rect, vec4 color)
{
   draw_rectangle(backbuffer, rect.x, rect.y, rect.width, rect.height, color);
}

function void draw_outline_rect(texture *destination, rectangle bounds, vec4 color)
{
   draw_outline(destination, bounds.x, bounds.y, bounds.width, bounds.height, color);
}

#if 0
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


#define HLDIM 2

function DRAW_REGION(draw_border_n)
{
   rectangle bounds;
   compute_region_size(&bounds, window, WINDOW_REGION_BORDER_N);

   draw_rectangle(destination, bounds.x, bounds.y, bounds.width, HLDIM, PALETTE[0]);
   draw_rectangle(destination, bounds.x, bounds.y + HLDIM, bounds.width, bounds.height - HLDIM, PALETTE[1]);
}

function DRAW_REGION(draw_border_s)
{
   rectangle bounds;
   compute_region_size(&bounds, window, WINDOW_REGION_BORDER_S);

   draw_rectangle(destination, bounds.x, bounds.y, bounds.width, bounds.height - HLDIM, PALETTE[1]);
   draw_rectangle(destination, bounds.x, bounds.y + bounds.height - HLDIM, bounds.width, HLDIM, PALETTE[2]);
}

function DRAW_REGION(draw_border_w)
{
   rectangle bounds;
   compute_region_size(&bounds, window, WINDOW_REGION_BORDER_W);

   draw_rectangle(destination, bounds.x, bounds.y, HLDIM, bounds.height, PALETTE[0]);
   draw_rectangle(destination, bounds.x + HLDIM, bounds.y, bounds.width - HLDIM, bounds.height, PALETTE[1]);
}

function DRAW_REGION(draw_border_e)
{
   rectangle bounds;
   compute_region_size(&bounds, window, WINDOW_REGION_BORDER_E);

   draw_rectangle(destination, bounds.x, bounds.y, bounds.width - HLDIM, bounds.height, PALETTE[1]);
   draw_rectangle(destination, bounds.x + bounds.width - HLDIM, bounds.y, HLDIM, bounds.height, PALETTE[2]);
}

function DRAW_REGION(draw_corner_nw)
{
   rectangle bounds;
   compute_region_size(&bounds, window, WINDOW_REGION_CORNER_NW);

   draw_rectangle(destination, bounds.x, bounds.y, HLDIM, bounds.height, PALETTE[0]);
   draw_rectangle(destination, bounds.x + HLDIM, bounds.y, bounds.width - HLDIM, HLDIM, PALETTE[0]);
   draw_rectangle(destination, bounds.x + HLDIM, bounds.y + HLDIM, bounds.width - HLDIM, bounds.height - HLDIM, PALETTE[1]);
}

function DRAW_REGION(draw_corner_ne)
{
   rectangle bounds;
   compute_region_size(&bounds, window, WINDOW_REGION_CORNER_NE);

   draw_rectangle(destination, bounds.x, bounds.y, bounds.width - HLDIM, HLDIM, PALETTE[0]);
   draw_rectangle(destination, bounds.x + bounds.width - HLDIM, bounds.y, HLDIM, bounds.height, PALETTE[2]);
   draw_rectangle(destination, bounds.x, bounds.y + HLDIM, bounds.width - HLDIM, bounds.height - HLDIM, PALETTE[1]);
}

function DRAW_REGION(draw_corner_sw)
{
   rectangle bounds;
   compute_region_size(&bounds, window, WINDOW_REGION_CORNER_SW);

   draw_rectangle(destination, bounds.x, bounds.y, HLDIM, bounds.height, PALETTE[0]);
   draw_rectangle(destination, bounds.x + HLDIM, bounds.y + bounds.height - HLDIM, bounds.width - HLDIM, HLDIM, PALETTE[2]);
   draw_rectangle(destination, bounds.x + HLDIM, bounds.y, bounds.width - HLDIM, bounds.height - HLDIM, PALETTE[1]);
}

function DRAW_REGION(draw_corner_se)
{
   rectangle bounds;
   compute_region_size(&bounds, window, WINDOW_REGION_CORNER_SE);

   draw_rectangle(destination, bounds.x, bounds.y, bounds.width - HLDIM, bounds.height - HLDIM, PALETTE[1]);
   draw_rectangle(destination, bounds.x + bounds.width - HLDIM, bounds.y, HLDIM, bounds.height, PALETTE[2]);
   draw_rectangle(destination, bounds.x, bounds.y + bounds.height - HLDIM, bounds.width - HLDIM, HLDIM, PALETTE[2]);
}

#undef HLDIM

function DRAW_REGION(draw_content)
{
   texture *canvas = &window->canvas;

   rectangle bounds;
   compute_region_size(&bounds, window, WINDOW_REGION_CONTENT);
   draw_rectangle_rect(destination, bounds, PALETTE[4]);

   clear(canvas, PALETTE[2]);

   s32 x = 3;
   s32 y = 6;

   char text_line[64];
   char *format = "{x:%d y:%d w:%d h:%d}";

   int length = sprintf(text_line, format, window->x, window->y, window->width, window->height);
   draw_text_line(canvas, x, &y, string8new((u8 *)text_line, length));

   length = sprintf(text_line, format, bounds.x, bounds.y, bounds.width, bounds.height);
   draw_text_line(canvas, x, &y, string8new((u8 *)text_line, length));

   length = sprintf(text_line, "state:%d", window->state);
   draw_text_line(canvas, x, &y, string8new((u8 *)text_line, length));

   y = ADVANCE_TEXT_LINE(y);
   draw_text_line(canvas, x, &y, string8("+----------------------------+"));
   draw_text_line(canvas, x, &y, string8("| ASCII FONT TEST            |"));
   draw_text_line(canvas, x, &y, string8("|----------------------------|"));
   draw_text_line(canvas, x, &y, string8("| ABCDEFGHIJKLMNOPQRSTUVWXYZ |"));
   draw_text_line(canvas, x, &y, string8("| abcdefghijklmnopqrstuvwxyz |"));
   draw_text_line(canvas, x, &y, string8("| AaBbCcDdEeFfGgHhIiJjKkLlMm |"));
   draw_text_line(canvas, x, &y, string8("| NnOoPpQqRrSsTtUuVvWwXxYyZz |"));
   draw_text_line(canvas, x, &y, string8("| 0123456789!\"#$%&'()*+,-./: |"));
   draw_text_line(canvas, x, &y, string8("| ;<=>?@[\\]^_`{|}~           |"));
   draw_text_line(canvas, x, &y, string8("+----------------------------+"));

   draw_texture_bounded(destination, canvas, bounds.x, bounds.y, bounds.width, bounds.height);
}

function DRAW_REGION(draw_titlebar)
{
   rectangle bounds;
   compute_region_size(&bounds, window, WINDOW_REGION_TITLEBAR);

   vec4 active_color = DEBUG_COLOR_GREEN;
   vec4 passive_color = PALETTE[1];

   draw_rectangle_rect(destination, bounds, (is_active_window) ? active_color : passive_color);

   s32 x = bounds.x + 3;
   s32 y = ALIGN_TEXT_VERTICALLY(bounds.y, DESKTOP_WINDOW_DIM_TITLEBAR);
   draw_text(destination, x, y, window->title);
}
#endif

function bool is_window_visible(desktop_window *window)
{
   bool result = (window->state != WINDOW_STATE_CLOSED && window->state != WINDOW_STATE_MINIMIZED);
   return(result);
}

function rectangle get_close_button_rect(desktop_window *window)
{
   int x = window->x + window->width - 20;
   int y = window->y + 6;
   int width = 9;
   int height = 9;

   return create_rectangle(x, y, width, height);
}

function rectangle get_maximize_button_rect(desktop_window *window)
{
   rectangle result = get_close_button_rect(window);
   result.x -= 16;

   return(result);
}

function rectangle get_titlebar_rect(desktop_window *window)
{
   return create_rectangle(window->x, window->y, window->width, DESKTOP_WINDOW_DIM_TITLEBAR);
}

function rectangle resize_rectangle(rectangle rect, int offset)
{
   rectangle result = rect;
   result.x -= offset;
   result.y -= offset;

   result.width += (offset*2);
   result.height += (offset*2);

   return(result);
}

function void draw_window(desktop_context *desktop, desktop_window *window, texture *destination)
{
   if(is_window_visible(window))
   {
      int window_width  = MAXIMUM(MINIMUM(window->width, desktop->backbuffer.width), 100);
      int window_height = MAXIMUM(MINIMUM(window->height, desktop->backbuffer.height), 100);

      int x = window->x;
      int y = window->y;

      draw_outline(destination, x, y, window_width, window_height, DEBUG_COLOR_BLACK);
      draw_outline(destination, x+window_width, y+1, 1, window_height, DEBUG_COLOR_BLACK);
      draw_outline(destination, x+1, y+window_height, window_width, 1, DEBUG_COLOR_BLACK);
      draw_rectangle(destination, x+1, y+1, window_width-2, window_height-2, DEBUG_COLOR_WHITE);
      if(window == desktop->active_window)
      {
         draw_outline(destination, x, y, window_width, window_height, DEBUG_COLOR_BLUE);
      }

      // NOTE: Draw titlebar.
      {
         int w = window_width;
         int h = DESKTOP_WINDOW_DIM_TITLEBAR;

         draw_rectangle(destination, x+1, y+h-1, w-2, 1, DEBUG_COLOR_BLACK);
         draw_rectangle(destination, x+1, y+h+1, w-2, 1, DEBUG_COLOR_BLACK);
         if(window == desktop->hot_window)
         {
            for(int index = 0; index < 6; index++)
            {
               int offset = (index * 2) + 5;
               draw_rectangle(destination, x+2, y+offset, w-4, 1, DEBUG_COLOR_BLACK);
            }

            rectangle close = get_close_button_rect(window);
            draw_rectangle_rect(destination, close, DEBUG_COLOR_WHITE);
            draw_outline_rect(destination, resize_rectangle(close, 1), DEBUG_COLOR_BLACK);
            draw_outline_rect(destination, resize_rectangle(close, 2), DEBUG_COLOR_WHITE);

            rectangle maximize = get_maximize_button_rect(window);
            draw_rectangle_rect(destination, maximize, DEBUG_COLOR_WHITE);
            draw_outline_rect(destination, resize_rectangle(maximize, 1), DEBUG_COLOR_BLACK);
            draw_outline_rect(destination, resize_rectangle(maximize, 2), DEBUG_COLOR_WHITE);
         }

         rectangle rect;
         get_text_bounds(&rect, window->title);

         int textx = x + w/2 - rect.width/2;
         int texty = ALIGN_TEXT_VERTICALLY(y+1, h);

         draw_rectangle(destination, textx-4, texty-1, rect.width+8, rect.height+2, DEBUG_COLOR_WHITE);
         draw_text(destination, textx, texty, window->title);
      }
   }
}

function void get_default_window_location(desktop_context *desktop, s32 *posx, s32 *posy)
{
   static s32 x = 120;
   static s32 y = 80;

   *posx = x;
   *posy = y;

   x -= 32/2;
   y += 20/2;

   x %= desktop->backbuffer.width;
   y %= desktop->backbuffer.height;
}

function void remove_window_from_list(desktop_context *desktop, desktop_window *window)
{
   // NOTE: Patch up list at removal site.
   if(window->prev)
   {
      window->prev->next = window->next;
   }
   if(window->next)
   {
      window->next->prev = window->prev;
   }

   // NOTE: Update first and last window in desktop state.
   if(desktop->first_window == window)
   {
      desktop->first_window = window->next;
   }
   if(desktop->last_window == window)
   {
      desktop->last_window = window->prev;
   }
}

function void raise_window(desktop_context *desktop, desktop_window *window)
{
   remove_window_from_list(desktop, window);

   // if(!desktop->first_window)
   // {
   //    desktop->first_window = window;
   // }
   // else
   // {
   //    window->next = desktop->first_window;
   //    desktop->first_window->prev = window;
   //    desktop->first_window = window;
   // }

   window->next = desktop->first_window;
   if(window->next)
   {
      window->next->prev = window;
   }
   window->prev = 0;
   desktop->first_window = window;

   if(!desktop->last_window)
   {
      desktop->last_window = window;
   }

   // if(!desktop->config.focus_follows_mouse)
   {
      desktop->hot_window = window;
   }
}

function void minimize_window(desktop_context *desktop, desktop_window *window)
{
   window->state = WINDOW_STATE_MINIMIZED;

   if(window == desktop->active_window)
   {
      desktop->active_window = 0;
      for(desktop_window *test = desktop->first_window; test; test = test->next)
      {
         if(is_window_visible(test))
         {
            desktop->active_window = test;
            break;
         }
      }
   }
}

function void create_window_position(desktop_context *desktop, string8 title, s32 x, s32 y)
{
   desktop_window *window = 0;
   if(desktop->free_window)
   {
      window = desktop->free_window;
      desktop->free_window = desktop->free_window->next;
   }
   else
   {
      window = arena_allocate(&desktop->window_arena, desktop_window, 1);
   }

   desktop_window cleared_window = {0};
   *window = cleared_window;
   window->state = WINDOW_STATE_NORMAL;
   window->title = title;

   window->x = x;
   window->y = y;
   window->width = 400;
   window->height = 300;

   // BUG: Decouple texture creation from window creation. Right now texture
   // memory does not get reused after windows are recreated.
   texture canvas = {0};
   canvas.width = window->width;
   canvas.height = window->height;
   canvas.memory = arena_allocate(&desktop->texture_arena, u32, canvas.width*canvas.height);
   window->canvas = canvas;

   raise_window(desktop, window);
}

function void create_window(desktop_context *desktop, string8 title)
{
   s32 posx, posy;
   get_default_window_location(desktop, &posx, &posy);
   create_window_position(desktop, title, posx, posy);
}

function desktop_window *close_window(desktop_context *desktop, desktop_window *window)
{
   desktop_window *result = window->prev;

   remove_window_from_list(desktop, window);

   // NOTE: Add the closed window to the front of the free list.
   window->prev = 0;
   window->next = desktop->free_window;
   desktop->free_window = window;

   return(result);
}

function bool in_visible_window(desktop_context *desktop, desktop_window *window, int x, int y)
{
   bool result = false;
   if(in_rectangle(window->bounds, x, y))
   {
      for(desktop_window *test = desktop->first_window; test; test = test->next)
      {
         if(window == test)
         {
            result = true;
            break;
         }
         else if(in_rectangle(test->bounds, x, y))
         {
            break;
         }
      }
   }

   return(result);
}

function void store_active_window_mouse_offset(desktop_context *desktop, desktop_window *window, int x, int y)
{
   if(window)
   {
      desktop->active_window_mouse_offsetx = x - window->x;
      desktop->active_window_mouse_offsety = y - window->y;
   }
   else
   {
      desktop->active_window_mouse_offsetx = 0;
      desktop->active_window_mouse_offsetx = 0;
   }
}

function bool window_wants_interaction(desktop_context *desktop, desktop_window *window)
{
   bool result = false;

   input_state left = desktop->input.keys[INPUT_KEY_MBLEFT];
   bool inside = in_visible_window(desktop, window, desktop->input.mousex, desktop->input.mousey);

   if(window == desktop->active_window)
   {
      if(was_released(left))
      {
         desktop->active_window = 0;
      }
      else
      {
         result = true;
      }
   }
   else if(window == desktop->hot_window)
   {
      if(was_pressed(left) && inside)
      {
         desktop->active_window = window;
         result = true;
      }
   }

   if(inside)
   {
      desktop->hot_window = window;
   }

   return(result);
}

function void interact_with_window(desktop_context *desktop, desktop_window *window)
{
   raise_window(desktop, window);

   input_state left = desktop->input.keys[INPUT_KEY_MBLEFT];

   int mousex = desktop->input.mousex;
   int mousey = desktop->input.mousey;

   if(was_pressed(left))
   {
      store_active_window_mouse_offset(desktop, window, mousex, mousey);

      if(in_rectangle(get_close_button_rect(window), mousex, mousey))
      {
         window->state = WINDOW_STATE_CLOSED;
         desktop->active_window = 0;
      }
      else if(in_rectangle(get_maximize_button_rect(window), mousex, mousey))
      {
         if(window->state == WINDOW_STATE_NORMAL)
         {
            window->state = WINDOW_STATE_MAXIMIZED;
            window->unmaximized = window->bounds;

            // TODO: Stop hard-coding offsets here.
            window->x = 0;
            window->y = DESKTOP_WINDOW_DIM_TITLEBAR;
            window->width = desktop->backbuffer.width;
            window->height = desktop->backbuffer.height - window->y;
         }
         else
         {
            window->state = WINDOW_STATE_NORMAL;
            window->bounds = window->unmaximized;
         }

         desktop->active_window = 0;
      }
      else if(in_rectangle(get_titlebar_rect(window), mousex, mousey))
      {
         if(window->state == WINDOW_STATE_MAXIMIZED)
         {
            window->state = WINDOW_STATE_NORMAL;
            window->bounds = window->unmaximized;

            store_active_window_mouse_offset(desktop, window, window->width/2, DESKTOP_WINDOW_HALFDIM_TITLEBAR);
         }
      }
   }

   if(desktop->active_window)
   {
      if(in_rectangle(get_titlebar_rect(window), desktop->input.previous_mousex, desktop->input.previous_mousey))
      {
         window->x = mousex - desktop->active_window_mouse_offsetx;
         window->y = mousey - desktop->active_window_mouse_offsety;
      }
   }

#if 0
   // TODO: This resize logic correctly maps the mouse to the window in
   // terms of positioning, but does not correctly account for the minimum
   // window size - it causes the window to move instead.

   int deltax = (desktop->input.mousex - desktop->input.previous_mousex);
   int deltay = (desktop->input.mousey - desktop->input.previous_mousey);

   switch(region_invariants[desktop->hot_region_index].interaction)
   {
      case WINDOW_INTERACTION_NONE:
      {
         assert(!"Interacting with window without interaction type.");
      } break;

      case WINDOW_INTERACTION_RAISE:
      {
         // NOTE: The window is raised for all interactions, so no need to do
         // anything here.
      } break;

      case WINDOW_INTERACTION_MOVE:
      {
         if(window->state == WINDOW_STATE_MAXIMIZED)
         {
            window->state = WINDOW_STATE_NORMAL;
            // window->content = window->unmaximized;

            window->x = input->mousex - (window->width / 2);
            window->y = input->mousey + DESKTOP_WINDOW_HALFDIM_TITLEBAR;
         }

         window->x += deltax;
         window->y += deltay;
      } break;

      case WINDOW_INTERACTION_CLOSE:
      {
         if(was_released(input->keys[INPUT_KEY_MBLEFT]))
         {
            window->state = WINDOW_STATE_CLOSED;
         }
      } break;

      case WINDOW_INTERACTION_MAXIMIZE:
      {
         if(was_released(input->keys[INPUT_KEY_MBLEFT]))
         {
            if(window->state == WINDOW_STATE_NORMAL)
            {
               window->state = WINDOW_STATE_MAXIMIZED;
               // window->unmaximized = window->content;

               // TODO: Stop hard-coding offsets here.
               window->x = DESKTOP_WINDOW_DIM_EDGE;
               window->y = DESKTOP_WINDOW_DIM_EDGE + DESKTOP_WINDOW_DIM_TITLEBAR;
               window->width = desktop->backbuffer.width - (2 * DESKTOP_WINDOW_DIM_EDGE);
               window->height = desktop->backbuffer.height - window->y - DESKTOP_TASKBAR_HEIGHT - DESKTOP_WINDOW_DIM_EDGE;

            }
            else
            {
               window->state = WINDOW_STATE_NORMAL;
               // window->content = window->unmaximized;
            }
         }
      } break;

      case WINDOW_INTERACTION_MINIMIZE:
      {
         if(was_released(input->keys[INPUT_KEY_MBLEFT]))
         {
            if(window->state != WINDOW_STATE_MINIMIZED)
            {
               minimize_window(desktop, window);
            }
            else
            {
               window->state = WINDOW_STATE_NORMAL;
            }
         }
      } break;

      case WINDOW_INTERACTION_RESIZE_N:
      {
         window->y += deltay;
         window->height -= deltay;
      } break;

      case WINDOW_INTERACTION_RESIZE_S:
      {
         window->height += deltay;
      } break;

      case WINDOW_INTERACTION_RESIZE_W:
      {
         window->x += deltax;
         window->width -= deltax;
      } break;

      case WINDOW_INTERACTION_RESIZE_E:
      {
         window->width += deltax;
      } break;

      case WINDOW_INTERACTION_RESIZE_NW:
      {
         window->width -= deltax;
         window->height -= deltay;

         window->x += deltax;
         window->y += deltay;
      } break;

      case WINDOW_INTERACTION_RESIZE_NE:
      {
         window->y += deltay;
         window->width += deltax;
         window->height -= deltay;
      } break;

      case WINDOW_INTERACTION_RESIZE_SW:
      {
         window->x += deltax;
         window->width -= deltax;
         window->height += deltay;
      } break;

      case WINDOW_INTERACTION_RESIZE_SE:
      {
         window->width += deltax;
         window->height += deltay;
      } break;

      default:
      {
         assert(!"Invalid default case.");
      } break;
   }
#endif
}

function void draw_debug_overlay(texture *destination, desktop_input *input)
{
   char overlay_text[32];

   s32 x = destination->width - (FONT_WIDTH * FONT_SCALE * sizeof(overlay_text));
   s32 y = 30;

   draw_text_line(destination, x, &y, string8("DEBUG INFORMATION"));
   draw_text_line(destination, x, &y, string8("-----------------"));

#if(SIMD_WIDTH == 8)
   draw_text_line(destination, x, &y, string8("SIMD target: AVX2"));
#elif(SIMD_WIDTH == 4)
   draw_text_line(destination, x, &y, string8("SIMD target: SSE2"));
#else
   draw_text_line(destination, x, &y, string8("SIMD target: NONE"));
#endif

   float frame_ms = input->frame_seconds_elapsed * 1000.0f;
   float target_ms = input->target_seconds_per_frame * 1000.0f;

   u32 sleep_ms = input->sleep_ms;
   float frame_utilization = ((frame_ms - sleep_ms) / target_ms * 100.0f);

   int length = sprintf(overlay_text, "Frame time:  %.04fms\n", frame_ms);
   draw_text_line(destination, x, &y, string8new((u8 *)overlay_text, length));

   length = sprintf(overlay_text, "Target time: %.04fms\n", target_ms);
   draw_text_line(destination, x, &y, string8new((u8 *)overlay_text, length));

   length = sprintf(overlay_text, "Sleep time:  %ums\n", sleep_ms);
   draw_text_line(destination, x, &y, string8new((u8 *)overlay_text, length));

   length = sprintf(overlay_text, "Work time:  %.2f%%\n", frame_utilization);
   draw_text_line(destination, x, &y, string8new((u8 *)overlay_text, length));

   length = sprintf(overlay_text, "Cursor Position: %d, %d\n", input->mousex, input->mousey);
   draw_text_line(destination, x, &y, string8new((u8 *)overlay_text, length));
}

DESKTOP_INITIALIZE(desktop_initialize)
{
   // TODO: It would be nice if the desktop environment had no dynamic
   // allocations. Get rid of the stdlib heap allocations.
   memindex size = KILOBYTES(64);
   arena_initialize(&desktop->window_arena, calloc(1, size), size);

   size = MEGABYTES(256);
   arena_initialize(&desktop->texture_arena, calloc(1, size), size);

   size = KILOBYTES(64);
   arena_initialize(&desktop->scratch_arena, calloc(1, size), size);

   desktop->backbuffer.width = width;
   desktop->backbuffer.height = height;
   desktop->backbuffer.memory = arena_allocate(&desktop->texture_arena, u32, width*height);

   create_window(desktop, string8("Test Window 0"));
   create_window(desktop, string8("Test Window 1"));
   create_window(desktop, string8("Test Window 2"));
   create_window(desktop, string8("Test Window 3"));
   create_window(desktop, string8("Test Window 4"));

   desktop->hot_window = 0;
   // desktop->hot_region_index = DESKTOP_REGION_NULL_INDEX;

   desktop->cursor_textures[CURSOR_ARROW]         = load_bitmap(desktop, "cursor_arrow.bmp", 0, 0);
   desktop->cursor_textures[CURSOR_MOVE]          = load_bitmap(desktop, "cursor_move.bmp", 8, 8);
   desktop->cursor_textures[CURSOR_RESIZE_VERT]   = load_bitmap(desktop, "cursor_vertical_resize.bmp", 4, 8);
   desktop->cursor_textures[CURSOR_RESIZE_HORI]   = load_bitmap(desktop, "cursor_horizontal_resize.bmp", 8, 4);
   desktop->cursor_textures[CURSOR_RESIZE_DIAG_L] = load_bitmap(desktop, "cursor_diagonal_left.bmp", 7, 7);
   desktop->cursor_textures[CURSOR_RESIZE_DIAG_R] = load_bitmap(desktop, "cursor_diagonal_right.bmp", 7, 7);

   desktop->region_textures[WINDOW_REGION_BUTTON_CLOSE]    = load_bitmap(desktop, "close.bmp", 0, 0);
   desktop->region_textures[WINDOW_REGION_BUTTON_MAXIMIZE] = load_bitmap(desktop, "maximize.bmp", 0, 0);
   desktop->region_textures[WINDOW_REGION_BUTTON_MINIMIZE] = load_bitmap(desktop, "minimize.bmp", 0, 0);

   initialize_font();

   // desktop->config.focus_follows_mouse = true;

   desktop->is_initialized = true;
}

DESKTOP_UPDATE(desktop_update)
{
   desktop_input *input = &desktop->input;

   if(was_pressed(input->keys[INPUT_KEY_MBRIGHT]))
   {
      create_window_position(desktop, string8("New Window"), input->mousex, input->mousey);
   }

   desktop->frame_cursor = CURSOR_ARROW;

   // NOTE: Handle window interactions.
   for(desktop_window *window = desktop->first_window; window; window = window->next)
   {
      if(window_wants_interaction(desktop, window))
      {
         interact_with_window(desktop, window);
      }
   }

   // NOTE: Defer "closing" the windows until after interactions are complete,
   // so that shuffling the list doesn't impact the loop.
   for(desktop_window *window = desktop->first_window; window; window = window->next)
   {
      if(window->state == WINDOW_STATE_CLOSED)
      {
         window = close_window(desktop, window);

         desktop->active_window = 0;
         desktop->hot_window = 0;

         if(!window)
         {
            break;
         }
      }
   }

   // NOTE: Don't let other windows grab focus when dragging a window around,
   // always give precedence to the active window.
   if(desktop->active_window && is_window_visible(desktop->active_window))
   {
      desktop->hot_window = desktop->active_window;
   }

   // NOTE: Draw desktop.
   draw_rectangle_25(&desktop->backbuffer, 0, 0, desktop->backbuffer.width, desktop->backbuffer.height);
   // draw_debug_overlay(&desktop->backbuffer, &input);

   // NOTE: Draw windows and their regions in reverse order, so that the earlier
   // elements in the list appear on top.
   for(desktop_window *window = desktop->last_window; window; window = window->prev)
   {
      draw_window(desktop, window, &desktop->backbuffer);
   }

   // NOTE: Draw desktop menu bar.
   rectangle taskbar = create_rectangle(0, 0, desktop->backbuffer.width, DESKTOP_TASKBAR_HEIGHT);
   draw_rectangle_rect(&desktop->backbuffer, taskbar, DEBUG_COLOR_WHITE);
   draw_rectangle(&desktop->backbuffer, 0, taskbar.height, desktop->backbuffer.width, 1, DEBUG_COLOR_BLACK);

   string8 menu_items[] = {
      string8("Exo"),
      string8("::"),
      string8("File"),
      string8("Edit"),
      string8("View"),
   };

   int menu_item_padding = 16;
   int menu_itemx = menu_item_padding;

   for(int index = 0; index < countof(menu_items); ++index)
   {
      rectangle rect;
      string8 text = menu_items[index];
      get_text_bounds(&rect, text);

      int menu_itemy = ALIGN_TEXT_VERTICALLY(0, DESKTOP_TASKBAR_HEIGHT);

      draw_text(&desktop->backbuffer, menu_itemx, menu_itemy, text);
      menu_itemx += rect.width + menu_item_padding;
   }

   // NOTE: Draw cursor.
   texture *cursor_texture = desktop->cursor_textures + desktop->frame_cursor;
   draw_texture(&desktop->backbuffer, cursor_texture, input->mousex, input->mousey);
}
