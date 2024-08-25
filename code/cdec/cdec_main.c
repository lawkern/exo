/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#include <stdarg.h>
#include <stdlib.h>
#include <cdec.h>

#define syntax_error(format, ...) do { platform_log("SYNTAX ERROR: " format, ##__VA_ARGS__); assert(0); } while(0)

#include "platform.h"
#include "cdec_lexer.c"
#include "cdec_parser.c"
#include "cdec_codegen.c"
#include "cdec_printer.c"

global token_stream global_tokens;

function arena generate_arena(size cap)
{
   u8 *base = platform_allocate(cap);
   assert(base);

   arena result;
   arena_initialize(&result, base, cap);

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
      arena source_arena = generate_arena(KILOBYTES(64));
      arena token_arena  = generate_arena(KILOBYTES(64));
      arena ast_arena    = generate_arena(KILOBYTES(64));

      initialize_lexer(&token_arena);

      for(int source_index = 1; source_index < argument_count; ++source_index)
      {
         char *source_path = arguments[source_index];
         platform_log("COMPILING CDEC SOURCE FILE: %s\n", source_path);

         s8 source_text = platform_load_file(&source_arena, source_path);
         lex(&token_arena, &global_tokens, source_text);
         print_token_stream(&global_tokens);

         ast_program program = parse_program(&ast_arena, &global_tokens);
         ast_print_program(&program);

         generate_asm_program(&program);

         arena_reset(&source_arena);
         arena_reset(&token_arena);
         arena_reset(&ast_arena);
         reset_token_stream(&global_tokens);

         platform_log("------------------------------------------------\n\n");
      }
   }

   return(0);
}
