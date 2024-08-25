/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

typedef enum {
   ASTEXPRESSION_LITERAL_INTEGER,
   ASTEXPRESSION_LITERAL_STRING,
   ASTEXPRESSION_OPERATION_UNARY,
   ASTEXPRESSION_OPERATION_BINARY,
} ast_expression_type;

typedef struct {
   s8 name;
} ast_identifier;

typedef struct {
   u64 value;
} ast_literal_integer;

typedef struct {
   s8 value;
} ast_literal_string;

typedef struct ast_expression {
   ast_expression_type type;
   union
   {
      ast_literal_integer literal_integer;
      ast_literal_string literal_string;
      struct
      {
         tokentype operator;
         struct ast_expression *expression;
         struct ast_expression *expression2;
      };
   };
} ast_expression;

typedef enum {
   ASTSTATEMENT_RETURN,
} ast_statement_type;

typedef struct {
   ast_statement_type type;
   ast_expression *result;
} ast_statement;

typedef struct ast_function {
   s8 name;
   ast_identifier return_type;
   ast_statement *body;

   struct ast_function *next;
} ast_function;

typedef struct {
   ast_function *functions;
} ast_program;

function ast_identifier parse_identifer(token_stream *tokens)
{
   ast_identifier result = {0};

   token identifier = expect_token(tokens, TOKENTYPE_IDENTIFIER);
   result.name = identifier.name;

   return(result);
}

function ast_expression *parse_expression(arena *a, token_stream *tokens)
{
   ast_expression *result = 0;

   token next = next_token(tokens);
   if(next.type == '(')
   {
      result = parse_expression(a, tokens);
      expect_token(tokens, ')');
   }
   else
   {
      result = arena_allocate(a, ast_expression, 1);

      if(next.type == TOKENTYPE_INTEGER)
      {
         result->type = ASTEXPRESSION_LITERAL_INTEGER;
         result->literal_integer.value = next.value_integer;
      }
      else if(next.type == TOKENTYPE_STRING)
      {
         result->type = ASTEXPRESSION_LITERAL_STRING;
         result->literal_string.value = next.value_string;
      }
      else if(next.type == '~' || next.type == '-')
      {
         result->type = ASTEXPRESSION_OPERATION_UNARY;
         result->operator = next.type;
         result->expression = parse_expression(a, tokens);
      }
      else
      {
         syntax_error("UNHANDLED EXPRESSION TYPE.");
      }
   }

   return(result);
}

function ast_statement *parse_statement(arena *a, token_stream *tokens)
{
   ast_statement *result = arena_allocate(a, ast_statement, 1);

   expect_token(tokens, TOKENTYPE_KEYWORD_RETURN);
   result->result = parse_expression(a, tokens);
   expect_token(tokens, ';');

   return(result);
}

function ast_function *parse_function(arena *a, token_stream *tokens)
{
   ast_function *result = arena_allocate(a, ast_function, 1);

   expect_token(tokens, TOKENTYPE_KEYWORD_FUNCTION);

   token function_name = expect_token(tokens, TOKENTYPE_IDENTIFIER);
   result->name = function_name.name;

   expect_token(tokens, '(');
   expect_token(tokens, ')');

   // NOTE: Optional return type.
   token return_type = peek_token(tokens);
   if(return_type.type == TOKENTYPE_IDENTIFIER)
   {
      result->return_type = parse_identifer(tokens);
   }

   expect_token(tokens, '{');
   result->body = parse_statement(a, tokens);
   expect_token(tokens, '}');

   return(result);
}

function ast_program parse_program(arena *a, token_stream *tokens)
{
   ast_program result = {0};

   result.functions = parse_function(a, tokens);

   return(result);
}

function void print_indentation(u32 indent_level)
{
   for(u32 index = 0; index < (indent_level); index++)
   {
      platform_log(INDENTATION);
   }
}

function void ast_print_expression(ast_expression *expression, u32 indent_level)
{
   print_indentation(indent_level);

   platform_log("EXPRESSION: ");
   switch(expression->type)
   {
      case ASTEXPRESSION_LITERAL_INTEGER:
      {
         platform_log("%lld\n", expression->literal_integer.value);
      } break;

      case ASTEXPRESSION_LITERAL_STRING:
      {
         platform_log("%s\n", expression->literal_string.value.data);
      } break;

      case ASTEXPRESSION_OPERATION_UNARY:
      {
         platform_log("%c\n", expression->operator);
         ast_print_expression(expression->expression, indent_level);
      } break;

      case ASTEXPRESSION_OPERATION_BINARY:
      {
         platform_log("%c\n", expression->operator);
         ast_print_expression(expression->expression, indent_level);
         ast_print_expression(expression->expression2, indent_level);
      } break;
   }
}

function void ast_print_statement(ast_statement *statement, u32 indent_level)
{
   print_indentation(indent_level);

   platform_log("STATEMENT: ");
   switch(statement->type)
   {
      case ASTSTATEMENT_RETURN:
      {
         platform_log("return\n");
         ast_print_expression(statement->result, indent_level + 1);
      } break;

      default:
      {
         platform_log("UNHANDLED EXPRESSION TYPE %d", statement->type);
      } break;
   }
   platform_log("\n");
}

function void ast_print_function(ast_function *func, u32 indent_level)
{
   print_indentation(indent_level);

   platform_log("FUNCTION: %s -> %s\n", func->name.data, func->return_type.name.data);
   ast_print_statement(func->body, indent_level + 1);
}

function void ast_print_program(ast_program *program)
{
   platform_log("PROGRAM AST:\n");

   ast_function *func = program->functions;
   while(func)
   {
      ast_print_function(program->functions, 1);
      func = program->functions->next;
   }

   platform_log("\n");
}
