/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#include <stdarg.h>
#include <cdec.h>

#define syntax_error(format, ...) do { platform_log("SYNTAX ERROR: " format, ##__VA_ARGS__); assert(0); } while(0)

global arena text_arena;
global arena string_arena;
global arena token_arena;
global arena ast_arena;

#include "platform.h"
#include "cdec_lexer.c"
#include "cdec_parser.c"
#include "cdec_codegen.c"
#include "cdec_printer.c"

function arena generate_arena(size cap)
{
   u8 *base = platform_allocate(cap);
   assert(base);

   arena result;
   arena_initialize(&result, base, cap);

   return(result);
}

function text_stream generate_text_stream(arena *a, char *path)
{
   s8 file = platform_load_file(a, path);

   text_stream result = {0};
   result.count = file.length;
   result.characters = (char *)file.data;

   return(result);
}

int main(int argument_count, char **arguments)
{
   if(argument_count == 1)
   {
      platform_log("USAGE: cdec example.cdec\n");
   }
   else
   {
      text_arena   = generate_arena(KILOBYTES(64));
      string_arena = generate_arena(KILOBYTES(64));
      token_arena  = generate_arena(MEGABYTES(1));
      ast_arena    = generate_arena(KILOBYTES(64));

      // NOTE: Initialize the keyword global values with interned strings.
#define X(keyword) keyword_##keyword = intern_string_length(#keyword, lengthof(#keyword));
      KEYWORDS_NAMES;
#undef X

      for(int source_file_index = 1; source_file_index < argument_count; source_file_index++)
      {
         char *path = arguments[source_file_index];
         platform_log("COMPILING CDEC SOURCE FILE: %s\n", path);

         text_stream text = generate_text_stream(&text_arena, path);

         // NOTE: Tokenize source code text stream.
         lex(text);
         print_token_stream();

         // NOTE: Parse tokens to generate AST.
         ast_program program = parse_program(&global_tokens);
         ast_print_program(&program);

         // NOTE: Generate assembly code from AST.
         generate_asm_program(&program);

         // NOTE: The text and token arenas can be flushed between source code
         // files. Just the string table and AST should stick around.
         arena_reset(&text_arena);
         arena_reset(&token_arena);
      }
   }

   return(0);
}
