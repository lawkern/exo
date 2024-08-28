/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#define INDENTATION "   "

global u32 indent_level = 0;
function void print_newline(void)
{
   platform_log("\n");
   for(u32 index = 0; index < indent_level; index++)
   {
      platform_log(INDENTATION);
   }
}

function void print_token(lexical_token token)
{
   switch(token.type)
   {
      case TOKENTYPE_INTEGER: { platform_log("%lld", token.value_integer); } break;
      case TOKENTYPE_STRING:  { platform_log("\"%s\"", token.value_string); } break;
      case TOKENTYPE_KEYWORD: { platform_log("%s", token.name); } break;
      case TOKENTYPE_NAME:    { platform_log("%s", token.name); } break;
      default:                { platform_log("%s", get_tokentype_name(token.type)); } break;
   }
}

function void print_token_stream(void)
{
   for(u32 token_index = 0; token_index < global_tokens.count; ++token_index)
   {
      lexical_token token = global_tokens.tokens[token_index];
      print_token(token);
      platform_log(" ");
   }
   platform_log("\n");
}

function void ast_print_typespec(ast_typespec *typespec)
{
   if(typespec->is_pointer)
   {
      platform_log("*");
   }
   platform_log("%s", typespec->name);
}

function void ast_print_expression(ast_expression *expression)
{
   if(expression)
   {
      switch(expression->type)
      {
         case AST_EXPRESSION_LITERAL_INTEGER:
         {
            platform_log("%lld", expression->value_integer);
         } break;

         case AST_EXPRESSION_LITERAL_STRING:
         {
            platform_log("\"%s\"", expression->value_string);
         } break;

         case AST_EXPRESSION_OPERATION_UNARY:
         {
            platform_log("(%s ", get_tokentype_name(expression->operator));
            ast_print_expression(expression->expression);
            platform_log(")");
         } break;

         case AST_EXPRESSION_OPERATION_BINARY:
         {
            platform_log("%s", get_tokentype_name(expression->operator));
            ast_print_expression(expression->expression);
            ast_print_expression(expression->expression2);
         } break;

         case AST_EXPRESSION_NAME:
         {
            platform_log("%s", expression->name);
         } break;

         case AST_EXPRESSION_FUNCTIONCALL:
         {
            platform_log("call %s ", expression->name);
            ast_expression *argument = expression->arguments;
            while(argument)
            {
               ast_print_expression(argument);
               argument = argument->next;
            }
         } break;
      }
   }
}

function void ast_print_statement_block(ast_statement *);

function void ast_print_statement(ast_statement *statement)
{
   if(statement)
   {
      print_newline();

      platform_log("(");
      switch(statement->type)
      {
         case AST_STATEMENT_RETURN:
         {
            platform_log("return ");
            ast_print_expression(statement->return_expression);
         } break;

         case AST_STATEMENT_IF:
         {
            platform_log("if ");
            ast_print_expression(statement->condition);

            indent_level++;
            ast_print_statement_block(statement->then_block);
            if(statement->else_block)
            {
               ast_print_statement_block(statement->else_block);
            }
            indent_level--;
         } break;

         case AST_STATEMENT_FOR:
         {
            platform_log("for ");
            if(statement->pre)
            {
               ast_print_expression(statement->pre);
               platform_log(" ");
            }
            ast_print_expression(statement->condition);
            if(statement->post)
            {
               platform_log(" ");
               ast_print_expression(statement->post);
            }

            indent_level++;
            ast_print_statement_block(statement->body);
            indent_level--;
         } break;

         case AST_STATEMENT_VAR:
         {
            platform_log("var %s ", statement->name);
            ast_print_typespec(statement->typespec);
            platform_log(" ");
            ast_print_expression(statement->initializer);
         } break;

         case AST_STATEMENT_IMPORT:
         {
            platform_log("import %s", statement->name);
         } break;

         case AST_STATEMENT_EXPRESSION:
         {
            ast_print_expression(statement->return_expression);
         } break;
      }

      platform_log(")");
   }
}

function void ast_print_statement_block(ast_statement *block)
{
   print_newline();

   platform_log("(block");
   ast_statement *statement = block;
   while(statement)
   {
      indent_level++;
      ast_print_statement(statement);
      statement = statement->next;
      indent_level--;
   }
   platform_log(")");
}

function void ast_print_function(ast_function *func)
{
   print_newline();

   platform_log("(function %s (", func->name);
   ast_parameter *parameter = func->parameters;
   while(parameter)
   {
      platform_log("%s %s", parameter->name, parameter->typespec->name);
      parameter = parameter->next;
      if(parameter)
      {
         platform_log(", ");
      }
   }

   platform_log(") ");

   if(func->return_type)
   {
      ast_print_typespec(func->return_type);
   }

   indent_level++;
   ast_print_statement_block(func->body);
   indent_level--;
}

function void ast_print_program(ast_program *program)
{
   ast_statement *import = program->imports;
   while(import)
   {
      ast_print_statement(import);
      import = import->next;

      print_newline();
   }

   ast_function *func = program->functions;
   while(func)
   {
      ast_print_function(func);
      func = func->next;

      print_newline();
   }
   print_newline();
}
