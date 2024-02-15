/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#include "graphics.h"

function bool is_pressed(input_state button)
{
   // NOTE(law): Check if the button is currently being pressed this frame,
   // regardless of what frame it was initially pressed.
   bool result = button.is_pressed;
   return(result);
}

function bool was_pressed(input_state button)
{
   // NOTE(law): Check if the button transitioned to being pressed on the
   // current frame.
   bool result = button.is_pressed && button.changed_state;
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

function void draw_rectangle(exo_texture *backbuffer, rectangle rect, v4 color)
{
   draw_rectangle(backbuffer, rect.x, rect.y, rect.width, rect.height, color);
}

function void draw_bitmap(exo_texture *backbuffer, exo_texture *bitmap, s32 posx, s32 posy)
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

function void get_default_window_location(s32 *posx, s32 *posy)
{
   s32 initial_x = 50;
   s32 initial_y = 50;

   static s32 x = initial_x;
   static s32 y = initial_y;

   *posx = x;
   *posy = y;

   x += 32;
   y += 20;

   x %= EXO_SCREEN_RESOLUTION_X;
   y %= EXO_SCREEN_RESOLUTION_Y;
}

function void compute_window_regions(exo_window *window, s32 x, s32 y, s32 w, s32 h)
{
   s32 e = EXO_WINDOW_EDGE_DIM;
   s32 t = EXO_WINDOW_TITLEBAR_DIM;
   s32 c = EXO_WINDOW_CORNER_DIM;
   s32 c2 = c / 2;

   window_region *regions = window->regions;
   regions[WINDOW_REGION_CONTENT].bounds   = create_rectangle(x, y, w, h);
   regions[WINDOW_REGION_TITLEBAR].bounds  = create_rectangle(x, y - t, w, t);

   regions[WINDOW_REGION_BORDER_N].bounds  = create_rectangle(x, y - t - e, w, e);
   regions[WINDOW_REGION_BORDER_S].bounds  = create_rectangle(x, y + h, w, e);
   regions[WINDOW_REGION_BORDER_W].bounds  = create_rectangle(x - e, y - t, e, h + t);
   regions[WINDOW_REGION_BORDER_E].bounds  = create_rectangle(x + w, y - t, e, h + t);

   regions[WINDOW_REGION_CORNER_NW].bounds = create_rectangle(x - c2, y - t - c2, c, c);
   regions[WINDOW_REGION_CORNER_NE].bounds = create_rectangle(x + w - c2, y - t - c2, c, c);
   regions[WINDOW_REGION_CORNER_SW].bounds = create_rectangle(x - c2, y + h - c2, c, c);
   regions[WINDOW_REGION_CORNER_SE].bounds = create_rectangle(x + w - c2, y + h - c2, c, c);
}

function void compute_window_regions(exo_window *window, rectangle bounds)
{
   compute_window_regions(window, bounds.x, bounds.y, bounds.width, bounds.height);
}

function int compare_window_sort_entries(void const *ap, void const *bp)
{
   window_sort_entry *a = (window_sort_entry *)ap;
   window_sort_entry *b = (window_sort_entry *)bp;

   if(a->z > b->z) return(-1);
   if(a->z < b->z) return(1);
   return(0);
}

function void sort_windows(exo_state *es)
{
   u32 entry_count = 0;
   for(u32 index = 0; index < EXO_WINDOW_MAX_COUNT; ++index)
   {
      exo_window *window = es->windows + index;
      if(window->state != WINDOW_STATE_CLOSED)
      {
         window_sort_entry *entry = es->window_order + entry_count++;
         entry->index = index;
         entry->z = window->z;
      }
   }

   // TODO(law): Replace with our own stable sort.
   qsort(es->window_order, entry_count, sizeof(es->window_order[0]), compare_window_sort_entries);

   // Remove any gaps/duplicates in the z values.
   for(u32 window_order_index = 0; window_order_index < entry_count; ++window_order_index)
   {
      s32 z = entry_count - window_order_index - 1;

      window_sort_entry *entry = es->window_order + window_order_index;
      entry->z = z;
      es->windows[entry->index].z = z;
   }
}

function void focus_window(exo_state *es, exo_window *window)
{
   window->z = es->window_count;
   sort_windows(es);
}

function void create_window(exo_state *es, s32 posx, s32 posy, s32 width, s32 height)
{
   assert(es->window_count < (EXO_WINDOW_MAX_COUNT - 1));

   // TODO(law): Probe the array for previously closed windows so that their
   // slots can be recycled.
   exo_window *window = es->windows + es->window_count++;
   window->state = WINDOW_STATE_NORMAL;

   focus_window(es, window);

   compute_window_regions(window, posx, posy, width, height);
}

function void create_window(exo_state *es, s32 width, s32 height)
{
   s32 posx;
   s32 posy;
   get_default_window_location(&posx, &posy);
   create_window(es, posx, posy, width, height);
}

bool exo_window::hit_test(exo_state *es, exo_input *input)
{
   if(state != WINDOW_STATE_CLOSED)
   {
      // Peform hit testing on each region of the window.
      for(u32 region_index = 0; region_index < WINDOW_REGION_COUNT; ++region_index)
      {
         window_region *region = regions + region_index;
         if(in_rectangle(region->bounds, input->mousex, input->mousey))
         {
            if(!es->frame_cursor_overriden)
            {
               es->frame_cursor_overriden = true;
               es->frame_cursor = window_region_invariants[region_index].cursor;
            }

            if(was_pressed(input->mouse_buttons[MOUSE_BUTTON_LEFT]))
            {
               if(!es->is_interacting)
               {
                  focus_window(es, this);
                  es->is_interacting = true;
                  interaction = window_region_invariants[region_index].interaction;
                  break;
               }
            }
         }
      }

      // If the window was previously being interacted with but the mouse
      // button is no longer pressed, clear the interaction state.
      if(interaction != WINDOW_INTERACTION_NONE && !is_pressed(input->mouse_buttons[MOUSE_BUTTON_LEFT]))
      {
         es->is_interacting = false;
         interaction = WINDOW_INTERACTION_NONE;
      }
   }

   bool result = (es->is_interacting && interaction != WINDOW_INTERACTION_NONE);
   return(result);
}

void exo_window::interact(exo_state *es, exo_input *input)
{
   window_region content = regions[WINDOW_REGION_CONTENT];
   switch(interaction)
   {
      case WINDOW_INTERACTION_NONE:
      {
         assert(!"Interacting with window without interaction type.");
      } break;

      case WINDOW_INTERACTION_FOCUS:
      {
         focus_window(es, this);
      } break;

      case WINDOW_INTERACTION_MOVE:
      {
         es->frame_cursor = CURSOR_MOVE;

         content.posx += (input->mousex - input->previous_mousex);
         content.posy += (input->mousey - input->previous_mousey);
      } break;

      case WINDOW_INTERACTION_RESIZE_N:
      {
         es->frame_cursor = CURSOR_RESIZE_VERT;

         s32 delta = input->mousey - input->previous_mousey;
         content.posy += delta;
         content.height -= delta;
      } break;

      case WINDOW_INTERACTION_RESIZE_S:
      {
         es->frame_cursor = CURSOR_RESIZE_VERT;

         s32 delta = input->mousey - input->previous_mousey;
         content.height += delta;
      } break;

      case WINDOW_INTERACTION_RESIZE_W:
      {
         es->frame_cursor = CURSOR_RESIZE_HORI;

         s32 delta = input->mousex - input->previous_mousex;
         content.posx += delta;
         content.width -= delta;
      } break;

      case WINDOW_INTERACTION_RESIZE_E:
      {
         es->frame_cursor = CURSOR_RESIZE_HORI;

         s32 delta = input->mousex - input->previous_mousex;
         content.width += delta;
      } break;

      case WINDOW_INTERACTION_RESIZE_NW:
      {
         es->frame_cursor = CURSOR_RESIZE_DIAG_L;

         s32 deltax = input->mousex - input->previous_mousex;
         s32 deltay = input->mousey - input->previous_mousey;
         content.posx += deltax;
         content.posy += deltay;
         content.width -= deltax;
         content.height -= deltay;
      } break;

      case WINDOW_INTERACTION_RESIZE_SW:
      {
         es->frame_cursor = CURSOR_RESIZE_DIAG_R;

         s32 deltax = input->mousex - input->previous_mousex;
         s32 deltay = input->mousey - input->previous_mousey;
         content.posx += deltax;
         content.width -= deltax;
         content.height += deltay;
      } break;

      case WINDOW_INTERACTION_RESIZE_NE:
      {
         es->frame_cursor = CURSOR_RESIZE_DIAG_R;

         s32 deltax = input->mousex - input->previous_mousex;
         s32 deltay = input->mousey - input->previous_mousey;
         content.posy += deltay;
         content.width += deltax;
         content.height -= deltay;
      } break;

      case WINDOW_INTERACTION_RESIZE_SE:
      {
         es->frame_cursor = CURSOR_RESIZE_DIAG_L;

         s32 deltax = input->mousex - input->previous_mousex;
         s32 deltay = input->mousey - input->previous_mousey;
         content.width += deltax;
         content.height += deltay;
      } break;
   }

   compute_window_regions(this, content.bounds);
}

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

   bitmap_header *header = (bitmap_header *)memory;

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

void update(exo_texture *backbuffer, exo_input *input, exo_storage *storage)
{
   exo_state *es = (exo_state *)storage->memory;
   if(!es->is_initialized)
   {
      create_window(es, 300, 200);
      create_window(es, 300, 200);
      create_window(es, 300, 200);
      create_window(es, 300, 200);
      create_window(es, 300, 200);

      sort_windows(es);

      es->cursors[CURSOR_ARROW]         = load_bitmap("cursor_arrow.bmp");
      es->cursors[CURSOR_MOVE]          = load_bitmap("cursor_move.bmp", 8, 8);
      es->cursors[CURSOR_RESIZE_VERT]   = load_bitmap("cursor_vertical_resize.bmp", 4, 8);
      es->cursors[CURSOR_RESIZE_HORI]   = load_bitmap("cursor_horizontal_resize.bmp", 8, 4);
      es->cursors[CURSOR_RESIZE_DIAG_L] = load_bitmap("cursor_diagonal_left.bmp", 7, 7);
      es->cursors[CURSOR_RESIZE_DIAG_R] = load_bitmap("cursor_diagonal_right.bmp", 7, 7);

      es->is_initialized = true;
   }

   if(was_pressed(input->mouse_buttons[MOUSE_BUTTON_RIGHT]))
   {
      create_window(es, input->mousex, input->mousey, 300, 200);
   }

   es->frame_cursor_overriden = false;
   es->frame_cursor = CURSOR_ARROW;

   // Draw desktop.
   v4 background_color = {0.157f, 0.157f, 0.157f, 1.0f};
   draw_rectangle(backbuffer, 0, 0, backbuffer->width, backbuffer->height, background_color);

   // Handle window interactions.
   for(u32 index = 0; index < es->window_count; ++index)
   {
      exo_window *window = es->windows + es->window_order[index].index;
      if(window->hit_test(es, input))
      {
         window->interact(es, input);
      }
   }

   // Draw windows and their regions in reverse order, so that the earlier
   // elements in the list appear on top.
   for(s32 sort_index = es->window_count - 1; sort_index >= 0; --sort_index)
   {
      u32 window_index = es->window_order[sort_index].index;

      exo_window *window = es->windows + window_index;
      if(window->state != WINDOW_STATE_CLOSED)
      {
         for(s32 region_index = WINDOW_REGION_COUNT - 1; region_index >= 0; --region_index)
         {
            window_region *region = window->regions + region_index;
            v4 color = window_region_invariants[region_index].color;

            draw_rectangle(backbuffer, region->bounds, color);
         }
      }
   }

   // Draw cursor.
   draw_bitmap(backbuffer, es->cursors + es->frame_cursor, input->mousex, input->mousey);
}
