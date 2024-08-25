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

function void ast_print_statement_block(ast_statement *, u32);

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

      case ASTSTATEMENT_IF:
      {
         platform_log("if\n");
         ast_print_statement_block(statement->body, indent_level + 1);
      } break;

      case ASTSTATEMENT_FOR:
      {
         platform_log("for\n");
         ast_print_statement_block(statement->body, indent_level + 1);
      } break;

      case ASTSTATEMENT_VAR:
      {
         platform_log("var %s = \n", statement->identifier->name.data);
         ast_print_expression(statement->result, indent_level + 1);
      } break;

      case ASTSTATEMENT_IMPORT:
      {
         platform_log("import %s\n", statement->identifier->name.data);
      } break;

      default:
      {
         platform_log("UNHANDLED EXPRESSION TYPE %d", statement->type);
      } break;
   }
   platform_log("\n");
}

function void ast_print_statement_block(ast_statement *block, u32 indent_level)
{
   ast_statement *statement = block;
   while(statement)
   {
      ast_print_statement(statement, indent_level);
      statement = statement->next;
   }
}

function void ast_print_function(ast_function *func, u32 indent_level)
{
   print_indentation(indent_level);

   platform_log("FUNCTION: %s(", func->name.data);
   ast_parameter *parameter = func->parameters;
   while(parameter)
   {
      platform_log("%s %s", parameter->name->name.data, parameter->type->name.data);
      parameter = parameter->next;
      if(parameter)
      {
         platform_log(", ");
      }
   }

   platform_log(")");
   if(func->return_type->name.length > 0)
   {
      platform_log(" -> %s", func->return_type->name.data);
   }
   platform_log("\n");
   ast_print_statement_block(func->body, indent_level + 1);
}

function void ast_print_program(ast_program *program)
{
   platform_log("PROGRAM AST:\n");

   ast_statement *import = program->imports;
   while(import)
   {
      ast_print_statement(import, 0);
      import = import->next;
   }

   ast_function *func = program->functions;
   while(func)
   {
      ast_print_function(func, 0);
      func = func->next;
   }

   platform_log("\n");
}
