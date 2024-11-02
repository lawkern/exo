/* ////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern ////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */

#include <cdec.h>
#include <libc.h>

#include "terminal_i386.c"

#if defined(__linux__) || defined(_WIN32)
#   error "Make sure to use a cross-compiler to build the kernel."
#endif

#if !defined(__i386__)
#   error "Use an ix86 compiler for now."
#endif

void kernel_main(void)
{
   terminal_initialize();

   terminal_output_stringz("exokernel v0.0\n");
   terminal_output_stringz("==============\n");
   terminal_output_stringz("Compiled on " __DATE__ " at " __TIME__ "\n");
   terminal_output_stringz("\n");

   terminal_output_stringz("program> ");
}
