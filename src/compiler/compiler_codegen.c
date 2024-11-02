/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#define ASM_INDENTATION "   "

function void generate_asm_expression(ast_expression *expression)
{
   switch(expression->type)
   {
      case AST_EXPRESSION_LITERAL_INTEGER:
      {
         platform_log(ASM_INDENTATION "mov eax %lld\n", expression->value_integer);
      } break;

      case AST_EXPRESSION_LITERAL_STRING:
      case AST_EXPRESSION_OPERATION_UNARY:
      case AST_EXPRESSION_OPERATION_BINARY:
      case AST_EXPRESSION_OPERATION_TERNARY:
      case AST_EXPRESSION_NAME:
      case AST_EXPRESSION_FUNCTIONCALL:
      {
         platform_log(ASM_INDENTATION ";; UNHANDLED EXPRESSION TYPE\n");
      } break;
   }
}

function void generate_asm_statement(ast_statement *statement)
{
   switch(statement->type)
   {
      case AST_STATEMENT_RETURN:
      {
         generate_asm_expression(statement->return_stmt.expression);
         platform_log(ASM_INDENTATION "ret\n");
      } break;

      default:
      {
         platform_log(ASM_INDENTATION ";; UNHANDLED STATEMENT TYPE %d\n", statement->type);
      } break;
   }
}

function void generate_asm_function(ast_function *func)
{
   platform_log(".global %s\n", func->name);
   platform_log("%s:\n", func->name);

   ast_statement *statement = func->body;
   while(statement)
   {
      generate_asm_statement(statement);
      statement = statement->next;
   }

   platform_log("\n");
}

function void generate_asm_program(ast_program *program)
{
   platform_log(";;; GENERATED ASSEMBLY CODE\n");

   ast_function *func = program->functions;
   while(func)
   {
      generate_asm_function(func);
      func = func->next;
   }
}
