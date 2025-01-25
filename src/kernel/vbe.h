#pragma once

/* (c) copyright 2024 Lawrence D. Kern ////////////////////////////////////// */

#pragma pack(push, 1)
typedef struct {
   char signature[4];
   u16 version;
   u32 oem;
   u32 capabilities;
   u32 video_modes;
   u16 video_memory;
   u16 software_revision;
   u32 vendor;
   u32 product_name;
   u32 produce_revision;
   char reserved[222];
   char oem_data[256];
} vbe_information;

typedef struct {
   u16 attributes;
   u8 window_a;
   u8 window_b;
   u16 granularity;
   u16 window_size;
   u16 segment_a;
   u16 segment_b;
   u32 window_function;
   u16 pitch;
   u16 width;
   u16 height;
   u8 w_char;
   u8 y_char;
   u8 planes;
   u8 bpp;
   u8 banks;
   u8 memory_model;
   u8 bank_size;
   u8 image_pages;
   u8 reserved0;

   u8 red_mask;
   u8 red_position;
   u8 green_mask;
   u8 green_position;
   u8 blue_mask;
   u8 blue_position;
   u8 reserved_mask;
   u8 reserved_position;
   u8 direct_color_attributes;

   u32 frame_buffer;
   u32 off_screen_mem_off;
   u32 off_screen_mem_size;
   u8 reserved1[206];
} vbe_mode_information;

typedef struct {
   u16 set_window;
   u16 set_display_start;
   u16 set_palette;
} vbe_protected_mode_interface;
#pragma pack(pop)
