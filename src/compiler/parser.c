/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#include "parser.h"

function ast_expression *new_expression(ast_expression_type type)
{
   ast_expression *result = arena_allocate(&ast_arena, ast_expression, 1);

   zero_memory(result, sizeof(*result));
   result->type = type;

   return(result);
}

function ast_expression *new_expression_integer(u64 value)
{
   ast_expression *result = new_expression(AST_EXPRESSION_LITERAL_INTEGER);
   result->value_integer = value;

   return(result);
}

function ast_expression *new_expression_string(char *value)
{
   ast_expression *result = new_expression(AST_EXPRESSION_LITERAL_STRING);
   result->value_string = value;

   return(result);
}

function ast_expression *new_expression_name(char *name)
{
   ast_expression *result = new_expression(AST_EXPRESSION_NAME);
   result->name = name;

   return(result);
}

function ast_expression *new_expression_function_call(char *name, ast_expression *arguments)
{
   ast_expression *result = new_expression(AST_EXPRESSION_FUNCTIONCALL);
   result->name = name;
   result->arguments = arguments;

   return(result);
}

function ast_expression *new_expression_unary_operation(tokentype operator, ast_expression *expression)
{
   ast_expression *result = new_expression(AST_EXPRESSION_OPERATION_UNARY);
   result->operator = operator;
   result->expression = expression;

   return(result);
}

function ast_expression *new_expression_binary_operation(tokentype operator, ast_expression *expression, ast_expression *expression2)
{
   ast_expression *result = new_expression(AST_EXPRESSION_OPERATION_BINARY);
   result->operator = operator;
   result->expression = expression;
   result->expression2 = expression2;

   return(result);
}

function ast_expression *new_expression_ternary_operation(tokentype operator, ast_expression *expression, ast_expression *expression2, ast_expression *expression3)
{
   ast_expression *result = new_expression(AST_EXPRESSION_OPERATION_TERNARY);
   result->operator = operator;
   result->expression = expression;
   result->expression2 = expression2;
   result->expression3 = expression3;

   return(result);
}

function ast_statement *new_statement(ast_statement_type type)
{
   ast_statement *result = arena_allocate(&ast_arena, ast_statement, 1);

   zero_memory(result, sizeof(*result));
   result->type = type;

   return(result);
}

function ast_statement *new_statement_return(ast_expression *return_expression)
{
   ast_statement *result = new_statement(AST_STATEMENT_RETURN);
   result->return_stmt.expression = return_expression;

   return(result);
}

function ast_statement *new_statement_if(ast_expression *condition, ast_statement *then_block, ast_statement *else_block)
{
   ast_statement *result = new_statement(AST_STATEMENT_IF);
   result->if_stmt.condition = condition;
   result->if_stmt.then_block = then_block;
   result->if_stmt.else_block = else_block;

   return(result);
}

function ast_statement *new_statement_for(ast_expression *condition, ast_expression *pre, ast_expression *post, ast_statement *body)
{
   ast_statement *result = new_statement(AST_STATEMENT_FOR);
   result->for_stmt.condition = condition;
   result->for_stmt.pre = pre;
   result->for_stmt.post = post;
   result->for_stmt.body = body;

   return(result);
}

function ast_statement *new_statement_var(char *name, ast_typespec *typespec, ast_expression *expression)
{
   ast_statement *result = new_statement(AST_STATEMENT_VAR);
   result->var_stmt.name = name;
   result->var_stmt.typespec = typespec;
   result->var_stmt.expression = expression;

   return(result);
}

function ast_statement *new_statement_import(char *name)
{
   ast_statement *result = new_statement(AST_STATEMENT_IMPORT);
   result->import_stmt.name = name;

   return(result);
}

function ast_statement *new_statement_expression(ast_expression *expression)
{
   ast_statement *result = new_statement(AST_STATEMENT_EXPRESSION);
   result->return_stmt.expression = expression;

   return(result);
}

function char *parse_identifer(token_stream *tokens)
{
   lexical_token identifier = advance_token(tokens);
   if(identifier.type != TOKENTYPE_NAME && identifier.type != TOKENTYPE_KEYWORD)
   {
      syntax_error("NOT AN IDENTIFIER");
   }

   char *result = identifier.name;
   return(result);
}

function ast_typespec *parse_typespec(token_stream *tokens)
{
   ast_typespec *result = arena_allocate(&ast_arena, ast_typespec, 1);

   result->is_pointer = match_token(tokens, TOKENTYPE_MUL);
   lexical_token name = advance_token(tokens);
   result->name = name.name;

   return(result);
}

