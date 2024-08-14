#pragma once

/* ////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern ////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */

typedef enum {
   VGA_TEXTMODE_COLOR_BLACK         = 0,
   VGA_TEXTMODE_COLOR_BLUE          = 1,
   VGA_TEXTMODE_COLOR_GREEN         = 2,
   VGA_TEXTMODE_COLOR_CYAN          = 3,
   VGA_TEXTMODE_COLOR_RED           = 4,
   VGA_TEXTMODE_COLOR_MAGENTA       = 5,
   VGA_TEXTMODE_COLOR_BROWN         = 6,
   VGA_TEXTMODE_COLOR_LIGHTGREY    = 7,
   VGA_TEXTMODE_COLOR_DARKGREY     = 8,
   VGA_TEXTMODE_COLOR_LIGHTBLUE    = 9,
   VGA_TEXTMODE_COLOR_LIGHTGREEN   = 10,
   VGA_TEXTMODE_COLOR_LIGHTCYAN    = 11,
   VGA_TEXTMODE_COLOR_LIGHTRED     = 12,
   VGA_TEXTMODE_COLOR_LIGHTMAGENTA = 13,
   VGA_TEXTMODE_COLOR_LIGHTBROWN   = 14,
   VGA_TEXTMODE_COLOR_WHITE         = 15,
} vgacolor;

#define VGA_TEXTMODE_COLOR(foreground, background) (((u8)(VGA_TEXTMODE_COLOR_##background) << 4) | (u8)((VGA_TEXTMODE_COLOR_##foreground) & 0xF))
#define VGA_TEXTMODE_CELL(codepoint, color) (u16)(((u8)(color) << 8) | (codepoint))

#define VGA_TEXTMODE_WIDTH  80
#define VGA_TEXTMODE_HEIGHT 25
