/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <cdec.h>

function void syntax_error(char *format, ...)
{
   char message[1024];
   va_list arguments;
   va_start(arguments, format);
   {
      vsnprintf(message, sizeof(message), format, arguments);
   }
   va_end(arguments);

   printf("%s", message);

}

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

function s8 load_file(arena *a, char *path)
{
   s8 result = s8("");

   FILE *file = fopen(path, "rb");
   if(file)
   {
      fseek(file, 0, SEEK_END);
      size_t size = ftell(file);
      fseek(file, 0, SEEK_SET);

      u8 *data = arena_allocate(a, u8, size + 1);

      size_t bytes_read = fread(data, 1, size, file);
      assert(bytes_read == size);

      data[size] = 0;

      result = s8new(data, size);

      fclose(file);
   }

   return(result);
}

int main(int argument_count, char **arguments)
{
   if(argument_count == 1)
   {
      printf("USAGE: cdec example.cdec\n");
   }
   else
   {
      arena source_arena = generate_arena(KILOBYTES(64));
      arena token_arena = generate_arena(KILOBYTES(64));
      arena ast_arena = generate_arena(KILOBYTES(64));

      for(int source_index = 1; source_index < argument_count; ++source_index)
      {
         s8 source_text = load_file(&source_arena, arguments[1]);

         lex(&token_arena, &global_tokens, source_text);
         print_token_stream(&global_tokens);

         ast_program program = parse_program(&ast_arena, &global_tokens);
         ast_print_program(&program);

         generate_program(&program);

         arena_reset(&source_arena);
         arena_reset(&token_arena);
      }
   }

   return(0);
}
