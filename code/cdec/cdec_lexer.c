/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

typedef enum {
   TOKENTYPE_UNKNOWN = 0,

   // ASCII Character Literals.

   TOKENTYPE_KEYWORD_FUNCTION = 256,
   TOKENTYPE_KEYWORD_RETURN,

   TOKENTYPE_IDENTIFIER,
   TOKENTYPE_INTEGER,
   TOKENTYPE_STRING,

   TOKENTYPE_COLONASSIGNMENT,
   TOKENTYPE_INCREMENT,
   TOKENTYPE_DECREMENT,

   TOKENTYPE_COUNT,
} tokentype;

global s8 token_names[] = {
   [TOKENTYPE_KEYWORD_FUNCTION] = s8("function"),
   [TOKENTYPE_KEYWORD_RETURN]	= s8("return"),
   [TOKENTYPE_COLONASSIGNMENT]	= s8(":="),
   [TOKENTYPE_INCREMENT]		= s8("++"),
   [TOKENTYPE_DECREMENT]		= s8("--"),
};

typedef struct {
   tokentype type;
   union
   {
      s8 name;
      s8 value_string;
      u64 value_integer;
   };
} token;

typedef struct {
   u32 index;
   u32 count;
   token tokens[2048];
} token_stream;

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

function u64 s8_to_u64(s8 string)
{
   u64 result = 0;

   u64 place = 1;
   for(size index = 0; index < string.length; index++)
   {
      u64 digit = (u64)(string.data[index] - '0');

      result *= ((10 * index) + 1);
      result += digit;
   }

   return(result);
}

function void lex(arena *a, token_stream *tokens, s8 text)
{
   u8 *scan = text.data;
   while(scan < (text.data + text.length))
   {
      if(is_whitespace(*scan))
      {
         scan++;
         continue;
      }

      assert(tokens->count < countof(tokens->tokens));
      token *token = tokens->tokens + tokens->count++;

      size advance = 1;
      switch(scan[0])
      {
         case ':':
         {
            if(scan[1] == '=')
            {
               token->type = TOKENTYPE_COLONASSIGNMENT;
               advance = 2;
            }
            else
            {
               token->type = *scan;
            }
         } break;

         case '+': case '-':
         {
            if(scan[0] == scan[1])
            {
               token->type = (scan[0] == '+') ? TOKENTYPE_INCREMENT : TOKENTYPE_DECREMENT;
               advance = 2;
            }
            else
            {
               token->type = scan[0];
            }
         } break;

         case '{': case '}':
         case '(': case ')':
         case '[': case ']':
         case '*': case '/':
         case '<': case '>':
         case '!': case '%': case '^':
         case '&': case '|': case '~':
         case ';': case ',': case '.':
         {
            token->type = scan[0];
         } break;

         case '"':
         {
            token->type = TOKENTYPE_STRING;

            u8 *start = scan + 1;
            size length = 0;
            while(start[length] != '"')
            {
               length++;
            }
            token->value_string = s8allocate(a, start, length);

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

            u8 *start = scan;
            size length = 0;
            while(is_alphanumeric(scan[length]) || scan[length] == 0)
            {
               length++;
            }
            token->name = s8allocate(a, start, length);

            // TODO: Handle this with string interning, or some other kind of faster lookup.
            if(s8equals(token->name, s8("function")))
            {
               token->type = TOKENTYPE_KEYWORD_FUNCTION;
            }
            else if(s8equals(token->name, s8("return")))
            {
               token->type = TOKENTYPE_KEYWORD_RETURN;
            }

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

            u8 *start = scan;
            size length = 0;
            while(is_decimal(scan[length]))
            {
               length++;
            }
            // token->value_string = s8allocate(a, start, length);
            token->value_integer = s8_to_u64(s8new(start, length));

            advance = length;
         } break;


         default: {token->type = TOKENTYPE_UNKNOWN;} break;
      }

      scan += advance;
   }
}

function void print_token(token *token)
{
   printf("\"");
   if(token->type == TOKENTYPE_INTEGER)
   {
      printf("%lld", token->value_integer);
   }
   else if(token->type == TOKENTYPE_STRING)
   {
      printf("%s", token->value_string.data);
   }
   else if(token_names[token->type].length > 0)
   {
      printf("%s", token_names[token->type].data);
   }
   else if(token->value_string.length == 0)
   {
      printf("%c", token->type);
   }
   else
   {
      printf("UNHANDLED TOKENTYPE");
   }
   printf("\", ");
   // printf("\n");
}

function void print_token_stream(token_stream *tokens)
{
   printf("TOKEN STREAM:\n");
   for(u32 token_index = 0; token_index < tokens->count; ++token_index)
   {
      token *token = tokens->tokens + token_index;
      print_token(token);
   }
   printf("\n\n");
}

function token peek_token(token_stream *tokens)
{
   assert(tokens->index < tokens->count);
   token result = tokens->tokens[tokens->index];
   return(result);
}

function token next_token(token_stream *tokens)
{
   assert(tokens->index < tokens->count);
   token result = tokens->tokens[tokens->index++];
   return(result);
}

function token expect_token(token_stream *tokens, tokentype type)
{
   token next = next_token(tokens);
   if(next.type != type)
   {
      syntax_error("Expected token of type %d, got type %d.", type, next.type);
   }
   return(next);
}