function ast_expression *parse_arguments(token_stream *);

function ast_expression *parse_expression(token_stream *tokens)
{
   ast_expression *result = 0;

   if(match_token(tokens, TOKENTYPE_OPENPAREN))
   {
      result = parse_expression(tokens);
      expect_token(tokens, TOKENTYPE_CLOSEPAREN);
   }
   else
   {
      lexical_token token = peek_token(tokens);
      switch(token.type)
      {
         case TOKENTYPE_INTEGER:
         {
			result = new_expression_integer(token.value_integer);
            advance_token(tokens);
         } break;

         case TOKENTYPE_STRING:
         {
			result = new_expression_string(token.value_string);
            advance_token(tokens);
         } break;

         case TOKENTYPE_NAME:
         {
            char *name = parse_identifer(tokens);

            lexical_token next = peek_token(tokens);
            if(next.type == TOKENTYPE_OPENPAREN)
            {
               result = new_expression_function_call(name, parse_arguments(tokens));
            }
            else if(next.type == TOKENTYPE_INCREMENT || next.type == TOKENTYPE_DECREMENT)
            {
               advance_token(tokens);
               result = new_expression_unary_operation(next.type, new_expression_name(name));
            }
            else
            {
               result = new_expression_name(name);
            }
         } break;

         case TOKENTYPE_ADD:
         case TOKENTYPE_SUB:
         case TOKENTYPE_MUL:
         case TOKENTYPE_BITWISE_AND:
         case TOKENTYPE_BITWISE_NOT:
         {
            advance_token(tokens);
            result = new_expression_unary_operation(token.type, parse_expression(tokens));
         } break;

         default:
         {
            syntax_error("UNHANDLED EXPRESSION TOKEN TYPE %s.", get_tokentype_name(token.type));
         } break;
      }
   }

   return(result);
}

function ast_expression *parse_arguments(token_stream *tokens)
{
   ast_expression *result = 0;

   expect_token(tokens, TOKENTYPE_OPENPAREN);

   ast_expression **argument = &result;
   while(peek_token(tokens).type != TOKENTYPE_CLOSEPAREN)
   {
      *argument = parse_expression(tokens);
      argument = &(*argument)->next;
   }

   expect_token(tokens, TOKENTYPE_CLOSEPAREN);

   return(result);
}

function ast_statement *parse_statement_block(token_stream *);

function ast_statement *parse_statement(token_stream *tokens)
{
   ast_statement *result = 0;

   lexical_token token = advance_token(tokens);
   switch(token.type)
   {
      case TOKENTYPE_KEYWORD:
      {
         if(token.name == keyword_struct || token.name == keyword_union || token.name == keyword_enum)
         {
            // result = arena_allocate(&ast_arena, ast_statement, 1);
            // result->name = parse_identifer(tokens);
            parse_identifer(tokens);

            expect_token(tokens, TOKENTYPE_OPENBRACE);
            while(peek_token(tokens).type != TOKENTYPE_CLOSEBRACE)
            {
               advance_token(tokens);
            }
            expect_token(tokens, TOKENTYPE_CLOSEBRACE);
            expect_token(tokens, TOKENTYPE_SEMICOLON);
         }
         else if(token.name == keyword_return)
         {
            result = new_statement_return(parse_expression(tokens));
            expect_token(tokens, TOKENTYPE_SEMICOLON);
         }
         else if(token.name == keyword_if)
         {
            expect_token(tokens, TOKENTYPE_OPENPAREN);
            ast_expression *condition = parse_expression(tokens);
            expect_token(tokens, TOKENTYPE_CLOSEPAREN);

            ast_statement *then_block = parse_statement_block(tokens);
            ast_statement *else_block = 0;
            if(peek_token(tokens).name == keyword_else)
            {
               expect_token_name(tokens, TOKENTYPE_KEYWORD, keyword_else);
               parse_statement_block(tokens);
            }

            result = new_statement_if(condition, then_block, else_block);
         }
         else if(token.name == keyword_for)
         {
            ast_expression *condition = 0, *pre = 0, *post = 0;
            if(match_token(tokens, TOKENTYPE_OPENPAREN))
            {
               ast_expression *expression = parse_expression(tokens);
               if(peek_token(tokens).type == TOKENTYPE_SEMICOLON)
               {
                  pre = expression;
                  expect_token(tokens, TOKENTYPE_SEMICOLON);

                  condition = parse_expression(tokens);
                  expect_token(tokens, TOKENTYPE_SEMICOLON);

                  post = parse_expression(tokens);
               }
               else
               {
                  condition = expression;
               }

               expect_token(tokens, TOKENTYPE_CLOSEPAREN);
            }

            result = new_statement_for(condition, pre, post, parse_statement_block(tokens));
         }
         else if(token.name == keyword_var)
         {
            // e.g. var foo int = 10;
            char *name = parse_identifer(tokens);
            ast_typespec *typespec = parse_typespec(tokens);
            ast_expression *return_expression = (match_token(tokens, TOKENTYPE_ASSIGN)) ? parse_expression(tokens) : 0;
            expect_token(tokens, TOKENTYPE_SEMICOLON);

            result = new_statement_var(name, typespec, return_expression);
         }
         else if(token.name == keyword_import)
         {
            result = new_statement_import(parse_identifer(tokens));
            expect_token(tokens, TOKENTYPE_SEMICOLON);
         }
         else
         {
            result = new_statement_expression(parse_expression(tokens));
            expect_token(tokens, TOKENTYPE_SEMICOLON);
         }
      } break;

      default:
      {
         rewind_token(tokens);
         result = new_statement_expression(parse_expression(tokens));
         expect_token(tokens, TOKENTYPE_SEMICOLON);
      } break;

      // default:
      // {
      //    syntax_error("UNHANDLED STATEMENT TYPE");
      // } break;
   }

   return(result);
}

