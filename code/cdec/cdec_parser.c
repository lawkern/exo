/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#include "cdec_parser.h"

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
      result = arena_allocate(&token_arena, ast_expression, 1);

      lexical_token token = peek_token(tokens);
      switch(token.type)
      {
         case TOKENTYPE_INTEGER:
         {
            result->type = AST_EXPRESSION_LITERAL_INTEGER;
            result->value_integer = token.value_integer;
            advance_token(tokens);
         } break;

         case TOKENTYPE_STRING:
         {
            result->type = AST_EXPRESSION_LITERAL_STRING;
            result->value_string = token.value_string;
            advance_token(tokens);
         } break;

         case TOKENTYPE_NAME:
         {
            result->name = parse_identifer(tokens);
            if(peek_token(tokens).type == TOKENTYPE_OPENPAREN)
            {
               result->type = AST_EXPRESSION_FUNCTIONCALL;
               result->arguments = parse_arguments(tokens);
            }
            else
            {
               result->type = AST_EXPRESSION_NAME;
            }
         } break;

         case TOKENTYPE_ADD:
         case TOKENTYPE_SUB:
         case TOKENTYPE_MUL:
         case TOKENTYPE_BITWISE_AND:
         case TOKENTYPE_BITWISE_NOT:
         {
            advance_token(tokens);

            result->type = AST_EXPRESSION_OPERATION_UNARY;
            result->operator = token.type;
            result->expression = parse_expression(tokens);
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
   ast_statement *result = arena_allocate(&ast_arena, ast_statement, 1);

   lexical_token first = peek_token(tokens);
   switch(first.type)
   {
      case TOKENTYPE_KEYWORD:
      {
         if(first.name == keyword_struct || first.name == keyword_union || first.name == keyword_enum)
         {
            advance_token(tokens);
            result->name = parse_identifer(tokens);

            expect_token(tokens, TOKENTYPE_OPENBRACE);
            while(peek_token(tokens).type != TOKENTYPE_CLOSEBRACE)
            {
               advance_token(tokens);
            }
            expect_token(tokens, TOKENTYPE_CLOSEBRACE);
            expect_token(tokens, TOKENTYPE_SEMICOLON);
         }
         else if(first.name == keyword_return)
         {
            advance_token(tokens);
            result->type = AST_STATEMENT_RETURN;
            result->result = parse_expression(tokens);
            expect_token(tokens, TOKENTYPE_SEMICOLON);
         }
         else if(first.name == keyword_if)
         {
            advance_token(tokens);
            result->type = AST_STATEMENT_IF;
            expect_token(tokens, TOKENTYPE_OPENPAREN);
            result->condition = parse_expression(tokens);
            expect_token(tokens, TOKENTYPE_CLOSEPAREN);

            result->body = parse_statement_block(tokens);
         }
         else if(first.name == keyword_for)
         {
            advance_token(tokens);
            result->type = AST_STATEMENT_FOR;
            if(match_token(tokens, TOKENTYPE_OPENPAREN))
            {
               result->condition = parse_expression(tokens);
               expect_token(tokens, TOKENTYPE_CLOSEPAREN);
            }

            result->body = parse_statement_block(tokens);
         }
         else if(first.name == keyword_var)
         {
            // var foo int = 10;
            advance_token(tokens);
            result->type = AST_STATEMENT_VAR;
            result->name = parse_identifer(tokens);
            result->typespec = parse_typespec(tokens);
            expect_token(tokens, TOKENTYPE_ASSIGN);
            result->result = parse_expression(tokens);
            expect_token(tokens, TOKENTYPE_SEMICOLON);
         }
         else if(first.name == keyword_import)
         {
            advance_token(tokens);
            result->type = AST_STATEMENT_IMPORT;
            result->name = parse_identifer(tokens);
            expect_token(tokens, TOKENTYPE_SEMICOLON);
         }
         else
         {
            result->type = AST_STATEMENT_EXPRESSION;
            result->result = parse_expression(tokens);
            expect_token(tokens, TOKENTYPE_SEMICOLON);
         }
      } break;

      default:
      {
         result->type = AST_STATEMENT_EXPRESSION;
         result->result = parse_expression(tokens);
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
      if(peek.name == intern_stringz("import"))
      {
         *import = parse_statement(tokens);
         import = &(*import)->next;
      }
      else if(peek.name == intern_stringz("function"))
      {
         *func = parse_function(tokens);
         func = &(*func)->next;
      }
      else if(peek.name == intern_stringz("struct") ||
              peek.name == intern_stringz("union") ||
              peek.name == intern_stringz("enum"))
      {
         // TODO: Store type information somewhere.
         parse_statement(tokens);
      }

      peek = peek_token(tokens);
   }

   return(result);
}
