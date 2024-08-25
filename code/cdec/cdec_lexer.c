/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#include "cdec_lexer.h"

global string_intern_table global_interns;
global char *global_tokentype_names[TOKENTYPE_COUNT];

function u8 *intern_string(s8 string)
{
   u8 *result = 0;

   // TODO: Replace this with a hash lookup.
   for(size index = 0; index < global_interns.count; index++)
   {
      if(s8equals(string, global_interns.strings[index]))
      {
         result = global_interns.strings[index].data;
         break;
      }
   }

   if(!result)
   {
      assert(global_interns.count < countof(global_interns.strings));

      global_interns.strings[global_interns.count++] = string;
      result = string.data;
   }

   return(result);
}

function u8 *intern_string_size(arena *a, u8 *data, size length)
{
   s8 string = s8allocate(a, data, length);
   u8 *result = intern_string(string);

   return(result);
}

// NOTE: Globally declare language keywords as interned strings.
#define X(keyword) global u8 *KEYWORD_##keyword;
KEYWORDS_NAMES
#undef X

function void initialize_lexer(arena *a)
{
   // NOTE: Initialize the keyword global values with interned strings.
#define X(keyword) KEYWORD_##keyword = intern_string(s8(#keyword));
   KEYWORDS_NAMES;
#undef X

   // // NOTE: Initialize multi-character tokens.
   global_tokentype_names[TOKENTYPE_ASSIGN_COLON]       = ":=";
   global_tokentype_names[TOKENTYPE_ASSIGN_ADD]         = "+=";
   global_tokentype_names[TOKENTYPE_ASSIGN_SUB]         = "-=";
   global_tokentype_names[TOKENTYPE_ASSIGN_MUL]         = "*=";
   global_tokentype_names[TOKENTYPE_ASSIGN_DIV]         = "/=";
   global_tokentype_names[TOKENTYPE_ASSIGN_MOD]         = "%=";
   global_tokentype_names[TOKENTYPE_ASSIGN_AND]         = "&=";
   global_tokentype_names[TOKENTYPE_ASSIGN_OR]          = "|=";
   global_tokentype_names[TOKENTYPE_ASSIGN_XOR]         = "^=";
   global_tokentype_names[TOKENTYPE_ASSIGN_NOT]         = "~=";
   global_tokentype_names[TOKENTYPE_ASSIGN_SHIFT_LEFT]  = "<<=";
   global_tokentype_names[TOKENTYPE_ASSIGN_SHIFT_RIGHT] = ">>=";

   global_tokentype_names[TOKENTYPE_EQ]  = "==";
   global_tokentype_names[TOKENTYPE_NE]  = "!=";
   global_tokentype_names[TOKENTYPE_GTE] = ">=";
   global_tokentype_names[TOKENTYPE_LTE] = "<=";

   global_tokentype_names[TOKENTYPE_INCREMENT]   = "++";
   global_tokentype_names[TOKENTYPE_DECREMENT]   = "--";
   global_tokentype_names[TOKENTYPE_SHIFT_LEFT]  = "<<";
   global_tokentype_names[TOKENTYPE_SHIFT_RIGHT] = ">>";
}

function char *get_tokentype_name(tokentype type)
{
   assert(type < TOKENTYPE_COUNT);

   char *result =  global_tokentype_names[type];
   return(result);
}

function void reset_token_stream(token_stream *tokens)
{
   tokens->index = 0;
   tokens->count = 0;
}

function b32 is_decimal(char c)
{
   return (c >= '0' && c <= '9');
}

function b32 is_alphanumeric(char c)
{
   b32 result = ((c >= '0' && c <= '9') ||
                 (c >= 'a' && c <= 'z') ||
                 (c >= 'A' && c <= 'Z'));

   return(result);
}

function b32 is_whitespace(char c)
{
   b32 result = (c > 0) && (c <= 32);
   return(result);
}

