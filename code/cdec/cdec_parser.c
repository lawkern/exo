/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#include "cdec_parser.h"

function ast_identifier *parse_identifer(arena *a, token_stream *tokens)
{
   ast_identifier *result = arena_allocate_size(a, sizeof(*result));

   token identifier = expect_token(tokens, TOKENTYPE_IDENTIFIER);
   result->name = identifier.name;

   return(result);
}

function ast_expression *parse_arguments(arena *, token_stream *);

function ast_expression *parse_expression(arena *a, token_stream *tokens)
{
   ast_expression *result = 0;

   token first = peek_token(tokens);
   if(first.type == '(')
   {
      next_token(tokens);
      result = parse_expression(a, tokens);
      expect_token(tokens, ')');
   }
   else
   {
      result = arena_allocate(a, ast_expression, 1);

      if(first.type == TOKENTYPE_INTEGER)
      {
         next_token(tokens);
         result->type = ASTEXPRESSION_LITERAL_INTEGER;
         result->literal_integer.value = first.value_integer;
      }
      else if(first.type == TOKENTYPE_STRING)
      {
         next_token(tokens);
         result->type = ASTEXPRESSION_LITERAL_STRING;
         result->literal_string.value = first.value_string;
      }
      else if(first.type == TOKENTYPE_IDENTIFIER)
      {
         result->type = ASTEXPRESSION_FUNCTIONCALL;
         result->name = parse_identifer(a, tokens);
         result->arguments = parse_arguments(a, tokens);
      }
      else if(first.type == '~' || first.type == '-')
      {
         result->type = ASTEXPRESSION_OPERATION_UNARY;
         result->operator = first.type;
         result->expression = parse_expression(a, tokens);
      }
      else
      {
         syntax_error("UNHANDLED EXPRESSION TYPE.");
      }
   }

   return(result);
}

function ast_expression *parse_arguments(arena *a, token_stream *tokens)
{
   ast_expression *result = 0;

   expect_token(tokens, '(');

   ast_expression **argument = &result;
   while(peek_token(tokens).type != ')')
   {
      *argument = parse_expression(a, tokens);
      argument = &(*argument)->next;
   }

   expect_token(tokens, ')');

   return(result);
}

function ast_statement *parse_statement_block(arena *, token_stream *);

function ast_statement *parse_statement(arena *a, token_stream *tokens)
{
   ast_statement *result = arena_allocate(a, ast_statement, 1);

   token first = peek_token(tokens);
   switch(first.type)
   {
      case TOKENTYPE_KEYWORD_RETURN:
      {
         next_token(tokens);
         result->type = ASTSTATEMENT_RETURN;
         result->result = parse_expression(a, tokens);
         expect_token(tokens, ';');
      } break;

      case TOKENTYPE_KEYWORD_IF:
      {
         next_token(tokens);
         result->type = ASTSTATEMENT_IF;
         expect_token(tokens, '(');
         result->condition = parse_expression(a, tokens);
         expect_token(tokens, ')');

         result->body = parse_statement_block(a, tokens);
      } break;

      case TOKENTYPE_KEYWORD_FOR:
      {
         next_token(tokens);
         result->type = ASTSTATEMENT_FOR;
         expect_token(tokens, '(');
         result->condition = parse_expression(a, tokens);
         expect_token(tokens, ')');

         result->body = parse_statement_block(a, tokens);
      } break;

      case TOKENTYPE_KEYWORD_VAR:
      {
         next_token(tokens);
         result->type = ASTSTATEMENT_VAR;
         result->identifier = parse_identifer(a, tokens);
         result->typename = parse_identifer(a, tokens);
         expect_token(tokens, '=');
         result->result = parse_expression(a, tokens);
         expect_token(tokens, ';');
      } break;

      case TOKENTYPE_KEYWORD_IMPORT:
      {
         next_token(tokens);
         result->type = ASTSTATEMENT_IMPORT;
         result->identifier = parse_identifer(a, tokens);
         expect_token(tokens, ';');
      } break;

      default:
      {
         result->type = ASTSTATEMENT_EXPRESSION;
         result->result = parse_expression(a, tokens);
         expect_token(tokens, ';');
      } break;

      // default:
      // {
      //    syntax_error("UNHANDLED STATEMENT TYPE");
      // } break;
   }

   return(result);
}

function ast_statement *parse_statement_block(arena *a, token_stream *tokens)
{
   ast_statement *result = 0;

   expect_token(tokens, '{');

   ast_statement **statement = &result;
   while(peek_token(tokens).type != '}')
   {
      *statement = parse_statement(a, tokens);
      statement = &(*statement)->next;
   }

   expect_token(tokens, '}');

   return(result);
}

function ast_statement *parse_imports(arena *a, token_stream *tokens)
{
   ast_statement *result = 0;

   ast_statement **import = &result;
   while(peek_token(tokens).type == TOKENTYPE_KEYWORD_IMPORT)
   {
      *import = parse_statement(a, tokens);
      import = &(*import)->next;
   }

   return(result);
}

function ast_parameter *parse_parameter(arena *a, token_stream *tokens)
{
   ast_parameter *result = arena_allocate_size(a, sizeof(*result));
   result->name = parse_identifer(a, tokens);
   result->type = parse_identifer(a, tokens);

   return(result);
}

function ast_parameter *parse_parameters(arena *a, token_stream *tokens)
{
   ast_parameter *result = 0;

   expect_token(tokens, '(');
   ast_parameter **parameter = &result;
   while(peek_token(tokens).type != ')')
   {
      *parameter = parse_parameter(a, tokens);
      if(peek_token(tokens).type != ')')
      {
         expect_token(tokens, ',');
      }
   }
   expect_token(tokens, ')');

   return(result);
}

function ast_function *parse_function(arena *a, token_stream *tokens)
{
   ast_function *result = arena_allocate(a, ast_function, 1);

   expect_token(tokens, TOKENTYPE_KEYWORD_FUNCTION);

   token function_name = expect_token(tokens, TOKENTYPE_IDENTIFIER);
   result->name = function_name.name;
   result->parameters = parse_parameters(a, tokens);

   // NOTE: Optional return type.
   token return_type = peek_token(tokens);
   if(return_type.type == TOKENTYPE_IDENTIFIER)
   {
      result->return_type = parse_identifer(a, tokens);
   }

   result->body = parse_statement_block(a, tokens);

   return(result);
}

function ast_function *parse_functions(arena *a, token_stream *tokens)
{
   ast_function *result = 0;

   ast_function **func = &result;
   while(peek_token(tokens).type == TOKENTYPE_KEYWORD_FUNCTION)
   {
      *func = parse_function(a, tokens);
      func = &(*func)->next;
   }

   return(result);
}

function ast_program parse_program(arena *a, token_stream *tokens)
{
   ast_program result = {0};

   result.imports = parse_imports(a, tokens);
   result.functions = parse_functions(a, tokens);

   return(result);
}
