/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */

#include "desktop.h"
#include "renderer.h"
#include "simd.cpp"

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

CLEAR(clear)
{
   color = (color * 255.0f) + 0.5f;
   u32 pixel = (((u32)color.r << 16) |
                ((u32)color.g << 8) |
                ((u32)color.b << 0) |
                ((u32)color.a << 24));
   u32w pixel_wide = set_u32w(pixel);

   s32 max = destination->width * destination->height;
   s32 wide_max = max - (max % SIMD_WIDTH);

   u32 *memory = destination->memory;
   for(s32 index = 0; index < wide_max; index += SIMD_WIDTH)
   {
      storeu_u32w((u32w *)(memory + index), pixel_wide);
   }
   for(s32 index = wide_max; index < max; ++index)
   {
      memory[index] = pixel;
   }
}

DRAW_RECTANGLE(draw_rectangle)
{
   s32 target_width = destination->width;
   s32 target_height = destination->height;
   u32 *target_memory = destination->memory;

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
         u32w wide_255  = set_u32w(0xFF);
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

DRAW_TEXTURE_BOUNDED(draw_texture_bounded)
{
   posx -= texture->offsetx;
   posy -= texture->offsety;

   width = MINIMUM(width, texture->width);
   height = MINIMUM(height, texture->height);

   s32 minx = MAXIMUM(posx, 0);
   s32 miny = MAXIMUM(posy, 0);
   s32 maxx = MINIMUM(posx + width, destination->width);
   s32 maxy = MINIMUM(posy + height, destination->height);

   s32 clippedy = (miny - posy) * texture->width;
   s32 clippedx = (minx - posx);

   s32 runoff = (maxx - minx) % SIMD_WIDTH;
   s32 wide_maxx = MAXIMUM(minx, maxx - runoff);

   u32w wide_255 = set_u32w(0xFF);
   f32w wide_inv_255f = set_f32w(1.0f / 255.0f);
   f32w wide_1f = set_f32w(1.0f);

   for(s32 destinationy = miny; destinationy < maxy; ++destinationy)
   {
      s32 sourcey = destinationy - miny;

      u32 *source_row = texture->memory + (sourcey * texture->width) + clippedy + clippedx;
      u32 *destination_row = destination->memory + (destinationy * destination->width);

      for(s32 destinationx = minx; destinationx < wide_maxx; destinationx += SIMD_WIDTH)
      {
         s32 sourcex = destinationx - minx;

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

      for(s32 destinationx = wide_maxx; destinationx < maxx; ++destinationx)
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

DRAW_TEXTURE(draw_texture)
{
   draw_texture_bounded(destination, texture, posx, posy, texture->width, texture->height);
}

DRAW_OUTLINE(draw_outline)
{
   draw_rectangle(destination, x, y, width, 1, color); // N
   draw_rectangle(destination, x, y + height - 1, width, 1, color); // S
   draw_rectangle(destination, x, y, 1, height, color); // W
   draw_rectangle(destination, x + width - 1, y, 1, height, color); // E
}

DRAW_RECTANGLE_25(draw_rectangle_25)
{
   int minx = MAXIMUM(x, 0);
   int miny = MAXIMUM(y, 0);
   int maxx = MINIMUM(x + width, destination->width);
   int maxy = MINIMUM(y + height, destination->height);

   u32 *memory = destination->memory;

   u32 colors[] = {0xFF000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
   for(int y = miny; y < maxy; ++y)
   {
      int color_offset = (y & 1) * 2;

      u32 *row = memory + (y * destination->width);
      for(int x = minx; x < maxx; ++x)
      {
         int color_index = (x + color_offset) & 3;
         row[x] = colors[color_index];
      }
   }
}

DRAW_RECTANGLE_50(draw_rectangle_50)
{
   int minx = MAXIMUM(x, 0);
   int miny = MAXIMUM(y, 0);
   int maxx = MINIMUM(x + width, destination->width);
   int maxy = MINIMUM(y + height, destination->height);

   u32 *memory = destination->memory;

   u32 colors[] = {0xFF000000, 0xFFFFFFFF};
   for(int y = miny; y < maxy; ++y)
   {
      int color_offset = y & 1;

      u32 *row = memory + (y * destination->width);
      for(int x = minx; x < maxx; ++x)
      {
         int color_index = (x + color_offset) & 1;
         row[x] = colors[color_index];
      }
   }
}

DRAW_RECTANGLE_75(draw_rectangle_75)
{
   int minx = MAXIMUM(x, 0);
   int miny = MAXIMUM(y, 0);
   int maxx = MINIMUM(x + width, destination->width);
   int maxy = MINIMUM(y + height, destination->height);

   u32 *memory = destination->memory;

   u32 colors[] = {0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF};
   for(int y = miny; y < maxy; ++y)
   {
      int color_offset = (y & 1) * 2;

      u32 *row = memory + (y * destination->width);
      for(int x = minx; x < maxx; ++x)
      {
         int color_index = (x + color_offset) & 3;
         row[x] = colors[color_index];
      }
   }
}
