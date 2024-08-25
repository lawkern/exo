/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

function void generate_expression(ast_expression *expression)
{
   switch(expression->type)
   {
      case ASTEXPRESSION_LITERAL_INTEGER:
      {
         platform_log(INDENTATION "mov eax %lld\n", expression->literal_integer.value);
      } break;

      case ASTEXPRESSION_LITERAL_STRING:
      case ASTEXPRESSION_OPERATION_UNARY:
      case ASTEXPRESSION_OPERATION_BINARY:
      {
         platform_log(INDENTATION ";; UNHANDLED EXPRESSION TYPE\n");
      } break;
   }
}

function void generate_statement(ast_statement *statement)
{
   switch(statement->type)
   {
      case ASTSTATEMENT_RETURN:
      {
         generate_expression(statement->result);
         platform_log(INDENTATION "ret\n");
      } break;

      default:
      {
         platform_log(INDENTATION ";; UNHANDLED STATEMENT TYPE %d", statement->type);
      } break;
   }
}

function void generate_function(ast_function *func)
{
   platform_log(".global %s\n", func->name.data);
   platform_log("%s:\n", func->name.data);

   generate_statement(func->body);

   platform_log("\n");
}

function void generate_program(ast_program *program)
{
   platform_log(";;; GENERATED ASSEMBLY CODE\n");

   ast_function *func = program->functions;
   while(func)
   {
      generate_function(program->functions);
      func = program->functions->next;
   }
}
