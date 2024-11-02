#pragma once

/* ////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern ////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */

// NOTE: This is the kernel's terminal API. Any system that wants to implement
// its own terminal just needs to implement these functions, using the macros to
// generate their signatures.

#define TERMINAL_INITIALIZE(name)     void name(void)
#define TERMINAL_OUTPUT_BYTE(name)    void name(u8 byte)
#define TERMINAL_OUTPUT_BYTES(name)   void name(u8 *data, size count)
#define TERMINAL_OUTPUT_STRINGZ(name) void name(char *data)
#define TERMINAL_OUTPUT(name)         void name(s8 string)

function TERMINAL_INITIALIZE(terminal_initialize);
function TERMINAL_OUTPUT_BYTE(terminal_output_byte);
function TERMINAL_OUTPUT_BYTES(terminal_output_bytes);
function TERMINAL_OUTPUT_STRINGZ(terminal_output_stringz);
function TERMINAL_OUTPUT(terminal_output);
