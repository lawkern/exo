/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#include "cdec_parser.h"

function u8 *parse_identifer(token_stream *tokens)
{
   token identifier = expect_token(tokens, TOKENTYPE_IDENTIFIER);
   u8 *result = identifier.name;

   return(result);
}

function ast_expression *parse_arguments(arena *, token_stream *);

function ast_expression *parse_expression(arena *a, token_stream *tokens)
{
   ast_expression *result = 0;

   if(match_token(tokens, '('))
   {
      result = parse_expression(a, tokens);
      expect_token(tokens, ')');
   }
   else
   {
      result = arena_allocate(a, ast_expression, 1);

      token token = peek_token(tokens);
      if(token.type == TOKENTYPE_INTEGER)
      {
         result->type = ASTEXPRESSION_LITERAL_INTEGER;
         result->value_integer = token.value_integer;
         advance_token(tokens);
      }
      else if(token.type == TOKENTYPE_STRING)
      {
         result->type = ASTEXPRESSION_LITERAL_STRING;
         result->value_string = token.value_string;
         advance_token(tokens);
      }
      else if(token.type == TOKENTYPE_IDENTIFIER)
      {
         result->type = ASTEXPRESSION_FUNCTIONCALL;
         result->name = parse_identifer(tokens);
         result->arguments = parse_arguments(a, tokens);
      }
      else if(token.type == '~' || token.type == '-')
      {
         result->type = ASTEXPRESSION_OPERATION_UNARY;
         result->operator = token.type;
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
      case TOKENTYPE_IDENTIFIER:
      {
         if(first.name == KEYWORD_struct || first.name == KEYWORD_union || first.name == KEYWORD_enum)
         {
            advance_token(tokens);
            result->name = parse_identifer(tokens);

            expect_token(tokens, '{');
            while(peek_token(tokens).type != '}')
            {
               advance_token(tokens);
            }
            expect_token(tokens, '}');
            expect_token(tokens, ';');
         }
         else if(first.name == KEYWORD_return)
         {
            advance_token(tokens);
            result->type = ASTSTATEMENT_RETURN;
            result->result = parse_expression(a, tokens);
            expect_token(tokens, ';');
         }
         else if(first.name == KEYWORD_if)
         {
            advance_token(tokens);
            result->type = ASTSTATEMENT_IF;
            expect_token(tokens, '(');
            result->condition = parse_expression(a, tokens);
            expect_token(tokens, ')');

            result->body = parse_statement_block(a, tokens);
         }
         else if(first.name == KEYWORD_for)
         {
            advance_token(tokens);
            result->type = ASTSTATEMENT_FOR;
            expect_token(tokens, '(');
            result->condition = parse_expression(a, tokens);
            expect_token(tokens, ')');

            result->body = parse_statement_block(a, tokens);
         }
         else if(first.name == KEYWORD_var)
         {
            advance_token(tokens);
            result->type = ASTSTATEMENT_VAR;
            result->name = parse_identifer(tokens);
            result->type_name = parse_identifer(tokens);
            expect_token(tokens, '=');
            result->result = parse_expression(a, tokens);
            expect_token(tokens, ';');
         }
         else if(first.name == KEYWORD_import)
         {
            advance_token(tokens);
            result->type = ASTSTATEMENT_IMPORT;
            result->name = parse_identifer(tokens);
            expect_token(tokens, ';');
         }
         else
         {
            result->type = ASTSTATEMENT_EXPRESSION;
            result->result = parse_expression(a, tokens);
            expect_token(tokens, ';');
         }
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

function ast_parameter *parse_parameter(arena *a, token_stream *tokens)
{
   ast_parameter *result = arena_allocate_size(a, sizeof(*result));
   result->name = parse_identifer(tokens);
   result->type_name = parse_identifer(tokens);

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

   expect_token_name(tokens, TOKENTYPE_IDENTIFIER, KEYWORD_function);

   token function_name = expect_token(tokens, TOKENTYPE_IDENTIFIER);
   result->name = function_name.name;
   result->parameters = parse_parameters(a, tokens);

   // NOTE: Optional return type.
   token return_type = peek_token(tokens);
   if(return_type.type == TOKENTYPE_IDENTIFIER)
   {
      result->return_type = parse_identifer(tokens);
   }

   result->body = parse_statement_block(a, tokens);

   return(result);
}

function ast_program parse_program(arena *a, token_stream *tokens)
{
   ast_program result = {0};

   ast_statement **import = &result.imports;
   ast_function **func = &result.functions;

   token peek = peek_token(tokens);
   while(peek.type)
   {
      if(peek.name == intern_string(s8("import")))
      {
         *import = parse_statement(a, tokens);
         import = &(*import)->next;
      }
      else if(peek.name == intern_string(s8("function")))
      {
         *func = parse_function(a, tokens);
         func = &(*func)->next;
      }
      else if(peek.name == intern_stringz("struct") ||
              peek.name == intern_stringz("union") ||
              peek.name == intern_stringz("enum"))
      {
         // TODO: Store type information somewhere.
         parse_statement(a, tokens);
      }

      peek = peek_token(tokens);
   }

   return(result);
}