function ast_statement *parse_statement_block(token_stream *tokens)
{
   ast_statement *result = 0;

   expect_token(tokens, TOKENTYPE_OPENBRACE);

   ast_statement **statement = &result;
   while(peek_token(tokens).type != TOKENTYPE_CLOSEBRACE)
   {
      *statement = parse_statement(tokens);
      statement = &(*statement)->next;
   }

   expect_token(tokens, TOKENTYPE_CLOSEBRACE);

   return(result);
}

function ast_parameter *parse_parameter(token_stream *tokens)
{
   ast_parameter *result = arena_allocate_size(&ast_arena, sizeof(*result));
   result->name = parse_identifer(tokens);
   result->typespec = parse_typespec(tokens);

   return(result);
}

function ast_parameter *parse_parameters(token_stream *tokens)
{
   ast_parameter *result = 0;

   expect_token(tokens, TOKENTYPE_OPENPAREN);
   ast_parameter **parameter = &result;
   while(peek_token(tokens).type != TOKENTYPE_CLOSEPAREN)
   {
      *parameter = parse_parameter(tokens);
      if(peek_token(tokens).type != TOKENTYPE_CLOSEPAREN)
      {
         expect_token(tokens, TOKENTYPE_COMMA);
      }
   }
   expect_token(tokens, TOKENTYPE_CLOSEPAREN);

   return(result);
}

function ast_function *parse_function(token_stream *tokens)
{
   ast_function *result = arena_allocate(&ast_arena, ast_function, 1);

   expect_token_name(tokens, TOKENTYPE_KEYWORD, keyword_function);

   lexical_token function_name = expect_token(tokens, TOKENTYPE_NAME);
   result->name = function_name.name;
   result->parameters = parse_parameters(tokens);

   // NOTE: Optional return type.
   lexical_token return_type = peek_token(tokens);
   if(return_type.type == TOKENTYPE_NAME || return_type.type == TOKENTYPE_KEYWORD)
   {
      result->return_type = parse_typespec(tokens);
   }

   result->body = parse_statement_block(tokens);

   return(result);
}

function ast_program parse_program(token_stream *tokens)
{
   ast_program result = {0};

   ast_statement **import = &result.imports;
   ast_function **func = &result.functions;

   lexical_token peek = peek_token(tokens);
   while(peek.type)
   {
      if(peek.name == keyword_import)
      {
         *import = parse_statement(tokens);
         import = &(*import)->next;
      }
      else if(peek.name == keyword_function)
      {
         *func = parse_function(tokens);
         func = &(*func)->next;
      }
      else if(peek.name == keyword_struct ||
              peek.name == keyword_union ||
              peek.name == keyword_enum)
      {
         // TODO: Store type information somewhere.
         parse_statement(tokens);
      }

      peek = peek_token(tokens);
   }

   return(result);
}