function void lex(arena *a, token_stream *tokens, s8 text)
{
   u8 *stream = text.data;
   while(stream < (text.data + text.length))
   {
      if(is_whitespace(stream[0]))
      {
         stream++;
         continue;
      }

      assert(tokens->count < countof(tokens->tokens));
      token *token = tokens->tokens + tokens->count++;

      size advance = 1;
      switch(stream[0])
      {
         case '{': case '}':
         case '(': case ')':
         case '[': case ']':
         case ';':
         case ',':
         case '.':
         {
            token->type = stream[0];
         } break;

         // NOTE: Handles patterns of the form "*" vs "*=".
#define CASE_PATTERN(t1, t2)                    \
         case t1:                               \
         {                                      \
            if(stream[1] == '=')                \
            {                                   \
               token->type = t2;                \
               advance = 2;                     \
            }                                   \
            else                                \
            {                                   \
               token->type = t1;                \
            }                                   \
         } break

         CASE_PATTERN(':', TOKENTYPE_ASSIGN_COLON);
         CASE_PATTERN('*', TOKENTYPE_ASSIGN_MUL);
         CASE_PATTERN('/', TOKENTYPE_ASSIGN_DIV);
         CASE_PATTERN('%', TOKENTYPE_ASSIGN_MOD);
         CASE_PATTERN('^', TOKENTYPE_ASSIGN_XOR);
         CASE_PATTERN('~', TOKENTYPE_ASSIGN_NOT);
         CASE_PATTERN('=', TOKENTYPE_EQ);
         CASE_PATTERN('!', TOKENTYPE_NE);
#undef CASE_PATTERN

         // NOTE: Handles patterns of the form "&" vs "&&" vs "&=".
#define CASE_PATTERN(t1, t2, ta)                \
         case t1:                               \
         {                                      \
            if(stream[1] == t1)                 \
            {                                   \
               token->type = t2;                \
               advance = 2;                     \
            }                                   \
            else if(stream[1] == '=')           \
            {                                   \
               token->type = ta;                \
               advance = 2;                     \
            }                                   \
            else                                \
            {                                   \
               token->type = t1;                \
            }                                   \
         } break

         CASE_PATTERN('&', TOKENTYPE_AND, TOKENTYPE_ASSIGN_AND);
         CASE_PATTERN('|', TOKENTYPE_OR, TOKENTYPE_ASSIGN_OR);
         CASE_PATTERN('+', TOKENTYPE_INCREMENT, TOKENTYPE_ASSIGN_ADD);
         CASE_PATTERN('-', TOKENTYPE_DECREMENT, TOKENTYPE_ASSIGN_SUB);
#undef CASE_PATTERN

         // NOTE: Handles patterns of the form "<" vs "<<" vs "<<=".
#define CASE_PATTERN(t1, t2, ta)                \
         case t1:                               \
         {                                      \
            if(stream[1] == t1)                 \
            {                                   \
               if(stream[2] == '=')             \
               {                                \
                  token->type = ta;             \
                  advance = 3;                  \
               }                                \
               else                             \
               {                                \
                  token->type = t2;             \
                  advance = 2;                  \
               }                                \
            }                                   \
            else                                \
            {                                   \
               token->type = t1;                \
            }                                   \
         } break

         CASE_PATTERN('<', TOKENTYPE_SHIFT_LEFT, TOKENTYPE_ASSIGN_SHIFT_LEFT);
         CASE_PATTERN('>', TOKENTYPE_SHIFT_RIGHT, TOKENTYPE_ASSIGN_SHIFT_RIGHT);
#undef CASE_PATTERN

         case '"':
         {
            token->type = TOKENTYPE_STRING;

            u8 *start = stream + 1;
            size length = 0;
            while(start[length] != '"')
            {
               length++;
            }
            token->value_string = intern_string_size(a, start, length);

            advance = length + 2;
         } break;

         case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h':
         case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p':
         case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x':
         case 'y': case 'z':
         case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H':
         case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
         case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
         case 'Y': case 'Z':
         case '_':
         {
            token->type = TOKENTYPE_IDENTIFIER;

            u8 *start = stream;
            size length = 0;
            while(is_alphanumeric(stream[length]) || stream[length] == '_')
            {
               length++;
            }
            token->name = intern_string_size(a, start, length);

            advance = length;
         } break;

         case '0':
         case '1':
         case '2':
         case '3':
         case '4':
         case '5':
         case '6':
         case '7':
         case '8':
         case '9':
         {
            token->type = TOKENTYPE_INTEGER;

            size length = 0;
            u64 value = 0;

            while(is_decimal(stream[length]))
            {
               value *= 10;
               value += (u64)(stream[length] - '0');
               length++;
            }
            token->value_integer = value;

            advance = length;
         } break;


         default: { token->type = 0; } break;
      }

      stream += advance;
   }
}

function token peek_token(token_stream *tokens)
{
   token result = {0};

   if(tokens->index < tokens->count)
   {
      result = tokens->tokens[tokens->index];
   }

   return(result);
}

function token advance_token(token_stream *tokens)
{
   assert(tokens->index < tokens->count);
   token result = tokens->tokens[tokens->index++];
   return(result);
}

function b32 is_token(token_stream *tokens, tokentype type)
{
   b32 result = (peek_token(tokens).type == type);
   return(result);
}

function b32 match_token(token_stream *tokens, tokentype type)
{
   b32 result = (peek_token(tokens).type == type);
   if(result)
   {
      advance_token(tokens);
   }
   return(result);
}

function token expect_token(token_stream *tokens, tokentype type)
{
   token next = advance_token(tokens);
   if(next.type != type)
   {
      syntax_error("Expected token of type %d, got type %d.", type, next.type);
   }
   return(next);
}

function token expect_token_name(token_stream *tokens, tokentype type, u8 *name)
{
   token next = advance_token(tokens);
   if(next.type != type)
   {
      syntax_error("Expected token of type %d, got type %d.", type, next.type);
   }
   else if(next.name != name)
   {
      syntax_error("Expected token of name %s, got %s.", name, next.name);
   }
   return(next);
}
