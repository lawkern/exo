/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

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

function texture load_bitmap(desktop_state *ds, char *file_path, u32 offsetx, u32 offsety)
{
   texture result = {0};
   result.offsetx = offsetx;
   result.offsety = offsety;

   FILE *file = fopen(file_path, "rb");
   assert(file);

   fseek(file, 0, SEEK_END);
   size_t size = ftell(file);
   fseek(file, 0, SEEK_SET);

   arena_marker marker = arena_marker_set(&ds->scratch_arena);
   u8 *memory = (u8 *)arena_allocate(&ds->scratch_arena, u8, size);

   size_t bytes_read = fread(memory, 1, size, file);
   assert(bytes_read == size);

   bitmap_header *header = (bitmap_header *)memory;

   assert(header->file_type == 0x4D42); // "BM"
   assert(header->bits_per_pixel == 32);

   result.width = header->width;
   result.height = header->height;
   result.memory = (u32 *)arena_allocate(&ds->texture_arena, u32, result.width * result.height);

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

function void compute_region_size(rectangle *result, desktop_window *window, window_region_type region)
{
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

function void draw_rectangle_rect(texture *backbuffer, rectangle rect, vec4 color)
{
   draw_rectangle(backbuffer, rect.x, rect.y, rect.width, rect.height, color);
}

function void draw_outline_rect(texture *destination, rectangle bounds, vec4 color)
{
   draw_outline(destination, bounds.x, bounds.y, bounds.width, bounds.height, color);
}

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
   texture *texture = &window->texture;

   rectangle bounds;
   compute_region_size(&bounds, window, WINDOW_REGION_CONTENT);
   draw_rectangle_rect(destination, bounds, PALETTE[4]);

   clear(texture, PALETTE[2]);

   s32 x = 3;
   s32 y = 6;

   char text_line[64];
   char *format = "{x:%d y:%d w:%d h:%d}";

   int length = sprintf(text_line, format, window->x, window->y, window->width, window->height);
   draw_text_line(texture, x, &y, string8new((u8 *)text_line, length));

   length = sprintf(text_line, format, bounds.x, bounds.y, bounds.width, bounds.height);
   draw_text_line(texture, x, &y, string8new((u8 *)text_line, length));

   length = sprintf(text_line, "state:%d", window->state);
   draw_text_line(texture, x, &y, string8new((u8 *)text_line, length));

   y = ADVANCE_TEXT_LINE(y);
   draw_text_line(texture, x, &y, string8("+----------------------------+"));
   draw_text_line(texture, x, &y, string8("| ASCII FONT TEST            |"));
   draw_text_line(texture, x, &y, string8("|----------------------------|"));
   draw_text_line(texture, x, &y, string8("| ABCDEFGHIJKLMNOPQRSTUVWXYZ |"));
   draw_text_line(texture, x, &y, string8("| abcdefghijklmnopqrstuvwxyz |"));
   draw_text_line(texture, x, &y, string8("| AaBbCcDdEeFfGgHhIiJjKkLlMm |"));
   draw_text_line(texture, x, &y, string8("| NnOoPpQqRrSsTtUuVvWwXxYyZz |"));
   draw_text_line(texture, x, &y, string8("| 0123456789!\"#$%&'()*+,-./: |"));
   draw_text_line(texture, x, &y, string8("| ;<=>?@[\\]^_`{|}~           |"));
   draw_text_line(texture, x, &y, string8("+----------------------------+"));

   draw_texture_bounded(destination, texture, bounds.x, bounds.y, bounds.width, bounds.height);
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

function bool is_window_visible(desktop_window *window)
{
   bool result = (window->state != WINDOW_STATE_CLOSED && window->state != WINDOW_STATE_MINIMIZED);
   return(result);
}

function void draw_window(desktop_state *ds, desktop_window *window, texture *destination)
{
   if(is_window_visible(window))
   {
      bool is_active_window = (window == ds->active_window);

      for(s32 region_index = WINDOW_REGION_COUNT - 1; region_index >= 0; --region_index)
      {
         rectangle bounds;
         compute_region_size(&bounds, window, (window_region_type)region_index);

         window_region_entry *invariants = region_invariants + region_index;

         texture *texture = ds->region_textures + region_index;
         if(texture->memory)
         {
            draw_texture(destination, texture, bounds.x, bounds.y);
         }
         else
         {
            assert(invariants->draw);
            invariants->draw(destination, window, is_active_window);
         }
      }

      rectangle bounds;
      compute_window_bounds(&bounds, window);
      draw_outline_rect(destination, bounds, PALETTE[3]);
   }
}

function void get_default_window_location(s32 *posx, s32 *posy)
{
   static s32 x = 50;
   static s32 y = 50;

   *posx = x;
   *posy = y;

   x += 32;
   y += 20;

   x %= DESKTOP_SCREEN_RESOLUTION_X;
   y %= DESKTOP_SCREEN_RESOLUTION_Y;
}

function void remove_window_from_list(desktop_state *ds, desktop_window *window)
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
   if(ds->first_window == window)
   {
      ds->first_window = window->next;
   }
   if(ds->last_window == window)
   {
      ds->last_window = window->prev;
   }
}

function void raise_window(desktop_state *ds, desktop_window *window)
{
   remove_window_from_list(ds, window);

   // if(!ds->first_window)
   // {
   //    ds->first_window = window;
   // }
   // else
   // {
   //    window->next = ds->first_window;
   //    ds->first_window->prev = window;
   //    ds->first_window = window;
   // }

   window->next = ds->first_window;
   if(window->next)
   {
      window->next->prev = window;
   }
   window->prev = 0;
   ds->first_window = window;

   if(!ds->last_window)
   {
      ds->last_window = window;
   }

   if(!ds->config.focus_follows_mouse)
   {
      ds->active_window = window;
   }
}

function void minimize_window(desktop_state *ds, desktop_window *window)
{
   window->state = WINDOW_STATE_MINIMIZED;

   if(window == ds->active_window)
   {
      ds->active_window = 0;
      for(desktop_window *test = ds->first_window; test; test = test->next)
      {
         if(is_window_visible(test))
         {
            ds->active_window = test;
            break;
         }
      }
   }
}

function void create_window_position(desktop_state *ds, string8 title, s32 x, s32 y)
{
   s32 width = 400;
   s32 height = 300;

   desktop_window *window = 0;
   if(ds->free_window)
   {
      window = ds->free_window;
      ds->free_window = ds->free_window->next;
   }
   else
   {
      window = arena_allocate(&ds->window_arena, desktop_window, 1);
   }

   desktop_window cleared_window = {0};
   *window = cleared_window;
   window->state = WINDOW_STATE_NORMAL;
   window->title = title;

   rectangle content = create_rectangle(x, y, width, height);
   window->content = content;

   // BUG: Decouple texture creation from window creation. Right now texture
   // memory does not get reused after windows are recreated.
   texture texture = {0};
   texture.width = content.width;
   texture.height = content.height;
   texture.memory = (u32 *)arena_allocate(&ds->texture_arena, u32, texture.width * texture.height);
   window->texture = texture;

   raise_window(ds, window);
}

function void create_window(desktop_state *ds, string8 title)
{
   s32 posx;
   s32 posy;
   get_default_window_location(&posx, &posy);
   create_window_position(ds, title, posx, posy);
}

function desktop_window *close_window(desktop_state *ds, desktop_window *window)
{
   desktop_window *result = window->prev;

   remove_window_from_list(ds, window);

   // NOTE: Add the closed window to the front of the free list.
   window->prev = 0;
   window->next = ds->free_window;
   ds->free_window = window;

   return(result);
}

function hit_result detect_window_hit(desktop_window *window, s32 x, s32 y)
{
   hit_result result = {DESKTOP_REGION_NULL_INDEX};

   if(is_window_visible(window))
   {
      // Peform hit testing on each region of the window.
      for(u32 region_index = 0; region_index < WINDOW_REGION_COUNT; ++region_index)
      {
         rectangle bounds;
         compute_region_size(&bounds, window, (window_region_type)region_index);

         if(in_rectangle(bounds, x, y))
         {
            result.region_index = region_index;
            break;
         }
      }
   }

   return(result);
}

function void interact_with_window(desktop_state *ds, desktop_window *window, desktop_input *input, hit_result hit)
{
   input_state left_click = input->mouse_buttons[MOUSE_BUTTON_LEFT];

   // If the window was previously being interacted with but the mouse button is
   // no longer pressed, clear the interaction state.
   if(ds->hot_window && !is_pressed(left_click) && !was_released(left_click))
   {
      ds->hot_window = 0;
      ds->hot_region_index = DESKTOP_REGION_NULL_INDEX;
   }

   // If a press/release occurred this frame and no hot interaction is currently
   // underway, update state to use the newly-hit window/region.
   if(left_click.changed_state)
   {
      if(!ds->hot_window)
      {
         ds->hot_window = window;
         ds->hot_region_index = hit.region_index;
      }
   }

   // Don't interact on release if outside the original region bounds
   // (e.g. clicking a button, then dragging the cursor away before releasing).
   if(was_released(left_click) && (ds->hot_window != window || ds->hot_region_index != hit.region_index))
   {
      ds->hot_window = 0;
      ds->hot_region_index = DESKTOP_REGION_NULL_INDEX;
   }

   // Set mouse_window_index regardless of other ongoing interactions.
   ds->mouse_window = window;

   // Default to using the appropriate hover cursor based on mouse position.
   if(hit.region_index != DESKTOP_REGION_NULL_INDEX)
   {
      ds->frame_cursor = region_invariants[hit.region_index].cursor;
   }

   if(ds->hot_window && ds->hot_region_index != DESKTOP_REGION_NULL_INDEX)
   {
      // Override the cursor if an ongoing interaction is underway.
      ds->frame_cursor = region_invariants[ds->hot_region_index].cursor;

      // All interactions result in raising the window.
      if(was_pressed(input->mouse_buttons[MOUSE_BUTTON_LEFT]))
      {
         raise_window(ds, window);
      }

      // TODO: This resize logic correctly maps the mouse to the window in
      // terms of positioning, but does not correctly account for the minimum
      // window size - it causes the window to move instead.

      s32 deltax = (input->mousex - input->previous_mousex);
      s32 deltay = (input->mousey - input->previous_mousey);

      switch(region_invariants[ds->hot_region_index].interaction)
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
               window->content = window->unmaximized;

               window->content.x = input->mousex - (window->width / 2);
               window->content.y = input->mousey + DESKTOP_WINDOW_HALFDIM_TITLEBAR;
            }

            window->x += deltax;
            window->y += deltay;
         } break;

         case WINDOW_INTERACTION_CLOSE:
         {
            if(was_released(input->mouse_buttons[MOUSE_BUTTON_LEFT]))
            {
               window->state = WINDOW_STATE_CLOSED;
            }
         } break;

         case WINDOW_INTERACTION_MAXIMIZE:
         {
            if(was_released(input->mouse_buttons[MOUSE_BUTTON_LEFT]))
            {
               if(window->state == WINDOW_STATE_NORMAL)
               {
                  window->state = WINDOW_STATE_MAXIMIZED;
                  window->unmaximized = window->content;

                  // TODO: Stop hard-coding offsets here.
                  window->x = DESKTOP_WINDOW_DIM_EDGE;
                  window->y = DESKTOP_WINDOW_DIM_EDGE + DESKTOP_WINDOW_DIM_TITLEBAR;
                  window->width = DESKTOP_SCREEN_RESOLUTION_X - (2 * DESKTOP_WINDOW_DIM_EDGE);
                  window->height = DESKTOP_SCREEN_RESOLUTION_Y - window->y - DESKTOP_TASKBAR_HEIGHT - DESKTOP_WINDOW_DIM_EDGE;

               }
               else
               {
                  window->state = WINDOW_STATE_NORMAL;
                  window->content = window->unmaximized;
               }
            }
         } break;

         case WINDOW_INTERACTION_MINIMIZE:
         {
            if(was_released(input->mouse_buttons[MOUSE_BUTTON_LEFT]))
            {
               if(window->state != WINDOW_STATE_MINIMIZED)
               {
                  minimize_window(ds, window);
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
   }

   if(was_released(input->mouse_buttons[MOUSE_BUTTON_LEFT]))
   {
      ds->hot_window = 0;
      ds->hot_region_index = DESKTOP_REGION_NULL_INDEX;

      // TODO: It would be nicer if we didn't allow the internal window size to
      // fall below the specified minimum. We fix it here when click events end,
      // but it leaves window.width and window.height unsafe to use in the
      // middle of a resizing (before compute_region_size is called).
      compute_region_size(&window->content, window, WINDOW_REGION_CONTENT);
   }
}

function void draw_debug_overlay(texture *destination, desktop_input *input)
{
   char overlay_text[32];

   s32 x = destination->width - (FONT_WIDTH * FONT_SCALE * sizeof(overlay_text));
   s32 y = 10;

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

function void draw_1bit_window(texture *destination, int x, int y, int window_width, int window_height)
{
   window_width = MAXIMUM(window_width, DESKTOP_WINDOW_MIN_WIDTH);
   window_height = MAXIMUM(window_height, DESKTOP_WINDOW_MIN_HEIGHT);

   vec4 white = {1, 1, 1, 1};
   vec4 black = {0, 0, 0, 1};

   draw_outline(destination, x, y, window_width, window_height, black);
   draw_rectangle(destination, x+1, y+1, window_width-2, window_height-2, white);

   // NOTE: Draw titlebar.
   {
      int w = window_width;
      int h = 21;

      draw_rectangle(destination, x+1, y+h-1, w-2, 1, black);
      draw_rectangle(destination, x+1, y+h+1, w-2, 1, black);
      for(int index = 0; index < 6; index++)
      {
         int offset = (index * 2) + 5;
         draw_rectangle(destination, x+2, y+offset, w-4, 1, black);
      }

      draw_rectangle(destination, x+11, y+6, 9, 9, white);
      draw_outline(destination,   x+10, y+5, 11, 11, black);
      draw_outline(destination,   x+9, y+4, 13, 13, white);

      draw_rectangle(destination, x+w-20, y+6, 9, 9, white);
      draw_outline(destination,   x+w-21, y+5, 11, 11, black);
      draw_outline(destination,   x+w-22, y+4, 13, 13, white);

      rectangle rect;
      string8 text = string8("Emacs");
      get_text_bounds(&rect, text);

      int textx = x + w/2 - rect.width/2;
      int texty = ALIGN_TEXT_VERTICALLY(y+1, h);

      draw_rectangle(destination, textx-4, texty-1, rect.width+8, rect.height+2, white);
      draw_text(destination, textx, texty, text);
   }
}

DESKTOP_UPDATE(desktop_update)
{
   desktop_state *ds = (desktop_state *)storage->memory;
   if(!ds->is_initialized)
   {
      u8 *base = storage->memory + sizeof(*ds);

      arena_initialize(&ds->window_arena, base, KILOBYTES(64));
      base += ds->window_arena.cap;

      arena_initialize(&ds->texture_arena, base, MEGABYTES(256));
      base += ds->texture_arena.cap;

      arena_initialize(&ds->scratch_arena, base, KILOBYTES(64));
      base += ds->scratch_arena.cap;

      create_window(ds, string8("Test Window 0"));
      create_window(ds, string8("Test Window 1"));
      create_window(ds, string8("Test Window 2"));
      create_window(ds, string8("Test Window 3"));
      create_window(ds, string8("Test Window 4"));

      ds->hot_window = 0;
      ds->hot_region_index = DESKTOP_REGION_NULL_INDEX;

      ds->cursor_textures[CURSOR_ARROW]         = load_bitmap(ds, "cursor_arrow.bmp", 0, 0);
      ds->cursor_textures[CURSOR_MOVE]          = load_bitmap(ds, "cursor_move.bmp", 8, 8);
      ds->cursor_textures[CURSOR_RESIZE_VERT]   = load_bitmap(ds, "cursor_vertical_resize.bmp", 4, 8);
      ds->cursor_textures[CURSOR_RESIZE_HORI]   = load_bitmap(ds, "cursor_horizontal_resize.bmp", 8, 4);
      ds->cursor_textures[CURSOR_RESIZE_DIAG_L] = load_bitmap(ds, "cursor_diagonal_left.bmp", 7, 7);
      ds->cursor_textures[CURSOR_RESIZE_DIAG_R] = load_bitmap(ds, "cursor_diagonal_right.bmp", 7, 7);

      ds->region_textures[WINDOW_REGION_BUTTON_CLOSE]    = load_bitmap(ds, "close.bmp", 0, 0);
      ds->region_textures[WINDOW_REGION_BUTTON_MAXIMIZE] = load_bitmap(ds, "maximize.bmp", 0, 0);
      ds->region_textures[WINDOW_REGION_BUTTON_MINIMIZE] = load_bitmap(ds, "minimize.bmp", 0, 0);

      initialize_font();

      // ds->config.focus_follows_mouse = true;

      ds->is_initialized = true;
   }

   if(was_pressed(input->mouse_buttons[MOUSE_BUTTON_RIGHT]))
   {
      create_window_position(ds, string8("New Window"), input->mousex, input->mousey);
   }

   ds->frame_cursor = CURSOR_ARROW;
   ds->mouse_window = 0;

   // NOTE: Handle window interactions.
   for(desktop_window *window = ds->first_window; window; window = window->next)
   {
      hit_result hit = detect_window_hit(window, input->mousex, input->mousey);
      if(hit.region_index != DESKTOP_REGION_NULL_INDEX || ds->hot_region_index != DESKTOP_REGION_NULL_INDEX)
      {
         interact_with_window(ds, window, input, hit);
         break;
      }
   }

   // NOTE: Defer "closing" the windows until after interactions are complete,
   // so that shuffling the list doesn't impact the loop.
   for(desktop_window *window = ds->first_window; window; window = window->next)
   {
      if(window->state == WINDOW_STATE_CLOSED)
      {
         window = close_window(ds, window);

         ds->active_window = 0;
         ds->hot_window = 0;
         ds->hot_region_index = DESKTOP_REGION_NULL_INDEX;

         if(!window)
         {
            break;
         }
      }
   }

   // NOTE: Don't let other windows grab focus when dragging a window around,
   // always give precedence to the hot window.
   if(ds->hot_window && is_window_visible(ds->hot_window))
   {
      ds->active_window = ds->hot_window;
   }
   else if(ds->config.focus_follows_mouse)
   {
      ds->active_window = ds->mouse_window;
   }

   // NOTE: Draw desktop.
   clear(backbuffer, PALETTE[3]);
   draw_debug_overlay(backbuffer, input);

   // NOTE: Draw windows and their regions in reverse order, so that the earlier
   // elements in the list appear on top.
   for(desktop_window *window = ds->last_window; window; window = window->prev)
   {
      draw_window(ds, window, backbuffer);
   }

   // NOTE: Draw taskbar.
   rectangle taskbar = create_rectangle(0, backbuffer->height - DESKTOP_TASKBAR_HEIGHT, backbuffer->width, DESKTOP_TASKBAR_HEIGHT);
   draw_rectangle_rect(backbuffer, taskbar, PALETTE[1]);
   draw_rectangle(backbuffer, taskbar.x, taskbar.y, taskbar.width, 2, PALETTE[0]);

   s32 gap = 4;
   rectangle tab = create_rectangle(taskbar.x + gap, taskbar.y + gap, DESKTOP_WINDOWTAB_WIDTH_MAX, taskbar.height - (2 * gap));

   for(desktop_window *window = ds->first_window; window; window = window->next)
   {
      assert(window->state != WINDOW_STATE_CLOSED);

      vec4 color = PALETTE[2];
      if(window == ds->active_window)
      {
         color = DEBUG_COLOR_GREEN;
      }
      else if(window->state == WINDOW_STATE_MINIMIZED)
      {
         color = DEBUG_COLOR_BLUE;
      }

      if(in_rectangle(tab, input->mousex, input->mousey))
      {
         if(was_released(input->mouse_buttons[MOUSE_BUTTON_LEFT]))
         {
            if(window->state == WINDOW_STATE_MINIMIZED)
            {
               window->state = WINDOW_STATE_NORMAL;
               raise_window(ds, window);
            }
            else if(window != ds->active_window)
            {
               raise_window(ds, window);
            }
            else
            {
               minimize_window(ds, window);
            }
         }
      }

      draw_rectangle_rect(backbuffer, tab, color);

      s32 x = tab.x + 3;
      s32 y = ALIGN_TEXT_VERTICALLY(tab.y, tab.height);
      draw_text(backbuffer, x, y, window->title);

      tab.x += (tab.width + (2 * gap));
   }

   // NOTE: Draw test titlebar
   draw_1bit_window(backbuffer, 10, 10, 300, 200);

   // NOTE: Draw cursor.
   texture *cursor_texture = ds->cursor_textures + ds->frame_cursor;
   draw_texture(backbuffer, cursor_texture, input->mousex, input->mousey);
}
