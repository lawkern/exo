/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#define INDENTATION "   "

function void print_token(token *token)
{
   platform_log("\"");
   switch(token->type)
   {
      case TOKENTYPE_INTEGER:    { platform_log("%lld", token->value_integer); } break;
      case TOKENTYPE_STRING:     { platform_log("\"%s\"", token->value_string); } break;
      case TOKENTYPE_IDENTIFIER: { platform_log("%s", token->name); } break;
      default:
      {
         if(token->type < 128)
         {
            platform_log("%c", token->type);
         }
         else
         {
            char *token_name = get_tokentype_name(token->type);
            if(token_name)
            {
               platform_log("%s", token_name);
            }
            else
            {
               syntax_error("UNHANDLED TOKEN %d", token->type);
            }
         }
      } break;
   }
   platform_log("\", ");
}

function void print_token_stream(token_stream *tokens)
{
   platform_log("TOKEN STREAM:\n");
   for(u32 token_index = 0; token_index < tokens->count; ++token_index)
   {
      token *token = tokens->tokens + token_index;
      print_token(token);
   }
   platform_log("\n\n");
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
         platform_log("%lld\n", expression->value_integer);
      } break;

      case ASTEXPRESSION_LITERAL_STRING:
      {
         platform_log("\"%s\"\n", expression->value_string);
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

      case ASTEXPRESSION_FUNCTIONCALL:
      {
         platform_log("%s()\n", expression->name);
         ast_expression *argument = expression->arguments;
         while(argument)
         {
            ast_print_expression(argument, indent_level + 1);
            argument = argument->next;
         }
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
         platform_log("var %s = \n", statement->name);
         ast_print_expression(statement->result, indent_level + 1);
      } break;

      case ASTSTATEMENT_IMPORT:
      {
         platform_log("import %s\n", statement->name);
      } break;

      case ASTSTATEMENT_EXPRESSION:
      {
         platform_log("\n");
         ast_print_expression(statement->result, indent_level + 1);
      } break;

      default:
      {
         platform_log("UNHANDLED EXPRESSION TYPE %d\n", statement->type);
      } break;
   }
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

   platform_log("FUNCTION: %s(", func->name);
   ast_parameter *parameter = func->parameters;
   while(parameter)
   {
      platform_log("%s %s", parameter->name, parameter->type_name);
      parameter = parameter->next;
      if(parameter)
      {
         platform_log(", ");
      }
   }

   platform_log(")");
   if(func->return_type)
   {
      platform_log(" -> %s", func->return_type);
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
