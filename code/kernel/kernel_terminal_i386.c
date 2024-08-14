/* ////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern ////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */

#include "kernel_terminal.h"
#include "kernel_vga.h"

global size global_terminal_cellx;
global size global_terminal_celly;
global u16 *global_terminal_cells;
global u8 global_terminal_color;

function TERMINAL_INITIALIZE(terminal_initialize)
{
   global_terminal_cellx = 0;
   global_terminal_celly = 0;
   global_terminal_cells = (u16 *)0xB8000;

   global_terminal_color = VGA_TEXTMODE_COLOR(WHITE, BLUE);
   // global_terminal_color = VGA_TEXTMODE_COLOR(LIGHTRED, BLACK);

   for(size y = 0; y < VGA_TEXTMODE_HEIGHT; y++)
   {
      for(size x = 0; x < VGA_TEXTMODE_WIDTH; x++)
      {
         size cell_index = (y * VGA_TEXTMODE_WIDTH) + x;
         global_terminal_cells[cell_index] = VGA_TEXTMODE_CELL(' ', global_terminal_color);
      }
   }
}

function void terminal_advance_row(void)
{
   global_terminal_celly++;

   global_terminal_cellx = 0;
   if(global_terminal_celly == VGA_TEXTMODE_HEIGHT)
   {
      global_terminal_celly = 0;
   }
}

function TERMINAL_OUTPUT_BYTE(terminal_output_byte)
{
   if(byte == '\n')
   {
      terminal_advance_row();
   }
   else
   {
      size cell_index = (global_terminal_celly * VGA_TEXTMODE_WIDTH) + global_terminal_cellx++;
      global_terminal_cells[cell_index] = VGA_TEXTMODE_CELL(byte, global_terminal_color);

      if(global_terminal_cellx == VGA_TEXTMODE_WIDTH)
      {
         terminal_advance_row();
      }
   }
}

function TERMINAL_OUTPUT_BYTES(terminal_output_bytes)
{
   for(size index = 0; index < count; index++)
   {
      terminal_output_byte(data[index]);
   }
}

function TERMINAL_OUTPUT_STRINGZ(terminal_output_stringz)
{
   terminal_output_bytes((u8 *)data, strlen(data));
}

function TERMINAL_OUTPUT(terminal_output)
{
   terminal_output_bytes(string.data, string.length);
}
