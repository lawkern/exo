/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#include "graphics.h"

#include "simd.cpp"
#include "text.cpp"

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

function exo_texture load_bitmap(char *file_path, u32 offsetx = 0, u32 offsety = 0)
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

function void clear(exo_texture *backbuffer, v4 color)
{
   color *= 255.0f;
   u32 pixel = (((u32)(color.r + 0.5f) << 16) |
                ((u32)(color.g + 0.5f) << 8) |
                ((u32)(color.b + 0.5f) << 0) |
                ((u32)(color.a + 0.5f) << 24));

   u32 *memory = backbuffer->memory;
   for(s32 index = 0; index < (backbuffer->width * backbuffer->height); ++index)
   {
      memory[index] = pixel;
   }
}

function void draw_rectangle(exo_texture *backbuffer, s32 posx, s32 posy, s32 width, s32 height, v4 color)
{
   s32 target_width = backbuffer->width;
   s32 target_height = backbuffer->height;
   u32 *target_memory = backbuffer->memory;

   s32 minx = MAXIMUM(posx, 0);
   s32 miny = MAXIMUM(posy, 0);
   s32 maxx = MINIMUM(posx + width, target_width);
   s32 maxy = MINIMUM(posy + height, target_height);

   s32 runoff = (maxx - minx) % SIMD_WIDTH;
   s32 wide_maxx = MAXIMUM(minx, maxx - runoff);

   float sanormal = color.a;
   float inv_sanormal = 1.0f - sanormal;
   color *= 255.0f;

   if(minx < maxx || miny < maxy)
   {
      u32 source = (((u32)(color.r + 0.5f) << 16) |
                    ((u32)(color.g + 0.5f) << 8) |
                    ((u32)(color.b + 0.5f) << 0) |
                    ((u32)(color.a + 0.5f) << 24));
      u32w source_wide = set_u32w(source);

      if(color.a == 255.0f)
      {
         for(s32 y = miny; y < maxy; ++y)
         {
            u32 *row = target_memory + (y * target_width);
            for(s32 x = minx; x < wide_maxx; x += SIMD_WIDTH)
            {
               storeu_u32w((u32w *)(row + x), source_wide);
            }

            for(s32 x = wide_maxx; x < maxx; ++x)
            {
               row[x] = source;
            }
         }
      }
      else
      {
         u32w wideFF  = set_u32w(0xFF);

         f32w wide_sanormal = set_f32w(sanormal);
         f32w wide_inv_sanormal = set_f32w(inv_sanormal);

         f32w wide_sra = set_f32w(color.r) * wide_sanormal;
         f32w wide_sga = set_f32w(color.g) * wide_sanormal;
         f32w wide_sba = set_f32w(color.b) * wide_sanormal;
         f32w wide_saa = set_f32w(color.a) * wide_sanormal;

         for(s32 y = miny; y < maxy; ++y)
         {
            u32 *row = target_memory + (y * target_width);
            for(s32 x = minx; x < wide_maxx; x += SIMD_WIDTH)
            {
               u32w *destination = (u32w *)(row + x);
               u32w dcolors = loadu_u32w(destination);

               f32w dr = convert_to_f32w((dcolors >> 16) & wideFF);
               f32w dg = convert_to_f32w((dcolors >>  8) & wideFF);
               f32w db = convert_to_f32w((dcolors >>  0) & wideFF);
               f32w da = convert_to_f32w((dcolors >> 24) & wideFF);

               f32w r = (wide_inv_sanormal * dr) + wide_sra;
               f32w g = (wide_inv_sanormal * dg) + wide_sga;
               f32w b = (wide_inv_sanormal * db) + wide_sba;
               f32w a = (wide_inv_sanormal * da) + wide_saa;

               u32w pr = convert_to_u32w(r) << 16;
               u32w pg = convert_to_u32w(g) << 8;
               u32w pb = convert_to_u32w(b) << 0;
               u32w pa = convert_to_u32w(a) << 24;

               storeu_u32w(destination, pr|pg|pb|pa);
            }

            for(s32 x = wide_maxx; x < maxx; ++x)
            {
               u32 *destination = row + x;

               u32 dcolor = *destination;
               float dr = (float)((dcolor >> 16) & 0xFF);
               float dg = (float)((dcolor >>  8) & 0xFF);
               float db = (float)((dcolor >>  0) & 0xFF);
               float da = (float)((dcolor >> 24) & 0xFF);

               float r = ((1.0f - sanormal) * dr) + (color.r * sanormal);
               float g = ((1.0f - sanormal) * dg) + (color.g * sanormal);
               float b = ((1.0f - sanormal) * db) + (color.b * sanormal);
               float a = ((1.0f - sanormal) * da) + (color.a * sanormal);

               *destination = (((u32)(r + 0.5f) << 16) |
                               ((u32)(g + 0.5f) << 8) |
                               ((u32)(b + 0.5f) << 0) |
                               ((u32)(a + 0.5f) << 24));
            }
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
   // TODO: Fix alpha blending.

   posx -= bitmap->offsetx;
   posy -= bitmap->offsety;

   s32 minx = MAXIMUM(posx, 0);
   s32 miny = MAXIMUM(posy, 0);
   s32 maxx = MINIMUM(posx + bitmap->width, backbuffer->width);
   s32 maxy = MINIMUM(posy + bitmap->height, backbuffer->height);

   s32 clippedy = (miny - posy) * bitmap->width;
   s32 clippedx = (minx - posx);

   for(s32 destinationy = miny; destinationy < maxy; ++destinationy)
   {
      s32 sourcey = destinationy - miny;

      u32 *source_row = bitmap->memory + (sourcey * bitmap->width) + clippedy + clippedx;
      u32 *destination_row = backbuffer->memory + (destinationy * backbuffer->width);

      for(s32 destinationx = minx; destinationx < maxx; ++destinationx)
      {
         s32 sourcex = destinationx - minx;

         u32 source_color = source_row[sourcex];
         float sr = (float)((source_color >> 16) & 0xFF);
         float sg = (float)((source_color >>  8) & 0xFF);
         float sb = (float)((source_color >>  0) & 0xFF);
         float sa = (float)((source_color >> 24) & 0xFF);

         u32 *destination_pixel = destination_row + destinationx;

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

function void draw_outline(exo_texture *backbuffer, s32 x, s32 y, s32 width, s32 height, v4 color)
{
   draw_rectangle(backbuffer, x, y, width, 1, color); // N
   draw_rectangle(backbuffer, x, y + height - 1, width, 1, color); // S
   draw_rectangle(backbuffer, x, y, 1, height, color); // W
   draw_rectangle(backbuffer, x + width - 1, y, 1, height, color); // E
}

function void draw_outline(exo_texture *backbuffer, rectangle bounds, v4 color)
{
   draw_outline(backbuffer, bounds.x, bounds.y, bounds.width, bounds.height, color);
}

function void compute_region_size(rectangle *result, exo_window *window, window_region_type region)
{
   s32 b = EXO_WINDOW_DIM_BUTTON;
   s32 e = EXO_WINDOW_DIM_EDGE;
   s32 t = EXO_WINDOW_DIM_TITLEBAR;
   s32 c = EXO_WINDOW_DIM_CORNER;
   s32 b2 = EXO_WINDOW_HALFDIM_BUTTON;
   s32 e2 = EXO_WINDOW_HALFDIM_EDGE;
   s32 t2 = EXO_WINDOW_HALFDIM_TITLEBAR;

   s32 x = window->content.x;
   s32 y = window->content.y;
   s32 w = MAXIMUM(window->content.width, EXO_WINDOW_MIN_WIDTH);
   s32 h = MAXIMUM(window->content.height, EXO_WINDOW_MIN_HEIGHT);

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

function void compute_window_bounds(rectangle *result, exo_window *window)
{
   // TODO: Stop hard-coding offsets like this.

   compute_region_size(result, window, WINDOW_REGION_CONTENT);

   result->x -= EXO_WINDOW_DIM_EDGE;
   result->y -= (EXO_WINDOW_DIM_TITLEBAR + EXO_WINDOW_DIM_EDGE);

   result->width += (2 * EXO_WINDOW_DIM_EDGE);
   result->height += (EXO_WINDOW_DIM_TITLEBAR + (2 * EXO_WINDOW_DIM_EDGE));
}

#define HLDIM 2

function DRAW_REGION(draw_border_n)
{
   rectangle bounds;
   compute_region_size(&bounds, window, WINDOW_REGION_BORDER_N);

   draw_rectangle(backbuffer, bounds.x, bounds.y, bounds.width, HLDIM, PALETTE[0]);
   draw_rectangle(backbuffer, bounds.x, bounds.y + HLDIM, bounds.width, bounds.height - HLDIM, PALETTE[1]);
}

function DRAW_REGION(draw_border_s)
{
   rectangle bounds;
   compute_region_size(&bounds, window, WINDOW_REGION_BORDER_S);

   draw_rectangle(backbuffer, bounds.x, bounds.y, bounds.width, bounds.height - HLDIM, PALETTE[1]);
   draw_rectangle(backbuffer, bounds.x, bounds.y + bounds.height - HLDIM, bounds.width, HLDIM, PALETTE[2]);
}

function DRAW_REGION(draw_border_w)
{
   rectangle bounds;
   compute_region_size(&bounds, window, WINDOW_REGION_BORDER_W);

   draw_rectangle(backbuffer, bounds.x, bounds.y, HLDIM, bounds.height, PALETTE[0]);
   draw_rectangle(backbuffer, bounds.x + HLDIM, bounds.y, bounds.width - HLDIM, bounds.height, PALETTE[1]);
}

function DRAW_REGION(draw_border_e)
{
   rectangle bounds;
   compute_region_size(&bounds, window, WINDOW_REGION_BORDER_E);

   draw_rectangle(backbuffer, bounds.x, bounds.y, bounds.width - HLDIM, bounds.height, PALETTE[1]);
   draw_rectangle(backbuffer, bounds.x + bounds.width - HLDIM, bounds.y, HLDIM, bounds.height, PALETTE[2]);
}

function DRAW_REGION(draw_corner_nw)
{
   rectangle bounds;
   compute_region_size(&bounds, window, WINDOW_REGION_CORNER_NW);

   draw_rectangle(backbuffer, bounds.x, bounds.y, HLDIM, bounds.height, PALETTE[0]);
   draw_rectangle(backbuffer, bounds.x + HLDIM, bounds.y, bounds.width - HLDIM, HLDIM, PALETTE[0]);
   draw_rectangle(backbuffer, bounds.x + HLDIM, bounds.y + HLDIM, bounds.width - HLDIM, bounds.height - HLDIM, PALETTE[1]);
}

function DRAW_REGION(draw_corner_ne)
{
   rectangle bounds;
   compute_region_size(&bounds, window, WINDOW_REGION_CORNER_NE);

   draw_rectangle(backbuffer, bounds.x, bounds.y, bounds.width - HLDIM, HLDIM, PALETTE[0]);
   draw_rectangle(backbuffer, bounds.x + bounds.width - HLDIM, bounds.y, HLDIM, bounds.height, PALETTE[2]);
   draw_rectangle(backbuffer, bounds.x, bounds.y + HLDIM, bounds.width - HLDIM, bounds.height - HLDIM, PALETTE[1]);
}

function DRAW_REGION(draw_corner_sw)
{
   rectangle bounds;
   compute_region_size(&bounds, window, WINDOW_REGION_CORNER_SW);

   draw_rectangle(backbuffer, bounds.x, bounds.y, HLDIM, bounds.height, PALETTE[0]);
   draw_rectangle(backbuffer, bounds.x + HLDIM, bounds.y + bounds.height - HLDIM, bounds.width - HLDIM, HLDIM, PALETTE[2]);
   draw_rectangle(backbuffer, bounds.x + HLDIM, bounds.y, bounds.width - HLDIM, bounds.height - HLDIM, PALETTE[1]);
}

function DRAW_REGION(draw_corner_se)
{
   rectangle bounds;
   compute_region_size(&bounds, window, WINDOW_REGION_CORNER_SE);

   draw_rectangle(backbuffer, bounds.x, bounds.y, bounds.width - HLDIM, bounds.height - HLDIM, PALETTE[1]);
   draw_rectangle(backbuffer, bounds.x + bounds.width - HLDIM, bounds.y, HLDIM, bounds.height, PALETTE[2]);
   draw_rectangle(backbuffer, bounds.x, bounds.y + bounds.height - HLDIM, bounds.width - HLDIM, HLDIM, PALETTE[2]);
}

#undef HLDIM

function DRAW_REGION(draw_content)
{
   rectangle bounds;
   compute_region_size(&bounds, window, WINDOW_REGION_CONTENT);

   draw_rectangle(backbuffer, bounds, PALETTE[2]);

   // TODO: Handle clipping properly.

   s32 x = bounds.x + 3;
   s32 y = bounds.y + 6;

   char text_line[64];
   char *format = "{x:%d y:%d w:%d h:%d}";

   sprintf(text_line, format, window->x, window->y, window->width, window->height);
   draw_text_line(backbuffer, x, &y, text_line);

   sprintf(text_line, format, bounds.x, bounds.y, bounds.width, bounds.height);
   draw_text_line(backbuffer, x, &y, text_line);

   sprintf(text_line, "state:%d", window->state);
   draw_text_line(backbuffer, x, &y, text_line);

   y = ADVANCE_TEXT_LINE(y);
   draw_text_line(backbuffer, x, &y, "+----------------------------+");
   draw_text_line(backbuffer, x, &y, "| ASCII FONT TEST            |");
   draw_text_line(backbuffer, x, &y, "|----------------------------|");
   draw_text_line(backbuffer, x, &y, "| ABCDEFGHIJKLMNOPQRSTUVWXYZ |");
   draw_text_line(backbuffer, x, &y, "| abcdefghijklmnopqrstuvwxyz |");
   draw_text_line(backbuffer, x, &y, "| 0123456789!\"#$%&'()*+,-./: |");
   draw_text_line(backbuffer, x, &y, "| ;<=>?@[\\]^_`{|}~           |");
   draw_text_line(backbuffer, x, &y, "+----------------------------+");
}

function DRAW_REGION(draw_titlebar)
{
   rectangle bounds;
   compute_region_size(&bounds, window, WINDOW_REGION_TITLEBAR);

   v4 active_color = DEBUG_COLOR_GREEN;
   v4 passive_color = PALETTE[1];

   draw_rectangle(backbuffer, bounds, (is_active_window) ? active_color : passive_color);

   s32 x = bounds.x + 3;
   s32 y = ALIGN_TEXT_VERTICALLY(bounds.y, EXO_WINDOW_DIM_TITLEBAR);
   draw_text(backbuffer, x, y, window->title);
}

function bool is_window_visible(exo_window *window)
{
   bool result = (window->state != WINDOW_STATE_CLOSED && window->state != WINDOW_STATE_MINIMIZED);
   return(result);
}

function void draw_window(exo_texture *backbuffer, exo_state *es, u32 window_index)
{
   exo_window *window = es->windows + window_index;
   if(is_window_visible(window))
   {
      bool is_active_window = (window_index == es->active_window_index);

      for(s32 region_index = WINDOW_REGION_COUNT - 1; region_index >= 0; --region_index)
      {
         rectangle bounds;
         compute_region_size(&bounds, window, (window_region_type)region_index);

         window_region_entry *invariants = region_invariants + region_index;

         exo_texture *bitmap = es->region_bitmaps + region_index;
         if(bitmap->memory)
         {
            draw_bitmap(backbuffer, bitmap, bounds.x, bounds.y);
         }
         else
         {
            assert(invariants->draw);
            invariants->draw(backbuffer, window, is_active_window);
         }
      }

      rectangle bounds;
      compute_window_bounds(&bounds, window);
      draw_outline(backbuffer, bounds, PALETTE[3]);
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

function int compare_window_sort_entries(void const *ap, void const *bp)
{
   window_sort_entry *a = (window_sort_entry *)ap;
   window_sort_entry *b = (window_sort_entry *)bp;

   if(a->z > b->z) return(-1);
   if(a->z < b->z) return(1);
   return(0);
}

function void sort_windows(exo_window *windows, window_sort_entry *window_order, u32 count)
{
   for(u32 index = 0; index < count; ++index)
   {
      window_sort_entry *entry = window_order + index;
      exo_window *window = windows + index;

      entry->index = index;
      entry->z = window->z;
   }

   // TODO: Replace with our own stable sort.
   qsort(window_order, count, sizeof(window_order[0]), compare_window_sort_entries);

   // Remove any gaps/duplicates in the z values.
   for(u32 sort_index = 0; sort_index < count; ++sort_index)
   {
      s32 z = count - sort_index - 1;

      window_sort_entry *entry = window_order + sort_index;
      entry->z = z;
      windows[entry->index].z = z;
   }
}

function void raise_window(exo_state *es, exo_window *window)
{
   window->z = es->window_count;
   sort_windows(es->windows, es->window_order, es->window_count);

   if(!es->config.focus_follows_mouse)
   {
      es->active_window_index = (u32)(window - es->windows);
   }
}

function void minimize_window(exo_state *es, exo_window *window)
{
   window->state = WINDOW_STATE_MINIMIZED;

   if((u32)(window - es->windows) == es->active_window_index)
   {
      u32 new_active_index = EXO_WINDOW_NULL_INDEX;;
      for(u32 sort_index = 0; sort_index < es->window_count; ++sort_index)
      {
         u32 window_index = es->window_order[sort_index].index;
         exo_window *test_window = es->windows + window_index;

         if(is_window_visible(test_window))
         {
            new_active_index = window_index;
            break;
         }
      }

      es->active_window_index = new_active_index;
   }
}

function void close_window(exo_state *es, exo_window *window)
{
   u32 closed_index = (u32)(window - es->windows);
   u32 last_index = es->window_count - 1;

   exo_window *closed = es->windows + closed_index;
   exo_window *last = es->windows + last_index;

   *closed = *last;
   last->state = WINDOW_STATE_CLOSED;
   last->z = -1;

   sort_windows(es->windows, es->window_order, es->window_count);

   for(u32 index = 0; index < es->window_count; ++index)
   {
      es->windows[index].z--;
      es->window_order[index].z--;
   }

   es->window_count--;
}

function void create_window(exo_state *es, char *title, s32 x, s32 y, s32 width, s32 height)
{
   assert(es->window_count < (EXO_WINDOW_MAX_COUNT - 1));

   exo_window *window = es->windows + es->window_count++;
   window->state = WINDOW_STATE_NORMAL;
   window->title = title;

   rectangle content = create_rectangle(x, y, width, height);
   window->content = content;

   raise_window(es, window);
}

function void create_window(exo_state *es, char *title, s32 width, s32 height)
{
   s32 posx;
   s32 posy;
   get_default_window_location(&posx, &posy);
   create_window(es, title, posx, posy, width, height);
}

function hit_result detect_window_hit(exo_window *window, s32 x, s32 y)
{
   hit_result result = {EXO_REGION_NULL_INDEX};

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

function void interact_with_window(exo_state *es, exo_window *window, exo_input *input, hit_result hit)
{
   u32 window_index = (u32)(window - es->windows);

   input_state left_click = input->mouse_buttons[MOUSE_BUTTON_LEFT];

   // If the window was previously being interacted with but the mouse button is
   // no longer pressed, clear the interaction state.
   if(es->hot_window_index != EXO_WINDOW_NULL_INDEX && !is_pressed(left_click) && !was_released(left_click))
   {
      es->hot_window_index = EXO_WINDOW_NULL_INDEX;
      es->hot_region_index = EXO_REGION_NULL_INDEX;
   }

   // If a press/release occurred this frame and no hot interaction is currently
   // underway, update state to use the newly-hit window/region.
   if(left_click.changed_state)
   {
      if(es->hot_window_index == EXO_WINDOW_NULL_INDEX)
      {
         es->hot_window_index = window_index;
         es->hot_region_index = hit.region_index;
      }
   }

   // Don't interact on release if outside the original region bounds
   // (e.g. clicking a button, then dragging the cursor away before releasing).
   if(was_released(left_click) && (es->hot_window_index != window_index || es->hot_region_index != hit.region_index))
   {
      es->hot_window_index = EXO_WINDOW_NULL_INDEX;
      es->hot_region_index = EXO_REGION_NULL_INDEX;
   }

   // Set mouse_window_index regardless of other ongoing interactions.
   es->mouse_window_index = window_index;

   // Default to using the appropriate hover cursor based on mouse position.
   if(hit.region_index != EXO_REGION_NULL_INDEX)
   {
      es->frame_cursor = region_invariants[hit.region_index].cursor;
   }

   if(es->hot_window_index != EXO_WINDOW_NULL_INDEX && es->hot_region_index != EXO_REGION_NULL_INDEX)
   {
      // Override the cursor if an ongoing interaction is underway.
      es->frame_cursor = region_invariants[es->hot_region_index].cursor;

      // All interactions result in raising the window.
      if(was_pressed(input->mouse_buttons[MOUSE_BUTTON_LEFT]))
      {
         raise_window(es, window);
      }

      // TODO: This resize logic correctly maps the mouse to the window in
      // terms of positioning, but does not correctly account for the minimum
      // window size - it causes the window to move instead.

      s32 deltax = (input->mousex - input->previous_mousex);
      s32 deltay = (input->mousey - input->previous_mousey);

      switch(region_invariants[es->hot_region_index].interaction)
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
               window->content.y = input->mousey + EXO_WINDOW_HALFDIM_TITLEBAR;
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
                  window->x = EXO_WINDOW_DIM_EDGE;
                  window->y = EXO_WINDOW_DIM_EDGE + EXO_WINDOW_DIM_TITLEBAR;
                  window->width = EXO_SCREEN_RESOLUTION_X - (2 * EXO_WINDOW_DIM_EDGE);
                  window->height = EXO_SCREEN_RESOLUTION_Y - window->y - EXO_TASKBAR_HEIGHT - EXO_WINDOW_DIM_EDGE;

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
                  minimize_window(es, window);
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
      es->hot_window_index = EXO_WINDOW_NULL_INDEX;
      es->hot_region_index = EXO_REGION_NULL_INDEX;

      // TODO: It would be nicer if we didn't allow the internal window size to
      // fall below the specified minimum. We fix it here when click events end,
      // but it leaves window.width and window.height unsafe to use in the
      // middle of a resizing (before compute_region_size is called).
      compute_region_size(&window->content, window, WINDOW_REGION_CONTENT);
   }
}

function void draw_debug_overlay(exo_texture *backbuffer, exo_input *input)
{
   char overlay_text[32];
   u32 color = 0xFF00FF00;

   s32 x = backbuffer->width - (FONT_WIDTH * FONT_SCALE * sizeof(overlay_text));
   s32 y = 10;

   draw_text_line(backbuffer, x, &y, "DEBUG INFORMATION", color);
   draw_text_line(backbuffer, x, &y, "-----------------", color);

#if(SIMD_WIDTH == 8)
   draw_text_line(backbuffer, x, &y, "SIMD target: AVX2", color);
#elif(SIMD_WIDTH == 4)
   draw_text_line(backbuffer, x, &y, "SIMD target: SSE2", color);
#else
   draw_text_line(backbuffer, x, &y, "SIMD target: NONE", color);
#endif

   sprintf(overlay_text, "Frame time:  %.04fms\n", input->frame_seconds_elapsed * 1000.0f);
   draw_text_line(backbuffer, x, &y, overlay_text, color);

   sprintf(overlay_text, "Target time: %.04fms\n", input->target_seconds_per_frame * 1000.0f);
   draw_text_line(backbuffer, x, &y, overlay_text, color);
}

function void update(exo_texture *backbuffer, exo_input *input, exo_storage *storage)
{
   exo_state *es = (exo_state *)storage->memory;
   if(!es->is_initialized)
   {
      create_window(es, "Test Window 0", 400, 300);
      create_window(es, "Test Window 1", 400, 300);
      create_window(es, "Test Window 2", 400, 300);
      create_window(es, "Test Window 3", 400, 300);
      create_window(es, "Test Window 4", 400, 300);

      es->hot_window_index = EXO_WINDOW_NULL_INDEX;
      es->hot_region_index = EXO_REGION_NULL_INDEX;

      es->cursor_bitmaps[CURSOR_ARROW]         = load_bitmap("cursor_arrow.bmp");
      es->cursor_bitmaps[CURSOR_MOVE]          = load_bitmap("cursor_move.bmp", 8, 8);
      es->cursor_bitmaps[CURSOR_RESIZE_VERT]   = load_bitmap("cursor_vertical_resize.bmp", 4, 8);
      es->cursor_bitmaps[CURSOR_RESIZE_HORI]   = load_bitmap("cursor_horizontal_resize.bmp", 8, 4);
      es->cursor_bitmaps[CURSOR_RESIZE_DIAG_L] = load_bitmap("cursor_diagonal_left.bmp", 7, 7);
      es->cursor_bitmaps[CURSOR_RESIZE_DIAG_R] = load_bitmap("cursor_diagonal_right.bmp", 7, 7);

      es->region_bitmaps[WINDOW_REGION_BUTTON_CLOSE]    = load_bitmap("close.bmp");
      es->region_bitmaps[WINDOW_REGION_BUTTON_MAXIMIZE] = load_bitmap("maximize.bmp");
      es->region_bitmaps[WINDOW_REGION_BUTTON_MINIMIZE] = load_bitmap("minimize.bmp");

      initialize_font();

      // es->config.focus_follows_mouse = true;

      es->is_initialized = true;
   }

   if(was_pressed(input->mouse_buttons[MOUSE_BUTTON_RIGHT]))
   {
      create_window(es, "New Window", input->mousex, input->mousey, 400, 300);
   }

   es->frame_cursor = CURSOR_ARROW;
   es->mouse_window_index = EXO_WINDOW_NULL_INDEX;

   // Handle window interactions.
   for(u32 sort_index = 0; sort_index < es->window_count; ++sort_index)
   {
      u32 window_index = es->window_order[sort_index].index;
      exo_window *window = es->windows + window_index;

      hit_result hit = detect_window_hit(window, input->mousex, input->mousey);
      if(hit.region_index != EXO_REGION_NULL_INDEX || es->hot_region_index != EXO_REGION_NULL_INDEX)
      {
         interact_with_window(es, window, input, hit);
         break;
      }
   }

   // Defer "closing" the windows until after interactions are complete, so that
   // shuffling the array doesn't impact the loop.
   for(u32 index = 0; index < es->window_count; ++index)
   {
      exo_window *window = es->windows + es->window_order[index].index;
      if(window->state == WINDOW_STATE_CLOSED)
      {
         close_window(es, window);
         index--;

         es->active_window_index = EXO_WINDOW_NULL_INDEX;
         es->hot_window_index = EXO_WINDOW_NULL_INDEX;
         es->hot_region_index = EXO_REGION_NULL_INDEX;
      }
   }

   // Don't let other windows grab focus when dragging a window around, always
   // give precedence to the hot window.
   if(es->hot_window_index != EXO_WINDOW_NULL_INDEX && is_window_visible(es->windows + es->hot_window_index))
   {
      es->active_window_index = es->hot_window_index;
   }
   else if(es->config.focus_follows_mouse)
   {
      es->active_window_index = es->mouse_window_index;
   }

   // Draw desktop.
   clear(backbuffer, PALETTE[3]);

   draw_debug_overlay(backbuffer, input);

   // Draw windows and their regions in reverse order, so that the earlier
   // elements in the list appear on top.
   for(s32 sort_index = es->window_count - 1; sort_index >= 0; --sort_index)
   {
      u32 window_index = es->window_order[sort_index].index;
      draw_window(backbuffer, es, window_index);
   }

   // Draw taskbar.
   rectangle taskbar = create_rectangle(0, backbuffer->height - EXO_TASKBAR_HEIGHT, backbuffer->width, EXO_TASKBAR_HEIGHT);
   draw_rectangle(backbuffer, taskbar, PALETTE[1]);
   draw_rectangle(backbuffer, taskbar.x, taskbar.y, taskbar.width, 2, PALETTE[0]);

   s32 gap = 4;
   rectangle tab = create_rectangle(taskbar.x + gap, taskbar.y + gap, EXO_WINDOWTAB_WIDTH_MAX, taskbar.height - (2 * gap));

   for(u32 window_index = 0; window_index < es->window_count; ++window_index)
   {
      exo_window *window = es->windows + window_index;
      assert(window->state != WINDOW_STATE_CLOSED);

      v4 color = PALETTE[2];
      if(window_index == es->active_window_index)
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
               raise_window(es, window);
            }
            else if(window_index != es->active_window_index)
            {
               raise_window(es, window);
            }
            else
            {
               minimize_window(es, window);
            }
         }
      }

      draw_rectangle(backbuffer, tab, color);

      s32 x = tab.x + 3;
      s32 y = ALIGN_TEXT_VERTICALLY(tab.y, tab.height);
      draw_text(backbuffer, x, y, window->title);

      tab.x += (tab.width + (2 * gap));
   }

   // Draw cursor.
   exo_texture *cursor_bitmap = es->cursor_bitmaps + es->frame_cursor;
   draw_bitmap(backbuffer, cursor_bitmap, input->mousex, input->mousey);
}
