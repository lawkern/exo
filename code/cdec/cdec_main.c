/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#include <stdarg.h>
#include <stdlib.h>
#include <cdec.h>

#define syntax_error(format, ...) do { platform_log((format), __VA_ARGS__); exit(1); } while(0)

#define INDENTATION "   "

#include "platform.h"
#include "cdec_lexer.c"
#include "cdec_parser.c"
#include "cdec_codegen.c"

global token_stream global_tokens;

function arena generate_arena(size cap)
{
   u8 *base = malloc(cap);
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
      initialize_token_names();

      arena source_arena = generate_arena(KILOBYTES(64));
      arena token_arena = generate_arena(KILOBYTES(64));
      arena ast_arena = generate_arena(KILOBYTES(64));

      for(int source_index = 1; source_index < argument_count; ++source_index)
      {
         s8 source_text = platform_load_file(&source_arena, arguments[1]);

         lex(&token_arena, &global_tokens, source_text);
         print_token_stream(&global_tokens);

         ast_program program = parse_program(&ast_arena, &global_tokens);
         ast_print_program(&program);

         generate_program(&program);

         arena_reset(&source_arena);
         arena_reset(&token_arena);
         arena_reset(&ast_arena);
      }
   }

   return(0);
}
