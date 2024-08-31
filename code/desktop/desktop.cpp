/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#include "desktop.h"

#include "desktop_simd.cpp"
#include "desktop_text.cpp"

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

function rectangle create_rectangle(i32 x, i32 y, i32 width, i32 height)
{
   rectangle result = {x, y, width, height};
   return(result);
}

function bool in_rectangle(rectangle rect, i32 x, i32 y)
{
   bool result = (x >= rect.x && x < (rect.x + rect.width) &&
                  y >= rect.y && y < (rect.y + rect.height));

   return(result);
}

function texture load_bitmap(desktop_state *ds, char *file_path, u32 offsetx = 0, u32 offsety = 0)
{
   arena *permanent = &ds->permanent;
   arena *scratch = &ds->scratch;

   texture result = {0};
   result.offsetx = offsetx;
   result.offsety = offsety;

   FILE *file = fopen(file_path, "rb");
   assert(file);

   fseek(file, 0, SEEK_END);
   size_t size = ftell(file);
   fseek(file, 0, SEEK_SET);

   arena_marker marker = arena_marker_set(scratch);
   u8 *memory = (u8 *)arena_allocate(scratch, u8, size);

   size_t bytes_read = fread(memory, 1, size, file);
   assert(bytes_read == size);

   bitmap_header *header = (bitmap_header *)memory;

   assert(header->file_type == 0x4D42); // "BM"
   assert(header->bits_per_pixel == 32);

   result.width = header->width;
   result.height = header->height;
   result.memory = (u32 *)arena_allocate(permanent, u32, result.width * result.height);

   u32 *source_memory = (u32 *)(memory + header->bitmap_offset);
   u32 *row = source_memory + (result.width * (result.height - 1));

   for(i32 y = 0; y < result.height; ++y)
   {
      for(i32 x = 0; x < result.width; ++x)
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

function void clear(texture *destination, vec4 color)
{
   color = (color * 255.0f) + 0.5f;
   u32 pixel = (((u32)color.r << 16) |
                ((u32)color.g << 8) |
                ((u32)color.b << 0) |
                ((u32)color.a << 24));
   u32w pixel_wide = set_u32w(pixel);

   i32 max = destination->width * destination->height;
   i32 wide_max = max - (max % SIMD_WIDTH);

   u32 *memory = destination->memory;
   for(i32 index = 0; index < wide_max; index += SIMD_WIDTH)
   {
      storeu_u32w((u32w *)(memory + index), pixel_wide);
   }
   for(i32 index = wide_max; index < max; ++index)
   {
      memory[index] = pixel;
   }
}

function void draw_rectangle(texture *backbuffer, i32 posx, i32 posy, i32 width, i32 height, vec4 color)
{
   i32 target_width = backbuffer->width;
   i32 target_height = backbuffer->height;
   u32 *target_memory = backbuffer->memory;

   i32 minx = MAXIMUM(posx, 0);
   i32 miny = MAXIMUM(posy, 0);
   i32 maxx = MINIMUM(posx + width, target_width);
   i32 maxy = MINIMUM(posy + height, target_height);

   i32 runoff = (maxx - minx) % SIMD_WIDTH;
   i32 wide_maxx = MAXIMUM(minx, maxx - runoff);

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
         for(i32 y = miny; y < maxy; ++y)
         {
            u32 *row = target_memory + (y * target_width);
            for(i32 x = minx; x < wide_maxx; x += SIMD_WIDTH)
            {
               storeu_u32w((u32w *)(row + x), source_wide);
            }

            for(i32 x = wide_maxx; x < maxx; ++x)
            {
               row[x] = source;
            }
         }
      }
      else
      {
         u32w wide_255  = set_u32w(0xFF);
         f32w wide_sanormal = set_f32w(sanormal);
         f32w wide_inv_sanormal = set_f32w(inv_sanormal);

         f32w wide_sra = set_f32w(color.r) * wide_sanormal;
         f32w wide_sga = set_f32w(color.g) * wide_sanormal;
         f32w wide_sba = set_f32w(color.b) * wide_sanormal;
         f32w wide_saa = set_f32w(color.a) * wide_sanormal;

         for(i32 y = miny; y < maxy; ++y)
         {
            u32 *row = target_memory + (y * target_width);
            for(i32 x = minx; x < wide_maxx; x += SIMD_WIDTH)
            {
               u32w *destination = (u32w *)(row + x);
               u32w dcolors = loadu_u32w(destination);

               f32w dr = convert_to_f32w((dcolors >> 16) & wide_255);
               f32w dg = convert_to_f32w((dcolors >>  8) & wide_255);
               f32w db = convert_to_f32w((dcolors >>  0) & wide_255);
               f32w da = convert_to_f32w((dcolors >> 24) & wide_255);

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

            for(i32 x = wide_maxx; x < maxx; ++x)
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

function void draw_rectangle(texture *backbuffer, rectangle rect, vec4 color)
{
   draw_rectangle(backbuffer, rect.x, rect.y, rect.width, rect.height, color);
}

function void draw_texture_bounded(texture *destination, texture *texture, i32 posx, i32 posy, i32 width, i32 height)
{
   posx -= texture->offsetx;
   posy -= texture->offsety;

   width = MINIMUM(width, texture->width);
   height = MINIMUM(height, texture->height);

   i32 minx = MAXIMUM(posx, 0);
   i32 miny = MAXIMUM(posy, 0);
   i32 maxx = MINIMUM(posx + width, destination->width);
   i32 maxy = MINIMUM(posy + height, destination->height);

   i32 clippedy = (miny - posy) * texture->width;
   i32 clippedx = (minx - posx);

   i32 runoff = (maxx - minx) % SIMD_WIDTH;
   i32 wide_maxx = MAXIMUM(minx, maxx - runoff);

   u32w wide_255 = set_u32w(0xFF);
   f32w wide_inv_255f = set_f32w(1.0f / 255.0f);
   f32w wide_1f = set_f32w(1.0f);

   for(i32 destinationy = miny; destinationy < maxy; ++destinationy)
   {
      i32 sourcey = destinationy - miny;

      u32 *source_row = texture->memory + (sourcey * texture->width) + clippedy + clippedx;
      u32 *destination_row = destination->memory + (destinationy * destination->width);

      for(i32 destinationx = minx; destinationx < wide_maxx; destinationx += SIMD_WIDTH)
      {
         i32 sourcex = destinationx - minx;

         u32w *source_address = (u32w *)(source_row + sourcex);
         u32w source_color = loadu_u32w(source_address);

         f32w sr = convert_to_f32w((source_color >> 16) & wide_255);
         f32w sg = convert_to_f32w((source_color >>  8) & wide_255);
         f32w sb = convert_to_f32w((source_color >>  0) & wide_255);
         f32w sa = convert_to_f32w((source_color >> 24) & wide_255);

         u32w *destination_address = (u32w *)(destination_row + destinationx);

         u32w destination_color = loadu_u32w(destination_address);
         f32w dr = convert_to_f32w((destination_color >> 16) & wide_255);
         f32w dg = convert_to_f32w((destination_color >>  8) & wide_255);
         f32w db = convert_to_f32w((destination_color >>  0) & wide_255);
         f32w da = convert_to_f32w((destination_color >> 24) & wide_255);

         f32w sanormal = wide_inv_255f * sa;

         f32w r = ((wide_1f - sanormal) * dr) + sr;
         f32w g = ((wide_1f - sanormal) * dg) + sg;
         f32w b = ((wide_1f - sanormal) * db) + sb;
         f32w a = ((wide_1f - sanormal) * da) + sa;

         u32w pr = convert_to_u32w(r) << 16;
         u32w pg = convert_to_u32w(g) << 8;
         u32w pb = convert_to_u32w(b) << 0;
         u32w pa = convert_to_u32w(a) << 24;

         storeu_u32w(destination_address, pr|pg|pb|pa);
      }

      for(i32 destinationx = wide_maxx; destinationx < maxx; ++destinationx)
      {
         i32 sourcex = destinationx - minx;

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

function void draw_texture(texture *destination, texture *texture, i32 posx, i32 posy)
{
   draw_texture_bounded(destination, texture, posx, posy, texture->width, texture->height);
}

function void draw_outline(texture *destination, i32 x, i32 y, i32 width, i32 height, vec4 color)
{
   draw_rectangle(destination, x, y, width, 1, color); // N
   draw_rectangle(destination, x, y + height - 1, width, 1, color); // S
   draw_rectangle(destination, x, y, 1, height, color); // W
   draw_rectangle(destination, x + width - 1, y, 1, height, color); // E
}

function void draw_outline(texture *destination, rectangle bounds, vec4 color)
{
   draw_outline(destination, bounds.x, bounds.y, bounds.width, bounds.height, color);
}

function void compute_region_size(rectangle *result, desktop_window *window, window_region_type region)
{
   i32 b = DESKTOP_WINDOW_DIM_BUTTON;
   i32 e = DESKTOP_WINDOW_DIM_EDGE;
   i32 t = DESKTOP_WINDOW_DIM_TITLEBAR;
   i32 c = DESKTOP_WINDOW_DIM_CORNER;
   i32 b2 = DESKTOP_WINDOW_HALFDIM_BUTTON;
   i32 e2 = DESKTOP_WINDOW_HALFDIM_EDGE;
   i32 t2 = DESKTOP_WINDOW_HALFDIM_TITLEBAR;

   i32 x = window->content.x;
   i32 y = window->content.y;
   i32 w = MAXIMUM(window->content.width, DESKTOP_WINDOW_MIN_WIDTH);
   i32 h = MAXIMUM(window->content.height, DESKTOP_WINDOW_MIN_HEIGHT);

   i32 buttonx = x + w - b - e2;
   i32 buttony = y - t2 - b2;

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
   draw_rectangle(destination, bounds, PALETTE[4]);

   clear(texture, PALETTE[2]);

   i32 x = 3;
   i32 y = 6;

   char text_line[64];
   char *format = "{x:%d y:%d w:%d h:%d}";

   int length = sprintf(text_line, format, window->x, window->y, window->width, window->height);
   draw_text_line(texture, x, &y, s8new((u8 *)text_line, length));

   length = sprintf(text_line, format, bounds.x, bounds.y, bounds.width, bounds.height);
   draw_text_line(texture, x, &y, s8new((u8 *)text_line, length));

   length = sprintf(text_line, "state:%d", window->state);
   draw_text_line(texture, x, &y, s8new((u8 *)text_line, length));

   y = ADVANCE_TEXT_LINE(y);
   draw_text_line(texture, x, &y, s8("+----------------------------+"));
   draw_text_line(texture, x, &y, s8("| ASCII FONT TEST            |"));
   draw_text_line(texture, x, &y, s8("|----------------------------|"));
   draw_text_line(texture, x, &y, s8("| ABCDEFGHIJKLMNOPQRSTUVWXYZ |"));
   draw_text_line(texture, x, &y, s8("| abcdefghijklmnopqrstuvwxyz |"));
   draw_text_line(texture, x, &y, s8("| AaBbCcDdEeFfGgHhIiJjKkLlMm |"));
   draw_text_line(texture, x, &y, s8("| NnOoPpQqRrSsTtUuVvWwXxYyZz |"));
   draw_text_line(texture, x, &y, s8("| 0123456789!\"#$%&'()*+,-./: |"));
   draw_text_line(texture, x, &y, s8("| ;<=>?@[\\]^_`{|}~           |"));
   draw_text_line(texture, x, &y, s8("+----------------------------+"));

   draw_texture_bounded(destination, texture, bounds.x, bounds.y, bounds.width, bounds.height);
}

function DRAW_REGION(draw_titlebar)
{
   rectangle bounds;
   compute_region_size(&bounds, window, WINDOW_REGION_TITLEBAR);

   vec4 active_color = DEBUG_COLOR_GREEN;
   vec4 passive_color = PALETTE[1];

   draw_rectangle(destination, bounds, (is_active_window) ? active_color : passive_color);

   i32 x = bounds.x + 3;
   i32 y = ALIGN_TEXT_VERTICALLY(bounds.y, DESKTOP_WINDOW_DIM_TITLEBAR);
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

      for(i32 region_index = WINDOW_REGION_COUNT - 1; region_index >= 0; --region_index)
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
      draw_outline(destination, bounds, PALETTE[3]);
   }
}

function void get_default_window_location(i32 *posx, i32 *posy)
{
   i32 initial_x = 50;
   i32 initial_y = 50;

   static i32 x = initial_x;
   static i32 y = initial_y;

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

function void create_window(desktop_state *ds, s8 title, i32 x, i32 y, i32 width, i32 height)
{
   desktop_window *window = 0;
   if(ds->free_window)
   {
      window = ds->free_window;
      ds->free_window = ds->free_window->next;
   }
   else
   {
      window = arena_allocate(&ds->permanent, desktop_window, 1);
   }

   *window = {};
   window->state = WINDOW_STATE_NORMAL;
   window->title = title;

   rectangle content = create_rectangle(x, y, width, height);
   window->content = content;

   // BUG: Decouple texture creation from window creation. Right now texture
   // memory does not get reused after windows are recreated.
   texture texture = {};
   texture.width = content.width;
   texture.height = content.height;
   texture.memory = (u32 *)arena_allocate(&ds->permanent, u32, texture.width * texture.height);
   window->texture = texture;

   raise_window(ds, window);
}

function void create_window(desktop_state *ds, s8 title, i32 initial_width, i32 initial_height)
{
   i32 posx;
   i32 posy;
   get_default_window_location(&posx, &posy);
   create_window(ds, title, posx, posy, initial_width, initial_height);
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

function hit_result detect_window_hit(desktop_window *window, i32 x, i32 y)
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

      i32 deltax = (input->mousex - input->previous_mousex);
      i32 deltay = (input->mousey - input->previous_mousey);

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
   u32 color = 0xFF00FF00;

   i32 x = destination->width - (FONT_WIDTH * FONT_SCALE * sizeof(overlay_text));
   i32 y = 10;

   draw_text_line(destination, x, &y, s8("DEBUG INFORMATION"), color);
   draw_text_line(destination, x, &y, s8("-----------------"), color);

#if(SIMD_WIDTH == 8)
   draw_text_line(destination, x, &y, s8("SIMD target: AVX2"), color);
#elif(SIMD_WIDTH == 4)
   draw_text_line(destination, x, &y, s8("SIMD target: SSE2"), color);
#else
   draw_text_line(destination, x, &y, s8("SIMD target: NONE"), color);
#endif

   int length = sprintf(overlay_text, "Frame time:  %.04fms\n", input->frame_seconds_elapsed * 1000.0f);
   draw_text_line(destination, x, &y, s8new((u8 *)overlay_text, length), color);

   length = sprintf(overlay_text, "Target time: %.04fms\n", input->target_seconds_per_frame * 1000.0f);
   draw_text_line(destination, x, &y, s8new((u8 *)overlay_text, length), color);
}

function void update(texture *backbuffer, desktop_input *input, desktop_storage *storage)
{
   desktop_state *ds = (desktop_state *)storage->memory;
   if(!ds->is_initialized)
   {
      arena_initialize(&ds->permanent, storage->memory + sizeof(*ds), MEGABYTES(256));
      arena_initialize(&ds->scratch, storage->memory + sizeof(*ds) + ds->permanent.cap, KILOBYTES(64));

      create_window(ds, s8("Test Window 0"), 400, 300);
      create_window(ds, s8("Test Window 1"), 400, 300);
      create_window(ds, s8("Test Window 2"), 400, 300);
      create_window(ds, s8("Test Window 3"), 400, 300);
      create_window(ds, s8("Test Window 4"), 400, 300);

      ds->hot_window = 0;
      ds->hot_region_index = DESKTOP_REGION_NULL_INDEX;

      ds->cursor_textures[CURSOR_ARROW]         = load_bitmap(ds, "cursor_arrow.bmp");
      ds->cursor_textures[CURSOR_MOVE]          = load_bitmap(ds, "cursor_move.bmp", 8, 8);
      ds->cursor_textures[CURSOR_RESIZE_VERT]   = load_bitmap(ds, "cursor_vertical_resize.bmp", 4, 8);
      ds->cursor_textures[CURSOR_RESIZE_HORI]   = load_bitmap(ds, "cursor_horizontal_resize.bmp", 8, 4);
      ds->cursor_textures[CURSOR_RESIZE_DIAG_L] = load_bitmap(ds, "cursor_diagonal_left.bmp", 7, 7);
      ds->cursor_textures[CURSOR_RESIZE_DIAG_R] = load_bitmap(ds, "cursor_diagonal_right.bmp", 7, 7);

      ds->region_textures[WINDOW_REGION_BUTTON_CLOSE]    = load_bitmap(ds, "close.bmp");
      ds->region_textures[WINDOW_REGION_BUTTON_MAXIMIZE] = load_bitmap(ds, "maximize.bmp");
      ds->region_textures[WINDOW_REGION_BUTTON_MINIMIZE] = load_bitmap(ds, "minimize.bmp");

      initialize_font();

      // ds->config.focus_follows_mouse = true;

      ds->is_initialized = true;
   }

   if(was_pressed(input->mouse_buttons[MOUSE_BUTTON_RIGHT]))
   {
      create_window(ds, s8("New Window"), input->mousex, input->mousey, 400, 300);
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
   draw_rectangle(backbuffer, taskbar, PALETTE[1]);
   draw_rectangle(backbuffer, taskbar.x, taskbar.y, taskbar.width, 2, PALETTE[0]);

   i32 gap = 4;
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

      draw_rectangle(backbuffer, tab, color);

      i32 x = tab.x + 3;
      i32 y = ALIGN_TEXT_VERTICALLY(tab.y, tab.height);
      draw_text(backbuffer, x, y, window->title);

      tab.x += (tab.width + (2 * gap));
   }

   // NOTE: Draw cursor.
   texture *cursor_texture = ds->cursor_textures + ds->frame_cursor;
   draw_texture(backbuffer, cursor_texture, input->mousex, input->mousey);
}
