/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#include "compiler_lexer.h"

global string_table global_strings;
global token_stream global_tokens;

// NOTE: Globally declare language keywords as interned strings.
#define X(keyword) global char *keyword_##keyword;
KEYWORDS_NAMES
#undef X

// NOTE: This is for C string literals only.
#define intern_stringz(s) intern_string_length((s), lengthof(s))

function char *intern_string_length(char *data, size length)
{
   char *result = 0;

   s8 string = {(u8 *)data, length};

   // TODO: Replace this with a hash lookup.
   for(size index = 0; index < global_strings.count; index++)
   {
      s8 compare = global_strings.strings[index];
      if(s8equals(string, compare))
      {
         result = (char *)compare.data;
         break;
      }
   }

   if(!result)
   {
      s8 s8result = s8allocate(&string_arena, (u8 *)data, length);
      global_strings.strings[global_strings.count++] = s8result;
      result = (char *)s8result.data;
   }

   return(result);
}

function b32 is_keyword(char *keyword)
{
   b32 result = (keyword >= keyword_begin && keyword <= keyword_end);
   return(result);
}

function char *get_tokentype_name(tokentype type)
{
   char *result = "UNKNOWN TOKEN";
   if(type < TOKENTYPE_COUNT)
   {
      result = tokentype_names[type];
   }
   return(result);
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

function b32 is_newline(char c)
{
   b32 result = (c == '\n' || c == '\r');
   return(result);
}

function void consume_whitespace(text_stream *text)
{
   while(text->characters[0])
   {
      if(is_whitespace(text->characters[0]))
      {
         text->characters++;
      }
      else if(text->characters[0] == '/' && text->characters[1] && text->characters[1] == '/')
      {
         text->characters += 2;
         while(text->characters[0] && !is_newline(text->characters[0]))
         {
            text->characters++;
         }
      }
      else
      {
         break;
      }
   }
}2

function lexical_token get_token(text_stream *text)
{
   lexical_token result = {0};

   consume_whitespace(text);

   size advance = 1;
   switch(text->characters[0])
   {
      case '\0': { result.type = TOKENTYPE_ENDOFSTREAM; } break;

      case '{': { result.type = TOKENTYPE_OPENBRACE; } break;
      case '}': { result.type = TOKENTYPE_CLOSEBRACE; } break;
      case '(': { result.type = TOKENTYPE_OPENPAREN; } break;
      case ')': { result.type = TOKENTYPE_CLOSEPAREN; } break;
      case '[': { result.type = TOKENTYPE_OPENBRACE; } break;
      case ']': { result.type = TOKENTYPE_CLOSEBRACE; } break;
      case ';': { result.type = TOKENTYPE_SEMICOLON; } break;
      case ',': { result.type = TOKENTYPE_COMMA; } break;
      case '.': { result.type = TOKENTYPE_PERIOD; } break;

         // NOTE: Handles patterns of the form "*" vs "*=".
#define CASE_PATTERN(c, t1, t2)                                     \
         case c:                                                    \
         {                                                          \
            if(text->characters[1] && text->characters[1] == '=')   \
            {                                                       \
               result.type = t2;                                    \
               advance = 2;                                         \
            }                                                       \
            else                                                    \
            {                                                       \
               result.type = t1;                                    \
            }                                                       \
         } break

         CASE_PATTERN(':', TOKENTYPE_COLON, TOKENTYPE_ASSIGN_COLON);
         CASE_PATTERN('*', TOKENTYPE_MUL, TOKENTYPE_ASSIGN_MUL);
         CASE_PATTERN('/', TOKENTYPE_DIV, TOKENTYPE_ASSIGN_DIV);
         CASE_PATTERN('%', TOKENTYPE_MOD, TOKENTYPE_ASSIGN_MOD);
         CASE_PATTERN('^', TOKENTYPE_BITWISE_XOR, TOKENTYPE_ASSIGN_XOR);
         CASE_PATTERN('~', TOKENTYPE_BITWISE_NOT, TOKENTYPE_ASSIGN_NOT);
         CASE_PATTERN('=', TOKENTYPE_ASSIGN, TOKENTYPE_EQ);
         CASE_PATTERN('!', TOKENTYPE_LOGICAL_NOT, TOKENTYPE_NE);
#undef CASE_PATTERN

         // NOTE: Handles patterns of the form "&" vs "&&" vs "&=".
#define CASE_PATTERN(c, t1, t2, t3)                                     \
         case c:                                                        \
         {                                                              \
            if(text->characters[1] && text->characters[1] == '=')		\
            {                                                           \
               result.type = t3;                                        \
               advance = 2;                                             \
            }                                                           \
            else if(text->characters[1] && text->characters[1] == c)	\
            {                                                           \
               result.type = t2;                                        \
               advance = 2;                                             \
            }                                                           \
            else                                                        \
            {                                                           \
               result.type = t1;                                        \
            }                                                           \
         } break

         CASE_PATTERN('&', TOKENTYPE_BITWISE_AND, TOKENTYPE_LOGICAL_AND, TOKENTYPE_ASSIGN_AND);
         CASE_PATTERN('|', TOKENTYPE_BITWISE_OR, TOKENTYPE_LOGICAL_OR, TOKENTYPE_ASSIGN_OR);
         CASE_PATTERN('+', TOKENTYPE_ADD, TOKENTYPE_INCREMENT, TOKENTYPE_ASSIGN_ADD);
         CASE_PATTERN('-', TOKENTYPE_SUB, TOKENTYPE_DECREMENT, TOKENTYPE_ASSIGN_SUB);
#undef CASE_PATTERN

         // NOTE: Handles patterns of the form "<" vs "<=" vs "<<" "<<=".
#define CASE_PATTERN(c, t1, t2, t3, t4)                                 \
         case c:                                                        \
         {                                                              \
            if(text->characters[1] && text->characters[1] == c)         \
            {                                                           \
               if(text->characters[2] && text->characters[2] == '=')    \
               {                                                        \
                  result.type = t4;                                     \
                  advance = 3;                                          \
               }                                                        \
               else                                                     \
               {                                                        \
                  result.type = t3;                                     \
                  advance = 2;                                          \
               }                                                        \
            }                                                           \
            else                                                        \
            {                                                           \
               if(text->characters[1] && text->characters[1] == '=')    \
               {                                                        \
                  result.type = t2;                                     \
               }                                                        \
               else                                                     \
               {                                                        \
                  result.type = t1;                                     \
               }                                                        \
            }                                                           \
         } break

         CASE_PATTERN('<', TOKENTYPE_LT, TOKENTYPE_LTE, TOKENTYPE_SHIFT_LEFT, TOKENTYPE_ASSIGN_SHIFT_LEFT);
         CASE_PATTERN('>', TOKENTYPE_GT, TOKENTYPE_GTE, TOKENTYPE_SHIFT_RIGHT, TOKENTYPE_ASSIGN_SHIFT_RIGHT);
#undef CASE_PATTERN

      case '"':
      {
         result.type = TOKENTYPE_STRING;

         char *start = text->characters + 1;
         size length = 0;
         while(start[length] && start[length] != '"')
         {
            length++;
         }
         result.value_string = intern_string_length(start, length);

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
         char *start = text->characters;
         size length = 0;
         while(text->characters[length] && (is_alphanumeric(text->characters[length]) || text->characters[length] == '_'))
         {
            length++;
         }
         result.name = intern_string_length(start, length);
         result.type = is_keyword(result.name) ? TOKENTYPE_KEYWORD : TOKENTYPE_NAME;

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
         result.type = TOKENTYPE_INTEGER;

         size length = 0;
         u64 value = 0;

         while(text->characters[length] && is_decimal(text->characters[length]))
         {
            value *= 10;
            value += (u64)(text->characters[length] - '0');
            length++;
         }
         result.value_integer = value;

         advance = length;
      } break;

      default: { result.type = 0; } break;
   }

   text->characters += advance;

   return(result);
}

function void lex(text_stream text)
{
   for(;;)
   {
      lexical_token token = get_token(&text);
      if(token.type == TOKENTYPE_ENDOFSTREAM)
      {
         break;
      }

      global_tokens.tokens[global_tokens.count++] = token;
   }
}

function lexical_token peek_token(token_stream *tokens)
{
   lexical_token result = {0};
   if(tokens->index < tokens->count)
   {
      result = tokens->tokens[tokens->index];
   }
   return(result);
}

function lexical_token advance_token(token_stream *tokens)
{
   assert(tokens->index < tokens->count);
   lexical_token result = tokens->tokens[tokens->index++];
   return(result);
}

function lexical_token rewind_token(token_stream *tokens)
{
   assert(tokens->index > 0);
   lexical_token result = tokens->tokens[--tokens->index];
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

function lexical_token expect_token(token_stream *tokens, tokentype type)
{
   lexical_token next = advance_token(tokens);
   if(next.type != type)
   {
      syntax_error("Expected token of type %s, got type %s.", get_tokentype_name(type), get_tokentype_name(next.type));
   }
   return(next);
}

function lexical_token expect_token_name(token_stream *tokens, tokentype type, char *name)
{
   lexical_token next = advance_token(tokens);
   if(next.type != type)
   {
      syntax_error("Expected token of type %s, got type %s.", get_tokentype_name(type), get_tokentype_name(next.type));
   }
   else if(next.name != name)
   {
      syntax_error("Expected token of name %s, got %s.", name, next.name);
   }
   return(next);
}
